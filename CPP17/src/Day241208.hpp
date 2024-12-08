#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstdint>
#include <tuple>
#include <limits>
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

    // CRC 
    class CRC32
    {
    private:
        // const var for CRC-32
        static const uint32_t CRC32_POLYNOMIAL = 0xEDB88320;

    public:
        static vector<uint32_t> generateCRCTable()
        {
            vector<uint32_t> table(256);
            for(uint32_t i =0;i<256;++i)
            {
                uint32_t crc = i;
                for(int j=0;j<8;++j)
                {
                    if(crc & 1)
                    {
                        crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
                    }
                    else
                    {
                        crc >>= 1;
                    }
                }
                table[i] = crc;
            }
            return table;
        }

        static uint32_t calculateCRC(const vector<uint8_t>& data, const vector<uint32_t>& table)
        {
            uint32_t crc = 0xFFFFFFFF;
            for(uint8_t byte : data)
            {
                uint8_t index = (crc ^ byte) & 0xFF;
                crc = (crc >> 8) ^ table[index];
            }
            return crc ^ 0xFFFFFFFF; 
        }
    };

    struct Edge {
        int to, weight;
    };

    struct State {
        int cost, node, transfers;

        bool operator>(const State& other) const {
            return cost > other.cost;
        }
    };
    // Graph
    class Graph
    {
    private:
        int V;
        vector<vector<Edge>> adj;

        explicit Graph(int v) : V(v), adj(v) {}

    public:
        void addEdge(int u, int v, int w)
        {
            adj[u].push_back({v, w});
        }

        int shortestPathWithTransfers(int start, int end, int transferCost);
    };

    int Graph::shortestPathWithTransfers(int star,int end, int transferCost)
    {
        priority_queue<State, vector<State>, greater<>> pq;
        // stores the minimum cost of reaching a particular node and the num of transfers
        vector<vector<int>> dist(V, vector<int>(V, INF)); // dist[node][환승 횟수]

        dist[start][0] = 0;
        pq.push({0, start, 0});

        while(!pq.empty())
        {
            auto [currentCosts, currentNode, transfers] = pq.top();
            pq.pop();

            if(currentNode == end)return currentCost;

            for(const auto& edge : adj[currentNode])
            {
                int nextNode = edge.to;
                int nextCost = currentCost + edge.weight + (transfers * transferCost);
                int nextTransfers = transfers + 1;

                if(nextTransfers < V && nextCost < dist[nextNode][nextTransfers])
                {
                    dist[nextNode][nextTransfers] = nextCost;
                    pq.pusH({nextCost, nextNode, nextTransfers});
                }
            }
        }
        return -1;// If no path exists
    }

6
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

    class Task2
    {
    public:
        static int Run()
        {
            /// The generateCRCTable function generates a table used by the CRC-32 arlgorithm.
            /// This table is used to speed up CRC calculations.
            vector<uint32_t> crcTable = CRC32::generateCRCTable();

            // 테스트 데이터
            vector<uint8_t> data = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello" 문자열
            cout << "Input data: ";
            for (uint8_t byte : data) {
                cout << hex << static_cast<int>(byte) << " ";
            }
            cout << endl;

            // CRC 계산
            uint32_t crc = CRC32::calculateCRC(data, crcTable);
            cout << "Calculated CRC32: 0x" << hex << crc << endl;

            // CRC 확인 (예: 데이터 끝에 CRC 추가 후 검증)
            // CRC32 -> uint8_t * 4
            data.push_back((crc >> 24) & 0xFF); 
            data.push_back((crc >> 16) & 0xFF);
            data.push_back((crc >> 8) & 0xFF);
            data.push_back(crc & 0xFF);

            /// After adding the CRC value at the end of the data,
            /// recalculate and if 0 comes out, the verfication is judged to be successful.
            uint32_t newCRC = CRC32::calculateCRC(data, crcTable);
            cout << "Validation CRC32: 0x" << hex << newCRC << endl;

            if (newCRC == 0) {
                cout << "CRC check passed!" << endl;
            } else {
                cout << "CRC check failed!" << endl;
            }

            return 0;
        }
    };
}