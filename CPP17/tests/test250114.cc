#include <gtest/gtest.h>
#include <string>
#include "../src/Year25/D250114.hpp"
using namespace Y25M1;

TEST(Test250114, RabinKarpTest1) {
    std::string ws = "AABDCDABD";
    std::string ps = "ABD";
    int result = RabinKarp1::findString(ws, ps);
    EXPECT_EQ(result, 2);
}

TEST(Test250114, RabinKarpTest2) {
    std::string ws = "AABDCDABD";
    std::string ps = "DBA";
    int result = RabinKarp1::findString(ws, ps);
    EXPECT_EQ(result, -1);
}