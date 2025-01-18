#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include <string>
#include "../src/Year25/D250119.hpp"
using namespace Y25M1;

TEST(Test250119, enumTest1) {
    std::vector<int> data{12, 89, 654, 123, 4582, 0};

    for(auto&& enumerated: enumerate(data))
    {
        EXPECT_EQ(data[enumerated.index], enumerated.element);        
    }
}

TEST(Test250119, enumTest2) {
    std::vector<int> data{12, 89, 654, 123, 4582, 0};
    std::vector<int> origin = data;
    for(auto&& enumerated: enumerate(data))
    {
        enumerated.element *= 2;
    }

    for(auto&& enumerated: enumerate(data))
    {
        EXPECT_EQ(origin[enumerated.index] * 2, enumerated.element);  
    }
}

TEST(Test250119, enumTest3) {
    std::string str = "bonjour";
    for (auto&&[index, character]: enumerate(str)) {
        EXPECT_EQ(str[index], character);
    }
}