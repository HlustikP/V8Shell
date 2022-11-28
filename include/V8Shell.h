#pragma once

#include <tuple>
#include <vector>

#include "Commands.h"

struct Settings {
  bool run_shell;
  inline const static std::string current_version = "0.4.0";
};


class V8Shell {
 public:
  V8Shell(int argc, const char** argv, int& exit_code /*OUT*/);
  ~V8Shell();

  // V8 isolates are supposed to be unique per process
  V8Shell(const V8Shell&) = delete;
  V8Shell operator=(const V8Shell&) = delete;

  int Run();
  bool AddHook(std::tuple<std::string, v8::FunctionCallback>& hook);
  bool RemoveHook(std::string& js_function);
  bool RemoveHook(v8::FunctionCallback cb);
 private:
  bool SetupV8Isolate();
  v8::Local<v8::Context> CreateShellContext();
  void RunShell(v8::Local<v8::Context> context);

  inline static std::vector<std::tuple<std::string, v8::FunctionCallback>>
    cpp_hooks {
                std::tuple("print", &Commands::Print),
                std::tuple("read", &Commands::Read),
                std::tuple("execute", &Commands::Execute),
                std::tuple("quit", &Commands::Quit),
                std::tuple("exit", &Commands::Quit),
                std::tuple("version", &Commands::Version),
                std::tuple("cd", &Commands::ChangeDirectory),
                std::tuple("changeDirectory", &Commands::ChangeDirectory),
                std::tuple("changeDir", &Commands::ChangeDirectory),
                std::tuple("ls", &Commands::ListFiles),
                std::tuple("ll", &Commands::ListFiles),
                std::tuple("runSync", &Commands::StartProcessSync),
                std::tuple("createFile", &Commands::CreateNewFile),
                std::tuple("touch", &Commands::CreateNewFile),
                std::tuple("removeFile", &Commands::RemoveFile),
                std::tuple("rf", &Commands::RemoveFile),
                std::tuple("removeDirectory", &Commands::RemoveDir),
                std::tuple("removeDir", &Commands::RemoveDir),
                std::tuple("rd", &Commands::RemoveDir),
                std::tuple("rm", &Commands::RemoveAny),
                std::tuple("rename", &Commands::Rename),
                std::tuple("move", &Commands::Move),
                std::tuple("mv", &Commands::Move),
                std::tuple("copy", &Commands::Copy),
                std::tuple("cp", &Commands::Copy),
                std::tuple("mkdir", &Commands::CreateNewDir),
                std::tuple("createDirectory", &Commands::CreateNewDir),
                std::tuple("createDir", &Commands::CreateNewDir),
                std::tuple("help", &Commands::Help)};
  int argc_;
  const char** argv_;

  std::unique_ptr<v8::Platform> platform_;
  v8::Isolate::CreateParams create_params_;
  v8::Isolate* isolate_;
  Settings settings_;
};
