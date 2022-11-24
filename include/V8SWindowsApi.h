// This File contains functions with Windows-specific api calls
#pragma once

#include <Windows.h>
#include <iostream>

namespace Commands {

void CreateNewProcess(std::string& process_command, bool verbose);

};
