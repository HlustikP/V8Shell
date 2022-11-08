#include <gtest/gtest.h>

#include "Commands.h"

TEST(Commands, GettingSettingCWD) {
  const auto path = fs::current_path();
  Commands::SetCWD(path);

  EXPECT_EQ(path, Commands::GetCWD());
}
