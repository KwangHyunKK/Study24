#include <iostream>
#include <deque>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "../src/Before25/Day241229.hpp"
using namespace D241229;

// TEST(Test241229, CalculateHash)
// {
//     namespace fs = std::filesystem;
//     fs::path currentPath = fs::current_path();
//     std::cout << currentPath << "\n";
//     std::string str1 = calculateHash("test", currentPath);
//     std::string str2 = calculateHash("test", currentPath);
//     EXPECT_EQ(str1, str2);
// }

TEST(Test241229, EmptyLCS)
{
    std::vector<std::string> oldLines = {};
    std::vector<std::string> newLines = {};
    auto result = computeLCS(oldLines, newLines);
    EXPECT_TRUE(result.empty());
}

TEST(Test241229, AddLine)
{
    std::vector<std::string> oldLines = {"Line 1"};
    std::vector<std::string> newLines = {"Line 1", "Line 2"};
    auto result = computeLCS(oldLines, newLines);
    EXPECT_EQ(result[1].first, std::make_pair("+", "Line 2").first);
    EXPECT_EQ(result[1].second, std::make_pair("+", "Line 2").second);
}

TEST(Test241229, RemoveLine) {
    std::vector<std::string> oldLines = {"Line 1", "Line 2"};
    std::vector<std::string> newLines = {"Line 1"};
    auto result = computeLCS(oldLines, newLines);
    EXPECT_EQ(result[1].first, std::make_pair("-", "Line 2").first);
    EXPECT_EQ(result[1].second, std::make_pair("-", "Line 2").second);
}

TEST(Test241229, ReplaceLine) {
    std::vector<std::string> oldLines = {"Line 1"};
    std::vector<std::string> newLines = {"Line 2"};
    auto result = computeLCS(oldLines, newLines);
    EXPECT_EQ(result[0].first, std::make_pair("-", "Line 1").first);
    EXPECT_EQ(result[0].second, std::make_pair("-", "Line 1").second);
    EXPECT_EQ(result[1].first, std::make_pair("+", "Line 2").first);
    EXPECT_EQ(result[1].second, std::make_pair("+", "Line 2").second);
}