#include <gtest/gtest.h>

#include "../src/Commands/Commands.h"

TEST(HelloTest, BasicAssertions) {
  SetCWD("");

  EXPECT_STRNE("hello", "world");
  EXPECT_EQ(7 * 6, 42);
}
