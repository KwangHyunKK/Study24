#include <deque>
#include <limits>
#include <cassert>
#include <iostream>

namespace Y25M1
{

    // Test is not passed. So this algorithm will be not used.
    // class ConvexHullTrick
    // {
    // public:
    //     void addLine(long long m, long long b)
    //     {
    //         Line newLine = {m, b};
    //         while(lines.size() >= 2 && isRedundant(lines[lines.size() - 2], lines.back(), newLine))
    //         {
    //             lines.pop_back();
    //         }
    //         lines.push_back(newLine);
    //     }


    //     long long query(long long x)
    //     {
    //         assert(!lines.empty());
    //         while(lines.size() >= 2 && evaluate(lines[0], x) >= evaluate(lines[1], x))
    //         {
    //             lines.pop_front();
    //         }
    //         return evaluate(lines[0], x);
    //     }
    // private:
    //     struct Line
    //     {
    //         long long m, b;
    //     };

    //     std::deque<Line> lines;

    //     long long evaluate(const Line& line, long long x) const
    //     {
    //         return line.m * x + line.b;
    //     }

    //     bool isRedundant(const Line& l1, const Line& l2, const Line& l3) const
    //     {
    //         // Avoid precision issues by cross multiplying
    //         return (l2.b - l1.b) * (l1.m - l3.m) >= (l3.b - l1.b) * (l1.m - l2.m);
    //     }
    // };
}