#include <gtest/gtest.h>

#include "Commands.h"

TEST(Commands, GettingSettingsCWD) {
  const auto path = fs::current_path();
  Commands::SetCWD(path);

  EXPECT_EQ(path, Commands::GetCWD());
}
