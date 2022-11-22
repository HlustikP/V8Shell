#pragma once

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace test {

const auto EXIT_CODE_OK = 0;

inline void PretestCleanup() {
  const auto test_dir = fs::path("test-dir");
  if (fs::exists(test_dir)) {
    fs::remove_all(test_dir);
  }
}

const struct BootV8Shell {
  inline static int argc = 1;
  inline static char* argv = "tests";
};

const struct RunV8Shell {
  inline static int argc = 2;
  inline static char* argv[] = { "tests", "" };
};

const struct CreateNewDir {
  inline static int argc = 2;
  inline static char* argv[] = {"tests", "../../../tests/scripts/mkdir.js"};
  inline static std::string target_dir = "test-dir";
};

const struct CreateNewFile {
  inline static int argc = 2;
  inline static char* argv[] = {"tests", "../../../tests/scripts/touch.js"};
  inline static std::string target_file = "test-dir/test-file.txt";
};

const struct CopySameDir {
  inline static int argc = 2;
  inline static char* argv[] = {"tests", "../../../tests/scripts/copy.js"};
  inline static std::string target_file = "test-dir/test-file-copy.txt";
};

const struct CopyDifferentDir {
  inline static int argc = 2;
  inline static char* argv[] = {"tests", "../../../tests/scripts/copy-different-dir.js"};
  inline static std::string target_file = "test-dir/new-dir/test-file-copy.txt";
};

const struct RenameSameDir {
  inline static int argc = 2;
  inline static char* argv[] = {"tests", "../../../tests/scripts/rename-success.js"};
  inline static std::string target_file = "test-dir/test-file-copy-renamed.txt";
};

const struct RenameDifferentDir {
  inline static int argc = 2;
  inline static char* argv[] = {"tests", "../../../tests/scripts/rename-failure.js"};
  inline static std::string target_file = "test-file-copy-renamed.txt";
};

const struct RemoveFileFile {
  inline static int argc = 2;
  inline static char* argv[] = {"tests", "../../../tests/scripts/rm-file-file.js"};
  inline static std::string target_file = "test-dir/test-file-copy-renamed.txt";
};

const struct RemoveFileDir {
  inline static int argc = 2;
  inline static char* argv[] = {"tests", "../../../tests/scripts/rm-file-dir.js"};
  inline static std::string target_file = "test-dir/new-dir";
};

const struct RemoveDirDir {
  inline static int argc = 2;
  inline static char* argv[] = {"tests", "../../../tests/scripts/rm-dir-dir.js"};
  inline static std::string target_file = "test-dir/test-rdd";
};

const struct RemoveDirFile {
  inline static int argc = 2;
  inline static char* argv[] = {"tests", "../../../tests/scripts/rm-dir-file.js"};
  inline static std::string target_file = "test-dir/test-file.txt";
};

const struct RemoveAnyDir {
  inline static int argc = 2;
  inline static char* argv[] = {"tests", "../../../tests/scripts/rm-any-dir.js"};
  inline static std::string target_file = "test-dir/test-rdd";
};

const struct RemoveAnyFile {
  inline static int argc = 2;
  inline static char* argv[] = {"tests", "../../../tests/scripts/rm-any-dir.js"};
  inline static std::string target_file = "test-dir/test.txt";
};

}  // namespace test
