#pragma once

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "libplatform/libplatform.h"
#include "v8-context.h"
#include "v8-exception.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-script.h"
#include "v8-template.h"

std::unique_ptr<v8::Platform> SetupV8(int argc, char* argv[]);
bool SetupV8Isolate(v8::Isolate::CreateParams* create_params, v8::Isolate** isolate);

v8::Local<v8::Context> CreateShellContext(v8::Isolate* isolate);
void RunShell(v8::Local<v8::Context> context, v8::Platform* platform);
int RunMain(v8::Isolate* isolate, v8::Platform* platform, int argc,
    char* argv[]);
bool ExecuteString(v8::Isolate* isolate, v8::Local<v8::String> source,
    v8::Local<v8::Value> name, bool print_result,
    bool report_exceptions);
void Print(const v8::FunctionCallbackInfo<v8::Value>& args);
void Read(const v8::FunctionCallbackInfo<v8::Value>& args);
void Load(const v8::FunctionCallbackInfo<v8::Value>& args);
void Quit(const v8::FunctionCallbackInfo<v8::Value>& args);
void Version(const v8::FunctionCallbackInfo<v8::Value>& args);
v8::MaybeLocal<v8::String> ReadFile(v8::Isolate* isolate, const char* name);
void ReportException(v8::Isolate* isolate, v8::TryCatch* handler);

bool run_shell;
