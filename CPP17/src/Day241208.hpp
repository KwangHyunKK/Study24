#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
using namespace std;

namespace D241208
{
    // Union - Find
    ///  # TODO
    /// In read-world development environments, recursive functions are considered dangerous
    /// and are prohibited.
    /// Consideration should be given to utilizing the content without the use of recursion
    /// or ways to increase safety.
    class UF
    {
    private:
        vector<int> parent;
        vector<int> rank;

    public:
        UF(int n)
        {
            parent.resize(n);
            rank.resize(n, 0);
            for(int i=0;i<n;++i)parent[i] = i;
        }

        int find(int x)
        {
            if(parent[x] != x)parent[x] = find(parent[x]);
            return parent[x];
        }

        void unite(int x, int y)
        {
            int rootX = find(x);
            int rootY = find(y);

            if(rootX != rootY)
            {
                if(rank[rootX] > rank[rootY])
                {
                    parent[rootY] = rootX;
                }
                else if (rank[rootX] < rank[rootY])
                {
                    parent[rootX] = rootY;
                }
                else 
                {
                    parent[rootY] = rootX;
                    rank[rootX]++;
                }
            }
        }

        bool isConn(int x, int y)
        {
            return find(x) == find(y);
        }
    };

    class Task1
    {
    public:
        static int Run()
        {
            int n = 10;
            UF uf(n);

            uf.unite(0, 1);
            uf.unite(1, 2);
            uf.unite(3, 4);
            uf.unite(5, 6);
            uf.unite(6, 7);
            uf.unite(8, 9);

            cout << "0 and 2 connected: " << (uf.isConn(0, 2) ? "Yes" : "No") << endl;
            cout << "3 and 4 connected: " << (uf.isConn(3, 4) ? "Yes" : "No") << endl;
            cout << "5 and 9 connected: " << (uf.isConn(5, 9) ? "Yes" : "No") << endl;

            uf.unite(7, 8);
            cout << "5 and 9 connected after union: " << (uf.isConn(5, 9) ? "Yes" : "No") << endl;
            return 0;
        }
    };
}