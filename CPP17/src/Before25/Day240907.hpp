#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <memory>
#include <future>
#include <deque>
#include <string>
#include <chrono>
#include <random>
#include <cmath>
#include <thread>
using namespace std;

namespace d240907
{
    // # 1. Newton-Raphson sqrt
    double newtonRaphsonSqrt(double value, double epsilon = 1e-6)
    {
        double guess = value /2;
        while(std::abs(guess * guess - value) > epsilon)
            guess = (guess + value / guess) /2;

        return guess;
    }

    double calculateHyptenuseOptimized(double a, double b)
    {
        double sumOfSuares = a * a + b * b;
        return newtonRaphsonSqrt(sumOfSuares);
    }

    int run1()
    {
        double a = 3.0;
        double b = 4.0;
        double c = calculateHyptenuseOptimized(a, b);

        std::cout << "c : " << c << "\n";
        return 0;
    }

    // # 2. Union-Find
    class UnionFind
    {
    private:
        vector<int> parent;
        vector<int> rank; // rank of parent

    public:
        UnionFind(int n)
        {
            parent.resize(n);
            rank.resize(n, 0);

            int value = 0;
            std::for_each(begin(parent), end(parent), [&value](int& element){
                element = value++;
            });
        }

        // Find
        int find(int x)
        {
            if(parent[x] != x)parent[x] = find(parent[x]);
            return parent[x];
        }

        void unionSets(int a, int b)
        {
            int x = find(a);
            int y = find(b);

            if(x != y)
            {
                if(rank[x] > rank[y])parent[y] = x;
                else if(rank[x] < rank[y])parent[x] = y;
                else{
                    parent[y] = x;
                    rank[x]++;
                }
            }
            return;
        }
    };

    bool checkCycle(int n, vector<pair<int, int>>& edges)
    {
        UnionFind uf(n);

        for(const auto& edge : edges)
        {
            int u = edge.first;
            int v = edge.second;

            if(uf.find(u) == uf.find(v))return true;

            uf.unionSets(u, v);
        }
        return false;
    }

    int run2()
    {
        int n = 5; // # of node
        vector<pair<int, int>> edges = {
            {0, 1}, {1,2}, {2,3}, {3,4}, {4,1}
        };

        if(checkCycle(n, edges))
        {
            std::cout << "사이클 존재\n";
        }
        else{
            std:cout << "No cycle\n";
        }

        return 0;
    }

    // # 3. Convex Hull Trick
    // A technique for efficiently solving the problem of minimization or maximization of linear functions in dynamic programming(DP).
    // It is primarily used to optimize DP, and helps to quickly find the best value among multiple linear functions using Convex Hull.
    // This technique usually allows use to reduce the time complexity from O(n^2) to O(n log n) or O(n) .

    class ConvexHullTrick
    {
        struct Line
        {
            long long slope, intercept;

            double intersect(const Line& other) const
            {
                return (double)(other.intercept - intercept) / (slope - other.slope);
            }
        };

        deque<Line> hull;

    public:
        void addLine(long long slope, long long intercept)
        {
            Line newLine = {slope, intercept};
            while(hull.size() >= 2)
            {
                if(newLine.intersect(hull[hull.size() - 2]) <= hull.back().intersect(hull[hull.size() - 2]))
                {
                    hull.pop_back();
                }
                else
                    break;
            }
            hull.push_back(newLine);
        }

        long long getMinValue(long long x)
        {
            while(hull.size() >= 2 && hull[1].intercept + hull[1].slope * x <= hull[0].intercept + hull[0].slope * x)
            {
                hull.pop_front();
            }

            return hull[0].intercept + hull[0].slope * x;
        }
    };

    int run3()
    {
        int n;
        cin >> n;

        vector<long long> a(n), b(n), dp(n);

        for (int i=0;i<n;++i) cin >> a[i];
        for(int i=0;i<n;++i) cin >> b[i];

        ConvexHullTrick cht;

        dp[0] = 0;
        cht.addLine(b[0], dp[0]);

        for(int i=1;i<n;++i)
        {
            dp[i] = cht.getMinValue(a[i]);
            cht.addLine(b[i],dp[i]);
        }

        std::cout << "최종 결과 : " << dp[n-1] << "\n";
        return 0;
    }

    // # 4. segment tree
    void init(vector<long long>& a, vector<long long>& tree, int node, int start, int end)
    {
        if(start == end)tree[node] = a[start];
        else{
            init(a, tree, node * 2, start, (start + end)/2);
            init(a, tree, node * 2 + 1, (start + end)/2, end);
            tree[node] = tree[node * 2] + tree[node * 2 + 1];
        }
    }

    void update(vector<long long>& a, vector<long long>& tree, int node, int start, int end, int index, long long val)
    {
        if(index < start || index > end)
            return;

        if(start == end)
        {
            a[index] = val;
            tree[node] = val;
            return;
        }
        update(a, tree, node * 2, start, (start + end)/2, index, val);
        update(a, tree, node * 2 + 1, (start + end)/2, end, index, val);
        tree[node] = tree[node * 2] + tree[node * 2 + 1];
    }

    long long query(vector<long long>& tree, int node, int start, int end, int left, int right)
    {
        if(left > end || right < start)return 0;

        if(left <= start && end <= right)return tree[node];

        long long lsum = query(tree, node * 2, start, (start + end)/2, left, right);
        long long rsum = query(tree, node *2 + 1, (start +end)/2 + 1, end, left, right);
        return lsum + rsum;
    }

    int run4()
    {
        ios_base::sync_with_stdio(false);
        cin.tie(nullptr);
        int n, m, k;
        cin >> n >> m >> k;
        vector<long long> a(n);
        int h = (int)ceil(log2(n));
        int tree_size = (1 << (h+1));
        vector<long long> tree(tree_size);
        m += k;
        for (int i=0; i<n; i++) {
            cin >> a[i];
        }
        init(a, tree, 1, 0, n-1);
        while (m--) {
            int what;
            cin >> what;
            if (what == 1) {
                int index;
                long long val;
                cin >> index >> val;
                update(a, tree, 1, 0, n-1, index-1, val);
            } else if (what == 2) {
                int left, right;
                cin >> left >> right;
                cout << query(tree, 1, 0, n-1, left-1, right-1) << '\n';
            }
        }
        return 0;
    }



    int run()
    {
        std::thread t1(run1);

        if(t1.joinable())t1.join();

        std::cout << "run1 end. run2 will be started.\n";

        std::thread t2(run2);
        
        if(t2.joinable())t2.join();

        return 0;
    }
}

