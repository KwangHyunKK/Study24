#include <iostream>
#include <vector>
#include <queue>
#include <climits>

namespace Y25M1
{
    using namespace std;

    class MaxFlow1
    {
    public:
        MaxFlow1(const MaxFlow1& other)=delete;
        MaxFlow1(MaxFlow&& other)=delete;
        MaxFlow1& operator=(const MaxFlow& other)=delete;
        Maxflow1& operator=(MaxFlox&& other)=delete;
        static int char2Int(char c)
        {
            if(c >= 'A' && c <='Z') return c - 'A' + 1;
            else if(c >= 'a' && c <= 'z')return c - 'a' + 27; // lower case and upper case is different.
        }

        int flow(int source, int destination)
        {
            int sum = 0;

            while(true)
            {
                vector<int> p(60, - 1);

                queue<int> q;
                q.push(source);

                // find path from source to destination
                while(!q.empty() && p[destination] == -1) // not empty && not destination
                {
                    int x = q.front(); q.pop();

                    for(int i=0;i<adj[x].size();++i)
                    {
                        int y = adj[x][i];
                        if(cap[x][y] - flo[x][y] <= 0 || p[y] != -1)continue; 

                        p[y] = x; // check parent
                        q.push(y);

                        if(y == destination)break;
                    }
                }
                if(p[destination] == -1)break; // no more path

                int val = INT_MAX;

                for(int i=destination;i!=source;i=p[i])
                {
                    val = min(val, cap[p[i]][i] - flow[p[i]][i]);
                }

                for(int i=destination;i!=source;i=p[i])
                {
                    flow[p[i]][i] += val;
                    flow[i][p[i]] -= val;
                }

                sum += val;
            }
            return sum;
        }

    private:
        vector<vector<int>> adj, cap, flow;
        // cap : capacity - max flow
        // flow : current flow
    };
}