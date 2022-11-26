// This File contains functions with Linux-specific api calls

#include "V8SLinuxApi.h"

extern char** environ;

namespace Commands {

void CreateNewProcess(std::string& process_path, std::vector<std::string>& args, bool verbose) {
  pid_t pid;

  std::vector<const char*> argv;
  // Add two extra spaces (process name and nullptr terminator)
  argv.reserve(args.size() + 2);

  argv.push_back(process_path.c_str());
  for (auto& arg : args) {
    argv.push_back(arg.c_str());
  }
  // Nullptr terminates the argument vector
  argv.push_back(nullptr);

  int status = posix_spawnp(&pid, process_path.c_str(), NULL, NULL,
    const_cast<char**>(&(argv[0])), environ);
  if (status == 0) {
    if (verbose) {
      std::cout << "Process with PID " << pid
        << " is currently running..." << std::endl;
    }
    if (waitpid(pid, &status, 0) != -1 && verbose) {
      std::cout << "Process " << pid << " ended execution!"
        << std::endl;
    }
    else {
      perror("waitpid");
    }
  }
  else {
    std::cerr << std::strerror(status) << std::endl;
  }
}

};
