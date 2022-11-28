// This File contains functions with Windows-specific api calls
#pragma once

#include <Windows.h>
#include <iostream>
#include <vector>

namespace Commands {

void CreateNewProcess(std::string& process_path, std::vector<std::string>& args, bool verbose);

};
