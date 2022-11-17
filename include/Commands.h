#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <optional>

#include "libplatform/libplatform.h"
#include "v8.h"

#include "console.hpp"

namespace fs = std::filesystem;

namespace Commands {

volatile struct RuntimeMemory {
  inline static fs::path current_directoy;
};

void SetCWD(fs::path path);
fs::path GetCWD();
void PrintCWD();
void PrintErrorTag(std::ostream& stream = std::cerr);
void PrintWarningTag(std::ostream& stream = std::cerr);

// Commands
void Print(const v8::FunctionCallbackInfo<v8::Value>& args);
void Read(const v8::FunctionCallbackInfo<v8::Value>& args);
void Execute(const v8::FunctionCallbackInfo<v8::Value>& args);
void Quit(const v8::FunctionCallbackInfo<v8::Value>& args);
void Version(const v8::FunctionCallbackInfo<v8::Value>& args);
void ChangeDirectory(const v8::FunctionCallbackInfo<v8::Value>& args);
void ListFiles(const v8::FunctionCallbackInfo<v8::Value>& args);
void CreateNewFile(const v8::FunctionCallbackInfo<v8::Value>& args);
void StartProcessSync(const v8::FunctionCallbackInfo<v8::Value>& args);
void RemoveFile(const v8::FunctionCallbackInfo<v8::Value>& args);
void RemoveDir(const v8::FunctionCallbackInfo<v8::Value>& args);
void RemoveAny(const v8::FunctionCallbackInfo<v8::Value>& args);
void Rename(const v8::FunctionCallbackInfo<v8::Value>& args);
void Move(const v8::FunctionCallbackInfo<v8::Value>& args);
void Copy(const v8::FunctionCallbackInfo<v8::Value>& args);
void CreateNewDir(const v8::FunctionCallbackInfo<v8::Value>& args);
void Help(const v8::FunctionCallbackInfo<v8::Value>& args);

/* Scheduled for implementation:
void SetPermissions(const v8::FunctionCallbackInfo<v8::Value>& args);
void StartProcess(const v8::FunctionCallbackInfo<v8::Value>& args); */

// Helper functions
std::optional<v8::MaybeLocal<v8::String>> ReadFile(v8::Isolate* isolate, const char* name);
bool ExecuteString(v8::Isolate* isolate, v8::Local<v8::String> source,
  v8::Local<v8::Value> name, bool print_result, bool report_exceptions);
void ReportException(v8::Isolate* isolate, v8::TryCatch* handler);
const char* ToCString(const v8::String::Utf8Value& value);
void ConstructAbsolutePath(fs::path& path/*OUT*/);

};
