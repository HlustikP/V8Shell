#include "V8Shell.h"

int main(int argc, char* argv[]) {
  int exit_code = 0;
  V8Shell shell(argc, argv, exit_code);
  if (exit_code == 0) {
    exit_code = shell.Run();
  }
    
  return exit_code;
}
