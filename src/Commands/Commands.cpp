#include "../../include/Commands.h"

namespace Commands {

// path + string = path
fs::path operator+(fs::path const& lhs, std::string const& rhs) {
  fs::path result = lhs;
  result.append(rhs);

  return result;
}

void SetCWD(fs::path path) {
  RuntimeMemory::current_directoy = path;
}

fs::path GetCWD() {
  return RuntimeMemory::current_directoy;
}

void PrintCWD() {
  auto path = RuntimeMemory::current_directoy.generic_string();
  std::cout << rang::fg::green << path << "> " << rang::fg::reset;
}

void PrintErrorTag(std::ostream& stream) {
  stream << rang::fg::red << rang::style::bold << "[Error]"
    << rang::style::reset << rang::fg::reset;
}

void PrintWarningTag(std::ostream& stream) {
  stream << rang::fg::yellow << rang::style::bold << "[Warning]"
    << rang::style::reset << rang::fg::reset;
}

// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Sends its arguments to stdout separated by
// semicolons and ending with a newline.
void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    if (first) {
      first = false;
    }
    else {
      std::cout << ";";
    }
    v8::String::Utf8Value str(args.GetIsolate(), args[i]);
    std::cout << ToCString(str);
  }

  std::cout << std::endl;
}


// The callback that is invoked by v8 whenever the JavaScript 'read'
// function is called.  This function loads the content of the file named in
// the argument into a JavaScript string.
void Read(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() != 1) {
    args.GetIsolate()->ThrowError("[Error] Bad parameters");
    return;
  }
  v8::String::Utf8Value file(args.GetIsolate(), args[0]);
  if (*file == NULL) {
    args.GetIsolate()->ThrowError("[Error] No file name given");
    return;
  }
  v8::Local<v8::String> source;
  if (!ReadFile(args.GetIsolate(), *file).ToLocal(&source)) {
    args.GetIsolate()->ThrowError("[Error] Cannot load file content");
    return;
  }

  args.GetReturnValue().Set(source);
}

// The callback that is invoked by v8 whenever the JavaScript 'load'
// function is called. Loads, compiles and executes its argument JavaScript file.
void Load(const v8::FunctionCallbackInfo<v8::Value>& args) {
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    v8::String::Utf8Value file(args.GetIsolate(), args[i]);
    if (*file == NULL) {
      args.GetIsolate()->ThrowError("[Error] No file name given");
      return;
    }
    v8::Local<v8::String> source;
    if (!ReadFile(args.GetIsolate(), *file).ToLocal(&source)) {
      args.GetIsolate()->ThrowError("[Error] Cannot load file content");
      return;
    }
    if (!ExecuteString(args.GetIsolate(), source, args[i], false, false)) {
      args.GetIsolate()->ThrowError("[Error] Failure to execute file content");
      return;
    }
  }
}


// The callback that is invoked by v8 whenever the JavaScript 'quit'
// function is called. Terminates execution.
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
  args.GetReturnValue().Set(v8::String::NewFromUtf8(args.GetIsolate(),
    v8::V8::GetVersion()).ToLocalChecked());
}

// Changes the current directory to operate on
// Inputting a number will go up this many parent directories
// Inputting a string will attempt to enter that subdirectory
void ChangeDirectory(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() == 0) {
    PrintCWD();

    return;
  }

  v8::HandleScope handle_scope(args.GetIsolate());
  auto* isolate = args.GetIsolate();

  if (args[0]->IsNumber()) {
    const auto js_value = args[0]->Int32Value(isolate->GetCurrentContext());

    if (js_value.IsNothing()) {
      args.GetIsolate()->ThrowError("[Error] Cannot deduce an argument value");
      std::cout << std::endl;

      return;
    }

    // Extract positive Integer
    const auto num = abs(js_value.FromJust());

    for (auto i = 0; i < num; i++) {
      RuntimeMemory::current_directoy = RuntimeMemory::current_directoy.parent_path();
    }
  }

  if (args[0]->IsString()) {
    const auto js_value = args[0]->ToString(isolate->GetCurrentContext());

    if (js_value.IsEmpty()) {
      isolate->ThrowError("[Error] Cannot deduce an argument value");
      std::cout << std::endl;

      return;
    }

    v8::String::Utf8Value str(isolate, args[0]);
    auto value = std::string(ToCString(str));

    fs::path trial_path = RuntimeMemory::current_directoy + value;
    if (!fs::is_directory(trial_path)) {
      PrintErrorTag();
      std::cerr << " " << trial_path.generic_string() << " is not a directory" << std::endl;

      return;
    }

    // path::append operates in-place
    RuntimeMemory::current_directoy.append(value);
  }
}

// ls(printToStd = true), ls() -> returns void and prints to std
// ls(printToStd = false) -> returns file list, doesnt print to std
void ListFiles(const v8::FunctionCallbackInfo<v8::Value>& args) {
  bool print_to_std = true;
  v8::HandleScope handle_scope(args.GetIsolate());
  auto* isolate = args.GetIsolate();

  if (args.Length() != 0) {
    if (args[0]->IsBoolean() && !(args[0]->ToBoolean(isolate)->BooleanValue(isolate))) {
      print_to_std = false;
    }
  }
  
  v8::Local<v8::Array> result = v8::Array::New(isolate, 3);
  auto result_index = 0;

  for (auto const& dir_entry :
        std::filesystem::directory_iterator(RuntimeMemory::current_directoy)) {
    const auto filename = dir_entry.path().filename().generic_string();

    if (dir_entry.is_directory()) {
      if (print_to_std) {
        std::cout << rang::fg::cyan;
      } else {
        auto object = v8::Object::New(isolate);
        object->Set(isolate->GetCurrentContext(),
          v8::String::NewFromUtf8(isolate, "isDirectory").ToLocalChecked(),
          v8::Boolean::New(isolate, true));

        object->Set(isolate->GetCurrentContext(),
        v8::String::NewFromUtf8(isolate, "filename").ToLocalChecked(),
            v8::String::NewFromUtf8(isolate, filename.c_str()).ToLocalChecked());

        result->Set(isolate->GetCurrentContext(), result_index, object);
        result_index++;
      }
    }

    if (print_to_std) {
      std::cout << filename << std::endl;
      std::cout << rang::fg::reset;
    } else {
      auto object = v8::Object::New(isolate);
      object->Set(
          isolate->GetCurrentContext(),
          v8::String::NewFromUtf8(isolate, "isDirectory").ToLocalChecked(),
          v8::Boolean::New(isolate, false));

      object->Set(
          isolate->GetCurrentContext(),
          v8::String::NewFromUtf8(isolate, "filename").ToLocalChecked(),
          v8::String::NewFromUtf8(isolate, filename.c_str()).ToLocalChecked());

      result->Set(isolate->GetCurrentContext(), result_index, object);
      result_index++;
    }
  }

  if (!print_to_std) {
    args.GetReturnValue().Set(result);
  }
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

// Parses and executes a string within the current v8 context.
bool ExecuteString(v8::Isolate* isolate, v8::Local<v8::String> source,
  v8::Local<v8::Value> name, bool print_result, bool report_exceptions) {
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
      if (print_result) {
        if (!result->IsUndefined()) {
          // If all went well and the result wasn't undefined then print
          // the returned value.
          v8::String::Utf8Value str(isolate, result);
          std::cout << ToCString(str) << std::endl;
        } 
        PrintCWD();
      } 
      return true;
    }
  }
}

// Runs a v8 stack trace to get what lead to the exception
void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope(isolate);
  v8::String::Utf8Value exception(isolate, try_catch->Exception());
  const char* exception_string = ToCString(exception);
  v8::Local<v8::Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    std::cerr << exception_string << std::endl;
  }
  else {
    // Print (filename):(line number) - (message).
    v8::String::Utf8Value filename(isolate,
      message->GetScriptOrigin().ResourceName());
    v8::Local<v8::Context> context(isolate->GetCurrentContext());
    const auto* filename_string = ToCString(filename);
    int linenum = message->GetLineNumber(context).FromJust();
    std::cerr << filename_string << ":" << linenum << " - " << exception_string
      << std::endl;

    // Print line of faulty source code.
    v8::String::Utf8Value sourceline(
      isolate, message->GetSourceLine(context).ToLocalChecked());
    std::cerr << ToCString(sourceline) << std::endl;

    // Print wavy underline
    int start = message->GetStartColumn(context).FromJust();
    for (int i = 0; i < start; i++) {
      std::cerr << " ";
    }
    int end = message->GetEndColumn(context).FromJust();
    for (int i = start; i < end; i++) {
      std::cerr << "^";
    }
    std::cerr << std::endl;

    v8::Local<v8::Value> stack_trace_string;
    if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
      stack_trace_string->IsString() &&
      stack_trace_string.As<v8::String>()->Length() > 0) {
      v8::String::Utf8Value stack_trace(isolate, stack_trace_string);
      std::cerr << ToCString(stack_trace) << std::endl;
    }

    PrintCWD();
  }
}

// Extracts the underlying C string from a V8 Utf8Value
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "[Error] string conversion failed";
}

};
