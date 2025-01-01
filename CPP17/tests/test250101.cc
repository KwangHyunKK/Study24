#include <iostream>
#include <vector>
#include <gtest/gtest.h>
#include "../src/Year25/D250101.hpp"
using namespace Y25M1;

TEST(Test250101, CreateFenwickTree)
{
    std::vector<long long> origin = {3,2,5,7,10,3,2,7,8};
    FenwickTree bit(origin);
    std::vector<long long> output = {0,3,5,5,17,10,13,2,39,8};
    std::vector<long long> result = bit.getTree();
    bool check = true;
    for(int i=1;i<=output.size();++i)
    {
        if(result[i] != output[i])
        {
            check = false;
        }
    }
    EXPECT_TRUE(check);
}

TEST(Test250101, FenwickTreeSum)
{
    std::vector<long long> origin = {3,2,5,7,10,3,2,7,8};
    FenwickTree bit(origin);
    EXPECT_EQ(bit.sum(5), 27);
}