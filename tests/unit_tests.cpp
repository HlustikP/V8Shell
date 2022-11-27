#include <gtest/gtest.h>

#include "test_ressources.hpp"
#include "V8Shell.h"

TEST(PretestUtils, CleanupTestdir) { 
  test::PretestCleanup(); 
}

TEST(CommandsUtils, GettingSettingsCWD) {
  const auto path = fs::current_path();
  Commands::SetCWD(path);

  ASSERT_EQ(path, Commands::GetCWD());
}

TEST(V8Shell, BootV8Shell) {
  int exit_code = 0;
  V8Shell shell(test::BootV8Shell::argc, &test::BootV8Shell::argv,
                exit_code);

  ASSERT_EQ(exit_code, test::EXIT_CODE_OK);
}

TEST(V8Shell, RunV8Shell) {
  int exit_code = 0;
  V8Shell shell(test::RunV8Shell::argc, test::RunV8Shell::argv,
                exit_code);
  exit_code = shell.Run();

  ASSERT_EQ(exit_code, test::EXIT_CODE_OK);
}

TEST(V8Shell, CreateNewDirectory) {
  int exit_code = 0;
  V8Shell shell(test::CreateNewDir::argc, test::CreateNewDir::argv,
                exit_code);
  exit_code = shell.Run();

  ASSERT_TRUE(fs::exists(test::CreateNewDir::target_dir));
  EXPECT_TRUE(fs::is_directory(test::CreateNewDir::target_dir));
}

TEST(V8Shell, CreateNewFile) {
  int exit_code = 0;
  V8Shell shell(test::CreateNewFile::argc, test::CreateNewFile::argv,
                exit_code);
  exit_code = shell.Run();

  ASSERT_TRUE(fs::exists(test::CreateNewFile::target_file));
  EXPECT_FALSE(fs::is_directory(test::CreateNewFile::target_file));
}

TEST(V8Shell, CopySameDir) {
  int exit_code = 0;
  V8Shell shell(test::CopySameDir::argc, test::CopySameDir::argv,
                exit_code);
  exit_code = shell.Run();

  ASSERT_TRUE(fs::exists(test::CopySameDir::target_file));
  EXPECT_FALSE(fs::is_directory(test::CopySameDir::target_file));
}

TEST(V8Shell, CopyDifferentDir) {
  int exit_code = 0;
  V8Shell shell(test::CopyDifferentDir::argc, test::CopyDifferentDir::argv,
                exit_code);
  exit_code = shell.Run();

  ASSERT_TRUE(fs::exists(test::CopyDifferentDir::target_file));
  EXPECT_FALSE(fs::is_directory(test::CopyDifferentDir::target_file));
}

TEST(V8Shell, RenameSameDir) {
  int exit_code = 0;
  V8Shell shell(test::RenameSameDir::argc, test::RenameSameDir::argv,
                exit_code);
  exit_code = shell.Run();

  ASSERT_TRUE(fs::exists(test::RenameSameDir::target_file));
  EXPECT_FALSE(fs::is_directory(test::RenameSameDir::target_file));
}

TEST(V8Shell, RenameDifferentDir) {
  int exit_code = 0;
  V8Shell shell(test::RenameDifferentDir::argc, test::RenameDifferentDir::argv,
                exit_code);
  exit_code = shell.Run();

  EXPECT_FALSE(fs::exists(test::RenameDifferentDir::target_file));
}

TEST(V8Shell, RemoveFileFile) {
  int exit_code = 0;
  V8Shell shell(test::RemoveFileFile::argc, test::RemoveFileFile::argv,
                exit_code);
  exit_code = shell.Run();

  EXPECT_FALSE(fs::exists(test::RemoveFileFile::target_file));
}

TEST(V8Shell, RemoveFileDir) {
  int exit_code = 0;
  V8Shell shell(test::RemoveFileDir::argc, test::RemoveFileDir::argv,
                exit_code);
  exit_code = shell.Run();

  EXPECT_TRUE(fs::exists(test::RemoveFileDir::target_file));
}

TEST(V8Shell, RemoveDirDir) {
  int exit_code = 0;
  V8Shell shell(test::RemoveDirDir::argc, test::RemoveDirDir::argv,
                exit_code);
  exit_code = shell.Run();

  EXPECT_FALSE(fs::exists(test::RemoveDirDir::target_file));
}

TEST(V8Shell, RemoveDirFile) {
  int exit_code = 0;
  V8Shell shell(test::RemoveDirFile::argc, test::RemoveDirFile::argv,
                exit_code);
  exit_code = shell.Run();

  EXPECT_TRUE(fs::exists(test::RemoveDirFile::target_file));
}

TEST(V8Shell, RemoveAnyDir) {
  int exit_code = 0;
  V8Shell shell(test::RemoveAnyDir::argc, test::RemoveAnyDir::argv,
                exit_code);
  exit_code = shell.Run();

  EXPECT_FALSE(fs::exists(test::RemoveAnyDir::target_file));
}

TEST(V8Shell, RemoveAnyFile) {
  int exit_code = 0;
  V8Shell shell(test::RemoveAnyFile::argc, test::RemoveAnyFile::argv,
                exit_code);
  exit_code = shell.Run();

  EXPECT_FALSE(fs::exists(test::RemoveAnyFile::target_file));
}

TEST(V8Shell, MoveFileTo) {
  int exit_code = 0;
  V8Shell shell(test::MoveFileTo::argc, test::MoveFileTo::argv,
                exit_code);
  exit_code = shell.Run();

  EXPECT_FALSE(fs::exists(test::MoveFileTo::target_file_source));
  EXPECT_TRUE(fs::exists(test::MoveFileTo::target_file));
}

TEST(V8Shell, SpawnProcessSyncNoArgs) {
  int exit_code = 0;
  V8Shell shell(test::SpawnProcessSyncNoArgs::argc,
                test::SpawnProcessSyncNoArgs::argv,
                exit_code);
  exit_code = shell.Run();

  EXPECT_TRUE(fs::exists(test::SpawnProcessSyncNoArgs::target_dir));
}
