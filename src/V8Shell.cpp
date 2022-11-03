#include "V8Shell.h"

int main(int argc, char* argv[]) {
    auto platform = SetupV8(argc, argv);
    if (platform == nullptr) {
        std::cerr << "[ERROR] Failed to setup V8 VM" << std::endl;

        return 1;
    }

    v8::Isolate::CreateParams create_params;
    v8::Isolate* isolate = nullptr;
    SetupV8Isolate(&create_params, &isolate);

    run_shell = (argc == 1);
    
    int result;
    {
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = CreateShellContext(isolate);
        if (context.IsEmpty()) {
            fprintf(stderr, "Error creating context\n");
            return 1;
        }
        v8::Context::Scope context_scope(context);
        result = RunMain(isolate, platform.get(), argc, argv);
        if (run_shell) RunShell(context, platform.get());
    }

    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    delete create_params.array_buffer_allocator;
    return result;
}


// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
}

// Setup the V8 VM
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

bool SetupV8Isolate(v8::Isolate::CreateParams* create_params, v8::Isolate** isolate) {
    create_params->array_buffer_allocator =
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    *isolate = v8::Isolate::New(*create_params);

    if (create_params->array_buffer_allocator == nullptr) {
        return false;
    }

    return true;
}

// Creates a new execution environment containing the built-in functions.
v8::Local<v8::Context> CreateShellContext(v8::Isolate* isolate) {
    // Create a template for the global object.
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    // Bind the global 'print' function to the C++ Print callback.
    global->Set(isolate, "print", v8::FunctionTemplate::New(isolate, Print));
    // Bind the global 'read' function to the C++ Read callback.
    global->Set(isolate, "read", v8::FunctionTemplate::New(isolate, Read));
    // Bind the global 'load' function to the C++ Load callback.
    global->Set(isolate, "load", v8::FunctionTemplate::New(isolate, Load));
    // Bind the 'quit' function
    global->Set(isolate, "quit", v8::FunctionTemplate::New(isolate, Quit));
    // Bind the 'version' function
    global->Set(isolate, "version", v8::FunctionTemplate::New(isolate, Version));
    return v8::Context::New(isolate, NULL, global);
}


// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.
void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
    bool first = true;
    for (int i = 0; i < args.Length(); i++) {
        v8::HandleScope handle_scope(args.GetIsolate());
        if (first) {
            first = false;
        }
        else {
            printf(" ");
        }
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        const char* cstr = ToCString(str);
        printf("%s", cstr);
    }
    printf("\n");
    fflush(stdout);
}


// The callback that is invoked by v8 whenever the JavaScript 'read'
// function is called.  This function loads the content of the file named in
// the argument into a JavaScript string.
void Read(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 1) {
        args.GetIsolate()->ThrowError("Bad parameters");
        return;
    }
    v8::String::Utf8Value file(args.GetIsolate(), args[0]);
    if (*file == NULL) {
        args.GetIsolate()->ThrowError("Error loading file");
        return;
    }
    v8::Local<v8::String> source;
    if (!ReadFile(args.GetIsolate(), *file).ToLocal(&source)) {
        args.GetIsolate()->ThrowError("Error loading file");
        return;
    }

    args.GetReturnValue().Set(source);
}

// The callback that is invoked by v8 whenever the JavaScript 'load'
// function is called.  Loads, compiles and executes its argument
// JavaScript file.
void Load(const v8::FunctionCallbackInfo<v8::Value>& args) {
    for (int i = 0; i < args.Length(); i++) {
        v8::HandleScope handle_scope(args.GetIsolate());
        v8::String::Utf8Value file(args.GetIsolate(), args[i]);
        if (*file == NULL) {
            args.GetIsolate()->ThrowError("Error loading file");
            return;
        }
        v8::Local<v8::String> source;
        if (!ReadFile(args.GetIsolate(), *file).ToLocal(&source)) {
            args.GetIsolate()->ThrowError("Error loading file");
            return;
        }
        if (!ExecuteString(args.GetIsolate(), source, args[i], false, false)) {
            args.GetIsolate()->ThrowError("Error executing file");
            return;
        }
    }
}


// The callback that is invoked by v8 whenever the JavaScript 'quit'
// function is called.  Quits.
void Quit(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // If not arguments are given args[0] will yield undefined which
    // converts to the integer value 0.
    int exit_code =
        args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);
    fflush(stdout);
    fflush(stderr);
    exit(exit_code);
}


void Version(const v8::FunctionCallbackInfo<v8::Value>& args) {
    args.GetReturnValue().Set(
        v8::String::NewFromUtf8(args.GetIsolate(), v8::V8::GetVersion())
        .ToLocalChecked());
}


// Reads a file into a v8 string.
v8::MaybeLocal<v8::String> ReadFile(v8::Isolate* isolate, const char* name) {
    FILE* file = nullptr;
    fopen_s(&file, name, "rb");
    if (file == NULL) return v8::MaybeLocal<v8::String>();

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* chars = new char[size + 1];
    chars[size] = '\0';
    for (size_t i = 0; i < size;) {
        i += fread(&chars[i], 1, size - i, file);
        if (ferror(file)) {
            fclose(file);
            return v8::MaybeLocal<v8::String>();
        }
    }
    fclose(file);
    v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(
        isolate, chars, v8::NewStringType::kNormal, static_cast<int>(size));
    delete[] chars;
    return result;
}


// Process remaining command line arguments and execute files
int RunMain(v8::Isolate* isolate, v8::Platform* platform, int argc,
    char* argv[]) {
    for (int i = 1; i < argc; i++) {
        const char* str = argv[i];
        if (strcmp(str, "--shell") == 0) {
            run_shell = true;
        }
        else if (strcmp(str, "-f") == 0) {
            // Ignore any -f flags for compatibility with the other stand-
            // alone JavaScript engines.
            continue;
        }
        else if (strncmp(str, "--", 2) == 0) {
            fprintf(stderr,
                "Warning: unknown flag %s.\nTry --help for options\n", str);
        }
        else if (strcmp(str, "-e") == 0 && i + 1 < argc) {
            // Execute argument given to -e option directly.
            v8::Local<v8::String> file_name =
                v8::String::NewFromUtf8Literal(isolate, "unnamed");
            v8::Local<v8::String> source;
            if (!v8::String::NewFromUtf8(isolate, argv[++i]).ToLocal(&source)) {
                return 1;
            }
            bool success = ExecuteString(isolate, source, file_name, false, true);
            while (v8::platform::PumpMessageLoop(platform, isolate)) continue;
            if (!success) return 1;
        }
        else {
            // Use all other arguments as names of files to load and run.
            v8::Local<v8::String> file_name =
                v8::String::NewFromUtf8(isolate, str).ToLocalChecked();
            v8::Local<v8::String> source;
            if (!ReadFile(isolate, str).ToLocal(&source)) {
                fprintf(stderr, "Error reading '%s'\n", str);
                continue;
            }
            bool success = ExecuteString(isolate, source, file_name, false, true);
            while (v8::platform::PumpMessageLoop(platform, isolate)) continue;
            if (!success) return 1;
        }
    }
    return 0;
}


// The read-eval-execute loop of the shell.
void RunShell(v8::Local<v8::Context> context, v8::Platform* platform) {
    //fprintf(stderr, "V8 version %s [V8Shell]\n", v8::V8::GetVersion());
    std::cout << "[V8Shell 0.0] V8 version " << v8::V8::GetVersion() << std::endl;
    static const int kBufferSize = 256;
    // Enter the execution environment before evaluating any code.
    v8::Context::Scope context_scope(context);
    v8::Local<v8::String> name(
        v8::String::NewFromUtf8Literal(context->GetIsolate(), "(shell)"));
    while (true) {
        char buffer[kBufferSize];
        fprintf(stderr, "> ");
        char* str = fgets(buffer, kBufferSize, stdin);
        if (str == NULL) break;
        v8::HandleScope handle_scope(context->GetIsolate());
        ExecuteString(
            context->GetIsolate(),
            v8::String::NewFromUtf8(context->GetIsolate(), str).ToLocalChecked(),
            name, true, true);
        while (v8::platform::PumpMessageLoop(platform, context->GetIsolate()))
            continue;
    }
    fprintf(stderr, "\n");
}


// Executes a string within the current v8 context.
bool ExecuteString(v8::Isolate* isolate, v8::Local<v8::String> source,
    v8::Local<v8::Value> name, bool print_result,
    bool report_exceptions) {
    v8::HandleScope handle_scope(isolate);
    v8::TryCatch try_catch(isolate);
    v8::ScriptOrigin origin(isolate, name);
    v8::Local<v8::Context> context(isolate->GetCurrentContext());
    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(context, source, &origin).ToLocal(&script)) {
        // Print errors that happened during compilation.
        if (report_exceptions)
            ReportException(isolate, &try_catch);
        return false;
    }
    else {
        v8::Local<v8::Value> result;
        if (!script->Run(context).ToLocal(&result)) {
            assert(try_catch.HasCaught());
            // Print errors that happened during execution.
            if (report_exceptions)
                ReportException(isolate, &try_catch);
            return false;
        }
        else {
            assert(!try_catch.HasCaught());
            if (print_result && !result->IsUndefined()) {
                // If all went well and the result wasn't undefined then print
                // the returned value.
                v8::String::Utf8Value str(isolate, result);
                const char* cstr = ToCString(str);
                printf("%s\n", cstr);
            }
            return true;
        }
    }
}


void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
    v8::HandleScope handle_scope(isolate);
    v8::String::Utf8Value exception(isolate, try_catch->Exception());
    const char* exception_string = ToCString(exception);
    v8::Local<v8::Message> message = try_catch->Message();
    if (message.IsEmpty()) {
        // V8 didn't provide any extra information about this error; just
        // print the exception.
        fprintf(stderr, "%s\n", exception_string);
    }
    else {
        // Print (filename):(line number): (message).
        v8::String::Utf8Value filename(isolate,
            message->GetScriptOrigin().ResourceName());
        v8::Local<v8::Context> context(isolate->GetCurrentContext());
        const char* filename_string = ToCString(filename);
        int linenum = message->GetLineNumber(context).FromJust();
        fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
        // Print line of source code.
        v8::String::Utf8Value sourceline(
            isolate, message->GetSourceLine(context).ToLocalChecked());
        const char* sourceline_string = ToCString(sourceline);
        fprintf(stderr, "%s\n", sourceline_string);
        // Print wavy underline (GetUnderline is deprecated).
        int start = message->GetStartColumn(context).FromJust();
        for (int i = 0; i < start; i++) {
            fprintf(stderr, " ");
        }
        int end = message->GetEndColumn(context).FromJust();
        for (int i = start; i < end; i++) {
            fprintf(stderr, "^");
        }
        fprintf(stderr, "\n");
        v8::Local<v8::Value> stack_trace_string;
        if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
            stack_trace_string->IsString() &&
            stack_trace_string.As<v8::String>()->Length() > 0) {
            v8::String::Utf8Value stack_trace(isolate, stack_trace_string);
            const char* err = ToCString(stack_trace);
            fprintf(stderr, "%s\n", err);
        }
    }
}
