#pragma once

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <filesystem>

#include "libplatform/libplatform.h"
#include "v8-context.h"
#include "v8-exception.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-script.h"
#include "v8-template.h"

namespace fs = std::filesystem;

volatile struct RuntimeMemory {
  inline static fs::path current_directoy;
};

void SetCWD(fs::path path);

// Commands
void Print(const v8::FunctionCallbackInfo<v8::Value>& args);
void Read(const v8::FunctionCallbackInfo<v8::Value>& args);
void Load(const v8::FunctionCallbackInfo<v8::Value>& args);
void Quit(const v8::FunctionCallbackInfo<v8::Value>& args);
void Version(const v8::FunctionCallbackInfo<v8::Value>& args);
void ChangeDirectory(const v8::FunctionCallbackInfo<v8::Value>& args);

// Helper functions
v8::MaybeLocal<v8::String> ReadFile(v8::Isolate* isolate, const char* name);
bool ExecuteString(v8::Isolate* isolate, v8::Local<v8::String> source,
    v8::Local<v8::Value> name, bool print_result, bool report_exceptions);
void ReportException(v8::Isolate* isolate, v8::TryCatch* handler);
const char* ToCString(const v8::String::Utf8Value& value);
