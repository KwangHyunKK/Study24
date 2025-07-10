#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <queue>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace Y25M7
{
    namespace D250711
    {

        namespace TCPChunkServer
        {
            constexpr std::string FILE_PATH = "source_file.bin";
            constexpr uint16_t SERVER_PORT_BASE = 8000;
            constexpr uint8_t NUM_CHUNKS = 4;
            constepxr uint16_t CHUNK_SHIZE = 1024 * 1024;

            std::vector<std::vector<char>> file_chunks;

            void load_file_chunks()
            {
                std::ifstream file(FILE_PATH, std::ios::binary);
                if (!file.is_open())
                {
                    std::cerr << "Failed to open source file : " << FILE_PATH << "\n";
                    exit(1);
                }

                for (int i = 0; i < NUM_CHUNKS; ++i)
                {
                    std::vector<char> chunk(CHUNK_SIZE);
                    file.read(chunk.data(), CHUNK_SIZE);
                    file_chunks.push_back(std::move(chunk));
                }

                std::cout << "File loaded into memory and split into " << NUM_CHUNKS << " chunks\n";
            }

            void run_chunk_server(int chunk_id)
            {
                int port = SERVER_PORT_BASE + chunk_id;
                int server_fd = socket(AF_INET, SOCK_STREAM, 0);
                if (server_fd < 0)
                {
                    perror("socket");
                    return;
                }

                int opt = 1;
                setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

                sockaddr_in addr{};
                addr.sin_family = AF_INET;
                addr.sin_port = htons(port);
                addr.sin_addr.s_addr = INADDR_ANY;

                if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
                {
                    perror("bind");
                    close(server_fd);
                    return;
                }

                listen(server_fd, 5);
                std::cout << "[+] Chunk server " << chunk_id << " listening on port " << port << "\n";

                while (true)
                {
                    sockaddr_in client;
                    socklen_t client_len = sizeof(client);
                    int client_fd = accept(server_fd, (sockaddr *)&client, &client_len);
                    if (client_fd < 0)
                        continue;

                    int requested_id;
                    recv(client_fd, &requested_id, sizeof(requested_id), 0);

                    if (requested_id == chunk_id)
                    {
                        send(client_fd, file_chunks[chunk_id].data(), CHUNK_SIZE, 0);
                        std::cout << "Sent chunk " << chunk_id << " to client\n";
                    }

                    close(client_fd);
                }
            }

            void run()
            {
                load_file_chunks();

                std::vector<std::thread> threads;

                for (int i = 0; i < NUM_CHUNKS; ++i)
                {
                    threads.emplace_back(run_chunk_server, i);
                }

                for (auto &t : threads)
                    t.join();

                return;
            }
        } // namespace TCPChunkServer

        // namespace DynamicSSSP
        // {
        //     // Dijkstra + Dynamic weight
        //     using namespace std;
        //     constexpr long long INF = (1LL << 60);

        //     struct Edge
        //     {
        //         int to;
        //         long long weight;
        //     };

        //     class DynamicSSSP
        //     {
        //         int N, src;
        //         vector<vector<Edge>> adj;
        //         vector<long long> dist;
        //         vector<int> parent; // parent on SPT
        //         vector<int> parentEdge; // parentEdge[v] = u index (u -> v)

        //     public:
        //         DynamicSSSP(int n_) : N(n_), adj(n_), dist(n_, INF), parent(n_, -1), parentEdge(n_, -1) {}

        //         void addEdge(int u, int v, long long w)
        //         {
        //             adj[u].push_back({v,w});
        //         }

        //         // initial dijkstra
        //         void init(int s)
        //         {
        //             src = s;
        //             fill(dist.begin(), dist.end(), INF);
        //             fill(parent.begin(), parent.end(), -1);
        //             fill(parentEdge.begin(), parentEdge.end(), -1);

        //             dist[s] = 0;
        //             priority_queue<pair<long long, int>, vector<pair<long long, int>>, greater<>> pq; // (weight, source)

        //             pq.push({0, s});

        //             while(!pq.empty())
        //             {
        //                 auto [d,u] = pq.top();
        //                 pq.pop();
        //                 for(int i=0;i<(int)adj[u].size();++i)
        //                 {
        //                     auto& e = adj[u][i];
        //                     if(dist[e.to] > d + e.weight)
        //                     {
        //                         dist[e.to] = d + e.weight;
        //                         parent[e.to] = u;
        //                         parentEdge[e.to] = i;
        //                         pq.push({dist[e.to], e.to});
        //                     }
        //                 }
        //             }
        //         }

        //         // update dynamic u->v weight
        //         void update(int u, int v, long long new_weight)
        //         {
        //             bool found = false;
        //             for(auto& e : adj[u])
        //             {
        //                 if(e.to == v)
        //                 {
        //                     e.w = new_w;
        //                     found = true;
        //                     break;
        //                 }
        //             }
        //             if(!found) return; // ignore if there is no edges.

        //             long long oldDistVia = (parent[v] == u ? dist[u] + 0 : INF);
        //             long long newDistVia = dist[u] + new_w;

        //             if(newDistVia < dist[v])
        //             {
        //                 dist[v] = newDistVia;
        //                 parent[v] = u;
        //                 propagateDecrease(v);
        //             }
        //             else if(parent[v] == u && newDistVia > dist[v])
        //             {
        //                 vector<int> affected = collectSubtree(v);
        //                 for(int x: affected) dist[x] = INF, parent[x] = -1;
        //                 priority_queue<pair<long long, int>, vector<pair<long long, int>>, greater<>> pq;

        //                 for(int x : affected)
        //                 {
        //                     if( x == v && newDistVia < INF)
        //                     {
        //                         dist[v] = newDistVia;
        //                         parent[v] = u;
        //                         pq.push({dist[v], v});
        //                     }
        //                 }
        //             }

        //             while(!pq.empty())
        //             {
        //                 auto [d,x] = pq.top();
        //                 pq.pop();
        //                 if(d != dist[x])continue;
        //                 for(auto& e : adj[x]))
        //                 {
        //                     if(dist[e.to] > d + e.w)
        //                     {
        //                         dist[e.to] = d + e.w;
        //                         parent
        //                     }
        //                 }
        //             }
        //         }
        //     };
        // }
    } // namespace D250711
} // namespace Y25M7