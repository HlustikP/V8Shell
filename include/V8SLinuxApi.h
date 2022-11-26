// This File contains functions with Linux-specific api calls
#pragma once

#include <spawn.h>
#include <sys/wait.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <vector>

namespace Commands {

void CreateNewProcess(std::string& process_path, std::vector<std::string>& args, bool verbose);

};
