// #include <gtest/gtest.h>
// #include <climits>
// #include "../src/Year25/D250112.hpp"
// using namespace Y25M1;

// TEST(Test250112, SingleLine) {
//     ConvexHullTrick cht;
//     cht.addLine(2, 3); // y = 2x + 3

//     // Test for specific x-values
//     EXPECT_EQ(cht.query(1), 5); // 2*1 + 3 = 5
//     EXPECT_EQ(cht.query(2), 7); // 2*2 + 3 = 7
//     EXPECT_EQ(cht.query(0), 3); // 2*0 + 3 = 3
// }

// TEST(Test250112, MultipleLines) {
//     ConvexHullTrick cht;
//     cht.addLine(1, 5);  // y = x + 5
//     cht.addLine(2, 3);  // y = 2x + 3
//     cht.addLine(3, -1); // y = 3x - 1

//     // Test for specific x-values
//     EXPECT_EQ(cht.query(0), -1);  // Min(3, 5, -1) = 3
//     EXPECT_EQ(cht.query(1), 2);  // Min(5, 6, 2) = 4
//     EXPECT_EQ(cht.query(2), 5);  // Min(7, 7, 5) = 5
//     EXPECT_EQ(cht.query(10), 15); // Min(23, 15, 29) = 15
// }

// TEST(Test250112, NegativeSlopes) {
//     ConvexHullTrick cht;

//     cht.addLine(-1, 4);  // y = -x + 4
//     cht.addLine(-2, 3);  // y = -2x + 3
//     cht.addLine(-3, 5);  // y = -3x + 5

//     // Test for specific x-values
//     EXPECT_EQ(cht.query(0), 3);  // Min(4, 3, 5) = 3
//     EXPECT_EQ(cht.query(1), 1);  // Min(3, 1, 2) = 1
//     EXPECT_EQ(cht.query(5), -10); // Min(-1, -7, -10) = -10
// }

// TEST(Test250112, UpdatesAndQueries) {
//     ConvexHullTrick cht;
//     cht.addLine(1, 1); // y = x + 1
//     cht.addLine(2, 2); // y = 2x + 2

//     // Test initial queries
//     EXPECT_EQ(cht.query(1), 2); // Min(2*1 + 2, 1*1 + 1) = 3
//     EXPECT_EQ(cht.query(2), 3); // Min(2*2 + 2, 1*2 + 1) = 4

//     // Add more lines and test
//     cht.addLine(0, 0); // y = 0
//     EXPECT_EQ(cht.query(1), 0); // Min(1*1 + 1, 2*1 + 2, 0) = 0
//     EXPECT_EQ(cht.query(5), 0); // Min(1*5 + 1, 2*5 + 2, 0) = 0
// }

// TEST(Test250112, EdgeCases) {
//     ConvexHullTrick cht;

//     // Test with no lines
//     EXPECT_NO_THROW({
//         EXPECT_EQ(cht.query(1), INT_MAX); // Assuming INT_MAX is returned when no lines exist
//     });

//     // Test with overlapping lines
//     cht.addLine(1, 0); // y = x
//     cht.addLine(1, 1); // y = x + 1
//     cht.addLine(1, -1); // y = x - 1

//     EXPECT_EQ(cht.query(1), 0); // Min(x, x+1, x-1) = 0
// }