#include <gtest/gtest.h>
#include "math/add.hpp"

TEST(Add, Basic) {
  EXPECT_EQ(math::add(2, 3), 5);
  EXPECT_EQ(math::add(-1, 1), 0);
}

TEST(Add, Edge) {
  EXPECT_EQ(math::add(0, 0), 0);
}
