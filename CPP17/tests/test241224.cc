#include <gtest/gtest.h>
#include "../src/Day241224.hpp"
#include <iostream>
using namespace D241224;

TEST(Test241224, BasicTree) {
    Tree tree(7);
    tree.addEdge(0, 1);
    tree.addEdge(0, 2);
    tree.addEdge(1, 3);
    tree.addEdge(1, 4);
    tree.addEdge(2, 5);
    tree.addEdge(2, 6);

    tree.preprocess();

    EXPECT_EQ(tree.lca(3, 4), 1);
    EXPECT_EQ(tree.lca(3, 5), 0);
    EXPECT_EQ(tree.lca(5, 6), 2);
}

TEST(Test241224, LinearTree) {
    Tree tree(5);
    tree.addEdge(0, 1);
    tree.addEdge(1, 2);
    tree.addEdge(2, 3);
    tree.addEdge(3, 4);

    tree.preprocess();

    EXPECT_EQ(tree.lca(4, 3), 3);
    EXPECT_EQ(tree.lca(4, 2), 2);
    EXPECT_EQ(tree.lca(1, 4), 1);
}

TEST(Test241224, SingleNode) {
    Tree tree(1);

    tree.preprocess();

    EXPECT_EQ(tree.lca(0, 0), 0);
}