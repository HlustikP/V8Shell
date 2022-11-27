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

struct BootV8Shell {
  inline static int argc = 1;
  inline static const char* argv = "tests";
};

struct RunV8Shell {
  inline static int argc = 2;
  inline static const char* argv[] = { "tests", "" };
};

struct CreateNewDir {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/mkdir.js"};
  inline static std::string target_dir = "test-dir";
};

struct CreateNewFile {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/touch.js"};
  inline static std::string target_file = "test-dir/test-file.txt";
};

struct CopySameDir {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/copy.js"};
  inline static std::string target_file = "test-dir/test-file-copy.txt";
};

struct CopyDifferentDir {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/copy-different-dir.js"};
  inline static std::string target_file = "test-dir/new-dir/test-file-copy.txt";
};

struct RenameSameDir {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/rename-success.js"};
  inline static std::string target_file = "test-dir/test-file-copy-renamed.txt";
};

struct RenameDifferentDir {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/rename-failure.js"};
  inline static std::string target_file = "test-file-copy-renamed.txt";
};

struct RemoveFileFile {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/rm-file-file.js"};
  inline static std::string target_file = "test-dir/test-file-copy-renamed.txt";
};

struct RemoveFileDir {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/rm-file-dir.js"};
  inline static std::string target_file = "test-dir/new-dir";
};

struct RemoveDirDir {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/rm-dir-dir.js"};
  inline static std::string target_file = "test-dir/test-rdd";
};

struct RemoveDirFile {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/rm-dir-file.js"};
  inline static std::string target_file = "test-dir/test-file.txt";
};

struct RemoveAnyDir {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/rm-any-dir.js"};
  inline static std::string target_file = "test-dir/test-rdd";
};

struct RemoveAnyFile {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/rm-any-file.js"};
  inline static std::string target_file = "test-dir/test.txt";
};

struct MoveFileTo {
  inline static int argc = 2;
  inline static const char* argv[] = {"tests", "../../../tests/scripts/move.js"};
  inline static std::string target_file_source = "test-dir/move-me.txt";
  inline static std::string target_file = "test-dir/move-to/move-me.txt";
};

#if _WIN32
struct SpawnProcessSyncNoArgs {
  inline static int argc = 2;
  inline static const char* argv[] = { "tests", "../../../tests/scripts/win-spawn-process-no-args.js" };
  inline static std::string target_dir = "test-dir/proc-one";
};
#endif

}  // namespace test
