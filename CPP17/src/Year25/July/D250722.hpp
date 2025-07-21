#include <iostream>
#include <vector>
#include <queue>
#include <sstream>

namespace D25M7
{
    using namespace std;
    namespace D250722
    {
        namespace mermaid
        {
            struct Edge
            {
                int u, v, w;
            };

            class Graph
            {
            private:
                int n;
                vector<Edge> edges;
                vector<vector<pair<int, int>>> adj;

            public:
                Graph(int _n) : n(_n), adj(_n) {}

                void addEdges(int u, int v, int w)
                {
                    edges.push_back({u, v, w});
                    int idx = edges.size() - 1;
                    adj[u].push_back({v, idx});
                    adj[v].push_back({u, idx});
                }

                auto PrimAlgorithm() -> decltype(auto)
                {
                    vector<bool> inMST(n, false);
                    vector<int> mstEdgeIdx;

                    struct Node
                    {
                        int w, u, ei;
                    };
                    auto cmp = [](const Node &a, const Node &b)
                    {
                        return a.w > b.w;
                    };

                    priority_queue<Node, vector<Node>, decltype(cmp)> pq(cmp);

                    inMST[0] = true;
                    for (auto &p : adj[0])
                    {
                        int v = p.first, ei = p.second;
                        pq.push({edges[ei].w, v, ei});
                    }

                    while (!pq.empty() && mstEdgeIdx.size() < (size_t)n - 1)
                    {
                        auto cur = pq.top();
                        pq.pop();
                        if (inMST[cur.u])
                            continue;
                        inMST[cur.u] = true;
                        mstEdgeIdx.push_back(cur.ei);

                        for (auto &p : adj[cur.u])
                        {
                            if (!inMST[p.first])
                            {
                                int ei = p.second;
                                pq.push({edges[ei].w, p.first, ei});
                            }
                        }
                    }
                    return mstEdgeIdx;
                }

                ostringstream convert2Mermaid(vector<int> &&mstEdgeIndex)
                {
                    ostringstream out;

                    out << "```mermaid\ngraph LR\n";

                    for (int i = 0; i < (int)edges.size(); ++i)
                    {
                        auto &e = edges[i];
                        out << "    V" << e.u
                            << " -- " << e.w << " --> "
                            << "V" << e.v << "\n";
                    }

                    for (int ei : mstEdgeIndex)
                    {
                        out << "    linkStyle " << ei
                            << " stroke:red,stroke-width:2px\n";
                    }
                    out << "```\n";

                    return out;
                }
            };
        } // namespace mermaid
    } // namespace D250722


} // namespace D25M7