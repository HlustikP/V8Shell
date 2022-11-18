#include "../../include/Commands.h"

namespace Commands {

// path + string = path
fs::path operator+(fs::path const& lhs, std::string const& rhs) {
	fs::path result = lhs;
	result.append(rhs);

	return result;
}

/** Setter for the current working directory of the shell. */
void SetCWD(fs::path path) { RuntimeMemory::current_directoy = path; }

/** Getter for the current working directory of the shell. */
fs::path GetCWD() { return RuntimeMemory::current_directoy; }

/** Prints colorized cwd to standard out. */
void PrintCWD() {
	auto path = RuntimeMemory::current_directoy.generic_string();
	std::cout << rang::fg::green << path << "> " << rang::fg::reset;
}

/** Prints a colorized error tag. Used to prepend error messages. */
void PrintErrorTag(std::ostream& stream) {
	stream << rang::fg::red << rang::style::bold << "[Error]"
				 << rang::style::reset << rang::fg::reset;
}

/** Prints a colorized warning tag. Used to prepend warning messages. */
void PrintWarningTag(std::ostream& stream) {
	stream << rang::fg::yellow << rang::style::bold << "[Warning]"
				 << rang::style::reset << rang::fg::reset;
}

/** The callback that is invoked by v8 whenever the JavaScript 'print'
 *   function is called. Sends its arguments to stdout separated by
 *   semicolons and ending with a newline. */
void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
	bool first = true;
	for (int i = 0; i < args.Length(); i++) {
		v8::HandleScope handle_scope(args.GetIsolate());
		if (first) {
			first = false;
		} else {
			std::cout << ";";
		}
		v8::String::Utf8Value str(args.GetIsolate(), args[i]);
		std::cout << ToCString(str);
	}

	std::cout << std::endl;
}

/** The callback that is invoked by v8 whenever the JavaScript 'read'
 *   function is called. This function loads the content of the file passed
 *   as a string in argument 0 into a JavaScript string. */
void Read(const v8::FunctionCallbackInfo<v8::Value>& args) {
	if (args.Length() != 1) {
		args.GetIsolate()->ThrowError("[Error] Bad parameters");
		return;
	}
	v8::String::Utf8Value file(args.GetIsolate(), args[0]);
	if (*file == NULL) {
		args.GetIsolate()->ThrowError("[Error] No file name passed");
		return;
	}
	v8::Local<v8::String> source;
	auto file_content = ReadFile(args.GetIsolate(), *file);
	if (!file_content.has_value()) {
		args.GetIsolate()->ThrowError("[Error] Cannot load file content");
		return;
	}

	file_content.value().ToLocal(&source);
	args.GetReturnValue().Set(source);
}

/** The callback that is invoked by v8 whenever the JavaScript 'execute'
 *   function is called. Loads, parses, compiles and executes its argument
 *   JavaScript file. */
void Execute(const v8::FunctionCallbackInfo<v8::Value>& args) {
	for (int i = 0; i < args.Length(); i++) {
		v8::HandleScope handle_scope(args.GetIsolate());
		v8::String::Utf8Value file(args.GetIsolate(), args[i]);
		if (*file == NULL) {
			args.GetIsolate()->ThrowError("[Error] No file name given");
			return;
		}
		v8::Local<v8::String> source;
		auto file_content = ReadFile(args.GetIsolate(), *file);
		if (!file_content.has_value()) {
			args.GetIsolate()->ThrowError("[Error] Cannot load file content");
			return;
		}
		file_content.value().ToLocal(&source);

		if (!ExecuteString(args.GetIsolate(), source, args[i], false, false)) {
			args.GetIsolate()->ThrowError("[Error] Failure to execute file content");
			return;
		}
	}
}

/** The callback that is invoked by v8 whenever the JavaScript 'quit'
 *   function is called. Terminates execution. */
void Quit(const v8::FunctionCallbackInfo<v8::Value>& args) {
	// If not arguments are given args[0] will yield undefined which
	// is coerced into the integer value 0.
	int exit_code =
			args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);
	fflush(stdout);
	fflush(stderr);
	exit(exit_code);
}

/** The callback that is invoked by v8 whenever the JavaScript 'version'
 *  function is called. Returns a string with the version of the embedded v8
 *  engine. */
void Version(const v8::FunctionCallbackInfo<v8::Value>& args) {
	args.GetReturnValue().Set(
			v8::String::NewFromUtf8(args.GetIsolate(), v8::V8::GetVersion())
					.ToLocalChecked());
}

/** The callback that is invoked by v8 whenever the JavaScript 'cd'
 *  function is called. Changes the current directory to operate on.
 *  Inputting a number will go up this many parent directories and
 *  inputting a string will attempt to enter a subdirectory. */
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
			RuntimeMemory::current_directoy =
					RuntimeMemory::current_directoy.parent_path();
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

		fs::path try_path = fs::path(value).is_absolute()
														? fs::path(value)
														: RuntimeMemory::current_directoy + value;

		if (!fs::is_directory(try_path)) {
			PrintErrorTag();
			std::cerr << " " << try_path.generic_string() << " is not a directory"
								<< std::endl;

			return;
		}

		RuntimeMemory::current_directoy = try_path;
	}
}

/** The callback that is invoked by v8 whenever the JavaScript 'ls'
 *  function is called. Returns void and prints the content of the
 *  current working directory to standard out.
 *  Returns an array of directory entry objects if 'true' is passed. */
void ListFiles(const v8::FunctionCallbackInfo<v8::Value>& args) {
	bool print_to_std = true;
	v8::HandleScope handle_scope(args.GetIsolate());
	auto* isolate = args.GetIsolate();

	if (args.Length() != 0) {
		if (args[0]->IsBoolean() &&
				!(args[0]->ToBoolean(isolate)->BooleanValue(isolate))) {
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
				object->Set(
						isolate->GetCurrentContext(),
						v8::String::NewFromUtf8(isolate, "isDirectory").ToLocalChecked(),
						v8::Boolean::New(isolate, true));

				object->Set(
						isolate->GetCurrentContext(),
						v8::String::NewFromUtf8(isolate, "filename").ToLocalChecked(),
						v8::String::NewFromUtf8(isolate, filename.c_str())
								.ToLocalChecked());

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

/** The callback that is invoked by v8 whenever the JavaScript 'createFile'
 *  function is called. Creates a new file in the cwd. */
void CreateNewFile(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value file(args.GetIsolate(), args[0]);
	auto filename = fs::path(ToCString(file));
  ConstructAbsolutePath(filename);

	if (fs::exists(filename)) {
		PrintErrorTag();
		std::cerr << " File " << filename << " already exists." << std::endl;

		return;
	}

	std::ofstream out_filestream(filename);
	out_filestream.close();
}

/** The callback that is invoked by v8 whenever the JavaScript 'removeFile'
 *  function is called. Deletes the file mentioned in arg[0]. */
void RemoveFile(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value file(args.GetIsolate(), args[0]);
	auto filename = fs::path(ToCString(file));
  ConstructAbsolutePath(filename);
	
	if (!fs::exists(filename)) {
	  PrintErrorTag();
	  std::cerr << " File " << rang::style::bold << filename << rang::style::reset
						  << " doesnt exists." << std::endl;

		return;
	}
	if (fs::is_directory(filename)) {
		PrintErrorTag();
		std::cerr << " Entity " << rang::style::bold << filename << rang::style::reset << " is a directory."
							<< " Try removeDir('" << filename << "') or rm('"
							<< filename << "') instead."
									 
			<< std::endl;

		return;
	}

	std::error_code err;
	auto OK = fs::remove(filename, err);
	if (!OK) {
		PrintErrorTag();
		std::cerr << " " << std::system_category().message(GetLastError())
							<< std::endl;
	}
}

/** The callback that is invoked by v8 whenever the JavaScript 'removeDir'
 *  function is called. Recursively deletes the directory mentioned in arg[0]. */
void RemoveDir(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value dir(args.GetIsolate(), args[0]);
	auto dirname = fs::path(ToCString(dir));
  ConstructAbsolutePath(dirname);

	if (!fs::exists(dirname)) {
		PrintErrorTag();
		std::cerr << " Directory " << rang::style::bold << dirname
							<< rang::style::reset << " doesnt exists." << std::endl;

		return;
	}
	if (!fs::is_directory(dirname)) {
		PrintErrorTag();
		std::cerr << " Entity " << rang::style::bold << dirname
							<< rang::style::reset << " is a file."
							<< " Try removeFile('" << dirname << "') or rm('" << dirname
							<< "') instead."

							<< std::endl;

		return;
	}

	std::error_code err;
	auto OK = fs::remove_all(dirname, err);
	if (!OK) {
		PrintErrorTag();
		std::cerr << " " << std::system_category().message(GetLastError())
							<< std::endl;
	}
}

/** The callback that is invoked by v8 whenever the JavaScript 'rm'
 *  function is called. (Recursively) deletes the path entity mentioned in arg[0]. */
void RemoveAny(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value path_entity(args.GetIsolate(), args[0]);
	auto pathname = fs::path(ToCString(path_entity));
	ConstructAbsolutePath(pathname);

	std::error_code err;
	auto OK = fs::remove_all(pathname, err);
	if (!OK) {
		PrintErrorTag();
		std::cerr << " " << std::system_category().message(GetLastError())
							<< std::endl;
	}
}

/** The callback that is invoked by v8 whenever the JavaScript 'rename'
 *  function is called. Renames the passed file or directory
 *  to the new name in arg[1]. This function guarantees that the new
 *  entity is in the same directory as the old one */
void Rename(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value path_entity(args.GetIsolate(), args[0]);
	auto old_pathname = fs::path(ToCString(path_entity));
	ConstructAbsolutePath(old_pathname);

	v8::String::Utf8Value new_path_entity(args.GetIsolate(), args[1]);
	auto new_pathname = fs::path(ToCString(new_path_entity));
	ConstructAbsolutePath(new_pathname);

	if (new_pathname.parent_path() != old_pathname.parent_path()) {
		PrintErrorTag();
		std::cerr << " Tried to move file " << rang::style::bold << old_pathname
							<< rang::style::reset << " to new location."
							<< " Use the move('from', 'to') function instead." << std::endl;

		return;
	}

	std::error_code err;
	fs::rename(old_pathname, new_pathname, err);

	if (err.value() != 0) {
		PrintErrorTag();
		std::cerr << " " << std::system_category().message(GetLastError())
							<< std::endl;
	}
}

/** The callback that is invoked by v8 whenever the JavaScript 'move'
 *  function is called. Moves the passed file or directory
 *  to the new path in arg[1]. */
void Move(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value path_entity(args.GetIsolate(), args[0]);
	auto old_path = fs::path(ToCString(path_entity));
	ConstructAbsolutePath(old_path);

	v8::String::Utf8Value new_path_entity(args.GetIsolate(), args[1]);
	auto new_path = fs::path(ToCString(new_path_entity));
	ConstructAbsolutePath(new_path);

	std::error_code err;
	fs::rename(old_path, new_path, err);

	if (err.value() != 0) {
		PrintErrorTag();
		std::cerr << " " << std::system_category().message(GetLastError())
							<< std::endl;
	}
}

/** The callback that is invoked by v8 whenever the JavaScript 'copy'
 *  function is called. Copies the passed file or directory
 *  to the new path in arg[1]. */
void Copy(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value path_entity(args.GetIsolate(), args[0]);
	auto source_path = fs::path(ToCString(path_entity));
	ConstructAbsolutePath(source_path);

	v8::String::Utf8Value new_path_entity(args.GetIsolate(), args[1]);
	auto dest_path = fs::path(ToCString(new_path_entity));
	ConstructAbsolutePath(dest_path);

	std::error_code err;
	fs::copy(source_path, dest_path, err);

	if (err.value() != 0) {
		PrintErrorTag();
		std::cerr << " " << std::system_category().message(GetLastError())
							<< std::endl;
	}
}

/** The callback that is invoked by v8 whenever the JavaScript 'mkdir'
 *  function is called. Creates a new directory. */
void CreateNewDir(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::String::Utf8Value dirname(args.GetIsolate(), args[0]);
	auto new_dir = fs::path(ToCString(dirname));
	ConstructAbsolutePath(new_dir);

	std::error_code err;
	fs::create_directory(new_dir, err);

	if (err.value() != 0) {
		PrintErrorTag();
		std::cerr << " " << std::system_category().message(GetLastError())
							<< std::endl;
	}
}

/** The callback that is invoked by v8 whenever the JavaScript 'runSync'
 *  function is called. Creates a child process and halts execution
 *  of the shell until the child process terminates.
 *  First argument is the application to be executed which is first
 *  looked for in the current executable's directory, otherwise the
 *  PATH is searched. Second argument is an optional additional
 *  object with parameeters to be passed to the executable.
 *  Third argument is controls verbosity of this functions. */
void StartProcessSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto* isolate = args.GetIsolate();
	auto* context =  &(isolate->GetCurrentContext());
	bool verbose = true;

	if (!(args[0]->IsString())) {
		PrintErrorTag();
		std::cerr << " No executable filename or path passed!" << std::endl;

		return;
	}

	std::string params_appendage = "";

	// Format additional parameters
	if (args.Length() > 1 && (args[1]->IsObject())) {
		auto object = args[1]->ToObject(isolate->GetCurrentContext()).FromMaybe(
			v8::Object::New(isolate));
		auto params = object->GetOwnPropertyNames(isolate->GetCurrentContext()).FromMaybe(
			v8::Array::New(isolate));
		auto params_count = params->Length();

		// Iterate over object entries
		for (unsigned int i = 0; i < params_count; i++) {
			auto param = params->Get(*context, i).ToLocalChecked();

			// Parse Object key-val pairs as strings
			v8::String::Utf8Value parameter(isolate, param);
			v8::String::Utf8Value value(
					isolate, object->Get(*context, param).ToLocalChecked());

			// -h vs --help
			params_appendage.append(parameter.length() == 1 ? " -" : " --");
			params_appendage.append(ToCString(parameter)).append(" ")
				.append(ToCString(value));
		}   
	}

	if (args.Length() > 2) {
		if (args[2]->IsBoolean() &&
				!(args[2]->ToBoolean(isolate)->BooleanValue(isolate))) {
			 verbose = false;
		}
	}

	// C style shortcut si.cb = sizeof(ci)
	STARTUPINFO si = {sizeof(si)};

	PROCESS_INFORMATION pi;

	v8::String::Utf8Value str(isolate, args[0]);
	std::string process_command = ToCString(str);

	// Check if cwd contains a file with that name
	fs::path try_local_file = RuntimeMemory::current_directoy;
	try_local_file.append(process_command);
	if (fs::exists(try_local_file) && !fs::is_directory(try_local_file)) {
		process_command = try_local_file.generic_string();
	}

	process_command.append(params_appendage);

	auto OK = CreateProcessA(nullptr,
		const_cast<char*>(process_command.c_str()),
		nullptr,
		nullptr,
		FALSE,
		0,                     
		NULL, 
		NULL,        
		&si, 
		&pi);

	// check if Windows was able to spawn a new child process
	if (OK) {
		if (verbose) {
			 std::cout << "Process with PID " << pi.dwProcessId
								 << " is currently running..." << std::endl;
		}

		// Wait untill Process object is signaled (usually when child process terminates)
		auto status = WaitForSingleObject(pi.hProcess, INFINITE);

		if (status == WAIT_OBJECT_0 && verbose) {
			std::cout << "Process " << pi.dwProcessId << " ended execution!"
								<< std::endl;
		}

		// Handles must be explicitly closed. If not, the parent process will
		// hold on to it even if the child process is terminated.
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	} else {
		PrintErrorTag();
		std::cerr << " " << std::system_category().message(GetLastError())
							<< std::endl;
	}
}

/** The callback that is invoked by v8 whenever the JavaScript 'help'
 *  function is called. Prints available shell functions. */
void Help(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::cout << "~Available functions (refer to the docs for further informations):~" 
						<< std::endl;
	std::cout << rang::style::underline << "File System:" << rang::style::reset 
						<< std::endl;
	std::cout
			<< rang::fg::magenta << "cd(path)/changeDirectory(path)" << rang::style::reset
			<< " - Changes the current working directory."
			<< " If a number is passed, the cwd goes up that many parent directories."
			<< std::endl << rang::fg::magenta << "ls(printToStd = true)/ll(printToStd = true)"
			<< rang::style::reset << " - Prints all files and directories"
			<< " in the current working directory. If 'false' is passed, it prints an array of"
			<< " objects describing the directory content."
			<< std::endl << rang::fg::magenta << "createFile(filename)/touch(filename)"
			<< rang::style::reset << " - Creates a new file."
			<< std::endl << rang::fg::magenta << "createDirectory(filename)/mkdir(filename)"
			<< rang::style::reset << " - Creates a new directory."
			<< std::endl << rang::fg::magenta << "removeFile(filename)/rf(filename)"
			<< rang::style::reset << " - Removes a file."
			<< std::endl << rang::fg::magenta << "removeDir(dirname)/rd(dirname)"
			<< rang::style::reset
			<< " - Recursively removes a Directory and it's contents."
			<< std::endl << rang::fg::magenta << "rm(entity)" << rang::style::reset
			<< " - Removes a file or recursively a directory and it's contents."
			<< std::endl << rang::fg::magenta << "rename(entity)" << rang::style::reset
			<< " - Renames the file or directory."
			<< std::endl << rang::fg::magenta << "move(from, to)/mv(from, to)"
			<< rang::style::reset << " - Moves a file or directory to a new location."
			<< std::endl << rang::fg::magenta << "copy(from, to)" << rang::style::reset
			<< " - Copies a file or directory to a new location."
			<< std::endl;

	std::cout << rang::style::underline << "Execution:" << rang::style::reset 
						<< std::endl;
	std::cout
			<< rang::fg::magenta << "execute(filename)" << rang::style::reset <<
			" - Reads, parses, compiles and executes a .js file."
			<< std::endl
			<< rang::fg::magenta << "runSync(filename, parameters, verbose = true)"
			<< rang::style::reset << " - Spawns a child process executing"
			<< " the chosen file. The 'parameters' argument is an optional object with additional"
			<< " parameters passed to the executable. Holds execution of the shell until the child"
			<< " process terminates and redirects standard streams to the shell."
			<< std::endl;

	std::cout << rang::style::underline << "General Functions:" << rang::style::reset 
						<< std::endl;
	std::cout << rang::fg::magenta << "print(expression)" << rang::style::reset 
			<< " - Prints the result of a JavaScript Expression."
			<< std::endl
			<< rang::fg::magenta << "read(filename)" << rang::style::reset <<
			" - Reads a file and returns it's content as a string."
			<< std::endl
			<< rang::fg::magenta << "quit()/exit()" << rang::style::reset
			<< " - Terminates the shell."
			<< std::endl
			<< rang::fg::magenta << "version()" << rang::style::reset
			<< " - Returns a string with the used v8 engine version."
			<< std::endl;
}

/** Reads the content of a file into a v8 string. */
std::optional<v8::MaybeLocal<v8::String>> ReadFile(v8::Isolate* isolate, const char* name) {
	std::ifstream input_file;
	input_file.open(name, std::ios::binary);
	if (!input_file) {
		PrintErrorTag();
		std::cerr << " Cannot open input file " << name << std::endl;

		return std::nullopt;
	}

	// get length of file:
	input_file.seekg(0, input_file.end);
	const auto size = input_file.tellg();

	// tellg() returns -1 if it fails
	if (size == -1) {
		PrintErrorTag();
		std::cerr << " Cannot tellg() file content length" << std::endl;

		return std::nullopt;
	}

	// reqind to beginning
	input_file.seekg(0, std::ifstream::beg);
	auto* buffer = new char[size];

	input_file.read(buffer, static_cast<int>(size));
	input_file.close();

	v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(
		isolate, buffer, v8::NewStringType::kNormal, static_cast<int>(size));

	delete[] buffer;
	return result;
}

/** Parses and executes a string within the current v8 context. */
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

/** Runs a v8 stack trace and reports the exception together with
*   the code that caused it. */
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

		v8::Local<v8::Value> stack_trace;
		if (try_catch->StackTrace(context).ToLocal(&stack_trace) &&
				stack_trace->IsString() && stack_trace.As<v8::String>()->Length() > 0) {
			v8::String::Utf8Value stack_trace_result(isolate, stack_trace);
			std::cerr << ToCString(stack_trace_result) << std::endl;
		}

		PrintCWD();
	}
}

/** Extracts the underlying C string from a v8 Utf8Value. */
const char* ToCString(const v8::String::Utf8Value& value) {
	return *value ? *value : "[Error] string conversion failed";
}

/** Constructs a path relative to the cwd if path is relative. */
void ConstructAbsolutePath(fs::path& path /*IN-OUT*/) {
	// Nothing needs to be done if path is already absolute
	if (path.is_absolute()) {
		return;
	}

	fs::path cwd = RuntimeMemory::current_directoy;
	path = cwd.append(path.generic_string());
}

};
