#pragma once

#include "Commands.h"

struct Settings {
  bool run_shell;
};

const auto current_version = "0.2.0";

// c++ hooks
static inline std::vector<std::tuple<std::string, v8::FunctionCallback>>
  cpp_hooks {
  std::tuple("print", Commands::Print),
  std::tuple("read", Commands::Read),
  std::tuple("execute", Commands::Execute),
  std::tuple("quit", Commands::Quit),
  std::tuple("exit", Commands::Quit),
  std::tuple("version", Commands::Version),
  std::tuple("cd", Commands::ChangeDirectory),
  std::tuple("changeDirectory", Commands::ChangeDirectory),
  std::tuple("ls", Commands::ListFiles),
  std::tuple("ll", Commands::ListFiles),
  std::tuple("runSync", Commands::StartProcessSync),
  std::tuple("createFile", Commands::CreateNewFile),
  std::tuple("touch", Commands::CreateNewFile),
  std::tuple("removeFile", Commands::RemoveFile),
  std::tuple("rf", Commands::RemoveFile),
  std::tuple("removeDir", Commands::RemoveDir),
  std::tuple("rd", Commands::RemoveDir),
  std::tuple("rm", Commands::RemoveAny)
};

std::unique_ptr<v8::Platform> SetupV8(int argc, char* argv[]);
bool SetupV8Isolate(v8::Isolate::CreateParams* create_params, v8::Isolate** isolate);
void Cleanup(v8::Isolate* isolate, v8::Isolate::CreateParams* create_params);
v8::Local<v8::Context> CreateShellContext(v8::Isolate* isolate);

void RunShell(v8::Local<v8::Context> context, v8::Platform* platform);
int RunMain(v8::Isolate* isolate, v8::Platform* platform, int argc,
    char* argv[], Settings& settings);
