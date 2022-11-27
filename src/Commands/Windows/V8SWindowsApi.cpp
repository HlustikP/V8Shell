// This File contains functions with Windows-specific api calls
#include "V8SWindowsApi.h"

namespace Commands {

void CreateNewProcess(std::string& process_path, std::vector<std::string>& args, bool verbose) {
  // C style shortcut si.cb = sizeof(ci)
  STARTUPINFO si = { sizeof(si) };

  PROCESS_INFORMATION pi;

  std::string process_command;

  process_command.append(process_path);
  for (auto& arg : args) {
    process_command.append(" ").append(arg);
  }

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
  }
  else {
    std::cerr << std::system_category().message(GetLastError())
      << std::endl;
  }
}

};
