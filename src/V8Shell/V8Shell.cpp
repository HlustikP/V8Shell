#include "../../include/V8Shell.h"

V8Shell::V8Shell(int argc, char* argv[], int& exit_code) {
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  platform_ = v8::platform::NewDefaultPlatform();

  if (platform_ == nullptr) {
    Commands::PrintErrorTag();
    std::cerr << " Failed to setup v8 platform." << std::endl;
    exit_code = 1;

    return;
  }

  argc_ = argc;
  argv_ = argv;

  // no arguments -> run shell, otherwise make it depend on the arguments
  settings_.run_shell = (argc == 1);

  v8::V8::InitializePlatform(platform_.get());
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  v8::V8::Initialize();

  auto v8_setup_valid = SetupV8Isolate();
  if (!v8_setup_valid) {
    Commands::PrintErrorTag();
    std::cerr << " Failed to setup V8 Isolate" << std::endl;
    exit_code = 1;

    return;
  }

  exit_code = 0;
}

V8Shell::~V8Shell() {
  isolate_->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  delete create_params_.array_buffer_allocator;
}

/** Setup the V8 Isolate. */
bool V8Shell::SetupV8Isolate() {
  create_params_.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  isolate_ = v8::Isolate::New(create_params_);

  if (create_params_.array_buffer_allocator == nullptr) {
    return false;
  }

  return true;
}

/** Process remaining command line arguments, execute files and possibly enter shell. */
int V8Shell::Run() {
  v8::Isolate::Scope isolate_scope(isolate_);
  v8::HandleScope handle_scope(isolate_);
  v8::Local<v8::Context> context = CreateShellContext();

  if (context.IsEmpty()) {
    Commands::PrintErrorTag();
    std::cerr << " Failed to create shell context" << std::endl;

    return 1;
  }
  v8::Context::Scope context_scope(context);

  // Process remaining command line arguments and execute files.
  for (int i = 1; i < argc_; i++) {
    const char* str = argv_[i];
    if (strcmp(str, "--shell") == 0) {
      settings_.run_shell = true;
    } else if (strcmp(str, "--no-shell") == 0) {
      settings_.run_shell = false;
    } else if (strcmp(str, "-f") == 0) {
      // Ignore any -f flags for compatibility with the other stand-
      // alone JavaScript engines.
      continue;
    } else if (strcmp(str, "--help") == 0) {
      // TODO: Implement
      continue;
    } else if (strcmp(str, "-e") == 0 && i + 1 < argc_) {
      // Execute argument given to -e option directly.
      v8::Local<v8::String> file_name =
          v8::String::NewFromUtf8Literal(isolate_, "unnamed");
      v8::Local<v8::String> source;
      if (!v8::String::NewFromUtf8(isolate_, argv_[++i]).ToLocal(&source)) {
        return 1;
      }
      bool success =
          Commands::ExecuteString(isolate_, source, file_name, false, true);
      settings_.run_shell = false;
      while (v8::platform::PumpMessageLoop(platform_.get(), isolate_)) continue;
      if (!success) return 1;
    } else if (strncmp(str, "-", 1) == 0) {
      Commands::PrintWarningTag();
      std::cerr << " used unknown flag " << str << std::endl
                << "Try --help for options" << std::endl;
    } else {
      // Use all other arguments as names of files to load and run.
      v8::Local<v8::String> file_name =
          v8::String::NewFromUtf8(isolate_, str).ToLocalChecked();
      v8::Local<v8::String> source;
      auto file_content = Commands::ReadFile(isolate_, str);
      if (!file_content.has_value()) {
        Commands::PrintErrorTag();
        std::cerr << " cannot read file " << str << std::endl;

        continue;
      }
      file_content.value().ToLocal(&source);
      bool success =
          Commands::ExecuteString(isolate_, source, file_name, false, true);

      while (v8::platform::PumpMessageLoop(platform_.get(), isolate_)) continue;

      if (!success) return 1;
    }
  }

  if (settings_.run_shell) {
    RunShell(context);
  }

  return 0;
}

/** The read-eval-execute loop of the shell. */
void V8Shell::RunShell(v8::Local<v8::Context> context) {
  auto path = fs::current_path();
  Commands::SetCWD(path);

  std::cout << "[V8Shell " << Settings::current_version << "] V8 version "
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

    while (v8::platform::PumpMessageLoop(platform_.get(), context->GetIsolate()))
      continue;
  }
  std::cout << std::endl;
}

/** Creates a new execution environment containing the built-in functions. */
v8::Local<v8::Context> V8Shell::CreateShellContext() {
  // Create a template for the global object.
  v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate_);

  // Register c++ hooks to global functions
  for (auto& hook : cpp_hooks) {
    global->Set(isolate_, std::get<0>(hook).c_str(),
                v8::FunctionTemplate::New(isolate_, std::get<1>(hook)));
  }

  return v8::Context::New(isolate_, NULL, global);
}
