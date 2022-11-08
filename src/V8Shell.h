#pragma once

#include "Commands.h"

struct Settings {
  bool run_shell;
};

const auto current_version = "0.0.0";

std::unique_ptr<v8::Platform> SetupV8(int argc, char* argv[]);
bool SetupV8Isolate(v8::Isolate::CreateParams* create_params, v8::Isolate** isolate);
void Cleanup(v8::Isolate* isolate, v8::Isolate::CreateParams* create_params);
v8::Local<v8::Context> CreateShellContext(v8::Isolate* isolate);

void RunShell(v8::Local<v8::Context> context, v8::Platform* platform);
int RunMain(v8::Isolate* isolate, v8::Platform* platform, int argc,
    char* argv[], Settings& settings);
