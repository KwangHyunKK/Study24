#include <iostream>
#include <vector>
#include <limits>
#include <cmath>

namespace Y25M7
{
    namespace Y250728
    {
        using namespace std;

        struct Edge
        {
            int to;
            long long weight;
        }; // Edge struct

        // Δ‑Stepping SSSP (Single‑Source Shortest Paths)
        // @param n : vertex count
        // @param delta : light/heavy thershhold
        // @param adj :graph
        // @param dist : the result

        void deltaStepping(int n, int src, long long delta, const vector<vector<Edge>> &adj, vector<long long> &dist)
        {
            const long long INF = numeric_limits<long long>::max();

            // 1) initialize
            dist.assign(n, INF);
            dist[src] = 0; // Distance of start point is zero.

            // 2) bucket array : bucket[i] <- [i * delta, (i + 1) * delta)
            // The bucket element contains the interval
            vector<vector<int>> bucket;
            bucket.emplace_back(); // bucket[0]
            bucket[0].push_back(src);

            // 3) Execute with bucket index
            size_t b = 0;
            while (true)
            {
                // --- (A) next empty bucket
                while (b < bucket.size() && bucket[b].empty())
                {
                    ++b;
                }

                // terminate condition
                if (b >= bucket.size())
                    break;

                // --- (B) vertex set R
                vector<int> R = move(bucket[b]);
                bucket[b].clear();

                // --- (C) light edges
                // light edges : weight <= delta
                // R is startset. relaxing
                vector<int> S = R; // relaxed vertexes.
                while (!S.empty())
                {
                    vector<int> nextS; // new relaxed vertexes.

                    for (int u : S)
                    {
                        long long du = dist[u];

                        // every edges from u
                        for (auto &e : adj[u])
                        {
                            if (e.weight <= delta)
                            {
                                long long nd = du + e.weight;
                                if (nd < dist[e.to])
                                {
                                    dist[e.to] = nd;
                                    size_t idx = nd / delta;
                                    if (idx >= bucket.size())
                                        bucket.resize(idx + 1);
                                    bucket[idx].push_back(e.to);
                                    nextS.push_back(e.to);
                                }
                            }
                        }
                    }
                    S.swap(nextS);
                }

                //  --- (D) heavy edge
                // heavy-edges : weight > delta
                for (int u : R)
                {
                    long long du = dist[u];
                    for (auto &e : adj[u])
                    {
                        if (e.weight > delta)
                        {
                            long long nd = du + e.weight;
                            if (nd < dist[e.to])
                            {
                                dist[e.to] = nd;
                                size_t idx = nd / delta;
                                if (idx >= bucket.size())
                                    bucket.resize(idx + 1);
                                bucket[idx].push_back(e.to);
                            }
                        }
                    }
                }
            }
        }

        void Run()
        {
            int n = 5;
            vector<vector<Edge>> graph(n);

            graph[0].push_back({1, 3});
            graph[0].push_back({2, 1});
            graph[1].push_back({3, 4});
            graph[2].push_back({1, 1});
            graph[2].push_back({4, 2});
            graph[3].push_back({4, 1});
            graph[4].push_back({3, 1});

            // Δ 값 설정 (도메인 특성에 따라 튜닝)
            long long delta = 2;

            // 결과를 저장할 배열
            vector<long long> dist;
            deltaStepping(n, /*src=*/0, delta, graph, dist);

            // 거리 출력
            cout << "Vertex : Distance from 0\n";
            for (int i = 0; i < n; ++i)
            {
                cout << "  " << i << "    : "
                     << (dist[i] == numeric_limits<long long>::max() ? -1 : dist[i])
                     << "\n";
            }
            return;
        }
    } // namespace Y250728
} // namespace Y25M7