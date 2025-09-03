#include <gtest/gtest.h>
#include <iostream>
#include <string>

extern int foo();

TEST(foo, foo1)
{
    EXPECT_EQ(123, foo());
}