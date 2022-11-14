#include "V8Shell.h"

int main(int argc, char* argv[]) {
  auto platform = SetupV8(argc, argv);
  if (platform == nullptr) {
    Commands::PrintErrorTag();
    std::cerr << " Failed to setup V8 VM" << std::endl;

    return 1;
  }

  v8::Isolate::CreateParams create_params;
  v8::Isolate* isolate = nullptr;
  auto v8_setup_valid = SetupV8Isolate(&create_params, &isolate);
  if (!v8_setup_valid) {
    Commands::PrintErrorTag();
    std::cerr << " Failed to setup V8 Isolate" << std::endl;

    return 1;
  }

  Settings settings;

  // no arguments -> run shell, otherwise make it depend on the arguments
  settings.run_shell = (argc == 1);
    
  int result = 0;
  {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = CreateShellContext(isolate);
    if (context.IsEmpty()) {
      Commands::PrintErrorTag();
      std::cerr << " Failed to create shell context" << std::endl;

      return 1;
    }
    v8::Context::Scope context_scope(context);
    result = RunMain(isolate, platform.get(), argc, argv, settings);
    if (settings.run_shell) RunShell(context, platform.get());
  }
  Cleanup(isolate, &create_params);
    
  return result;
}

/** Setup the V8 VM, returns nullptr on failure. */
std::unique_ptr<v8::Platform> SetupV8(int argc, char* argv[]) {
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    auto platform = v8::platform::NewDefaultPlatform();

    if (platform == nullptr) {
        return nullptr;
    }

    v8::V8::InitializePlatform(platform.get());
    v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
    v8::V8::Initialize();

    return std::move(platform);
}

/** Setup the V8 Isolate. */
bool SetupV8Isolate(v8::Isolate::CreateParams* create_params, v8::Isolate** isolate) {
    create_params->array_buffer_allocator =
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    *isolate = v8::Isolate::New(*create_params);

    if (create_params->array_buffer_allocator == nullptr) {
        return false;
    }

    return true;
}

/** Free memory held by V8. */
void Cleanup(v8::Isolate* isolate, v8::Isolate::CreateParams* create_params) {
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    delete create_params->array_buffer_allocator;
}

/** Creates a new execution environment containing the built-in functions. */
v8::Local<v8::Context> CreateShellContext(v8::Isolate* isolate) {
    // Create a template for the global object.
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

    // Register c++ hooks to global functions
    global->Set(isolate, "print", v8::FunctionTemplate::New(isolate, Commands::Print));
    global->Set(isolate, "read", v8::FunctionTemplate::New(isolate, Commands::Read));
    global->Set(isolate, "execute", v8::FunctionTemplate::New(isolate, Commands::Execute));
    global->Set(isolate, "quit", v8::FunctionTemplate::New(isolate, Commands::Quit));
    global->Set(isolate, "exit", v8::FunctionTemplate::New(isolate, Commands::Quit));
    global->Set(isolate, "version", v8::FunctionTemplate::New(isolate, Commands::Version));
    global->Set(isolate, "cd", v8::FunctionTemplate::New(isolate, Commands::ChangeDirectory));
    global->Set(isolate, "changeDirectory", 
      v8::FunctionTemplate::New(isolate, Commands::ChangeDirectory));
    global->Set(isolate, "ls", v8::FunctionTemplate::New(isolate, Commands::ListFiles));
    global->Set(isolate, "ll", v8::FunctionTemplate::New(isolate, Commands::ListFiles));
    global->Set(isolate, "runSync", v8::FunctionTemplate::New(isolate, Commands::StartProcessSync));

    return v8::Context::New(isolate, NULL, global);
}

/** Process remaining command line arguments and execute files. */
int RunMain(v8::Isolate* isolate, v8::Platform* platform, int argc,
  char* argv[], Settings& settings) {
  for (int i = 1; i < argc; i++) {
    const char* str = argv[i];
    if (strcmp(str, "--shell") == 0) {
      settings.run_shell = true;
    }
    else if (strcmp(str, "--no-shell") == 0) {
      settings.run_shell = false;
    }
    else if (strcmp(str, "-f") == 0) {
      // Ignore any -f flags for compatibility with the other stand-
      // alone JavaScript engines.
      continue;
    }
    else if (strncmp(str, "--", 2) == 0) {
      Commands::PrintWarningTag();
      std::cerr << " used unknown flag " << str << std::endl 
        << "Try --help for options" << std::endl;
    }
    else if (strcmp(str, "-e") == 0 && i + 1 < argc) {
      // Execute argument given to -e option directly.
      v8::Local<v8::String> file_name =
          v8::String::NewFromUtf8Literal(isolate, "unnamed");
      v8::Local<v8::String> source;
      if (!v8::String::NewFromUtf8(isolate, argv[++i]).ToLocal(&source)) {
        return 1;
      }
      bool success = Commands::ExecuteString(isolate, source, file_name, false, true);
      settings.run_shell = false;
      while (v8::platform::PumpMessageLoop(platform, isolate)) continue;
      if (!success) return 1;
    }
    else {
      // Use all other arguments as names of files to load and run.
      v8::Local<v8::String> file_name =
          v8::String::NewFromUtf8(isolate, str).ToLocalChecked();
      v8::Local<v8::String> source;
      auto file_content = Commands::ReadFile(isolate, str);
      if (file_content.has_value()) {
        file_content.value().ToLocal(&source);
        Commands::PrintErrorTag();
        std::cerr << " cannot read file " << str << std::endl;

        continue;
      }
      bool success = Commands::ExecuteString(isolate, source, file_name, false, true);
      while (v8::platform::PumpMessageLoop(platform, isolate)) continue;
      if (!success) return 1;
    }
  }
  return 0;
}

/** The read-eval-execute loop of the shell. */
void RunShell(v8::Local<v8::Context> context, v8::Platform* platform) {
  auto path = fs::current_path();
  Commands::SetCWD(path);

  std::cout << "[V8Shell " << current_version << "] V8 version " 
    << v8::V8::GetVersion() << std::endl;
  Commands::PrintCWD();

  static const int kBufferSize = 1024;
  char buffer[kBufferSize];

  // Enter the execution environment before evaluating any code.
  v8::Context::Scope context_scope(context);
  v8::Local<v8::String> name(
      v8::String::NewFromUtf8Literal(context->GetIsolate(), "(shell)"));
  while (true) {
    char* str = fgets(buffer, kBufferSize, stdin);
    if (str == NULL) break;

    v8::HandleScope handle_scope(context->GetIsolate());
    auto print_expression_eval = true;

    Commands::ExecuteString(
        context->GetIsolate(),
        v8::String::NewFromUtf8(context->GetIsolate(), str).ToLocalChecked(),
        name, print_expression_eval, true);

    while (v8::platform::PumpMessageLoop(platform, context->GetIsolate()))
      continue;
  }
  std::cout << std::endl;
}
