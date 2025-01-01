#include <iostream>
#include <future>
#include <stdexcept>
#include <thread>
#include <future>
#include <vector>
#include <algorithm>

namespace D241224
{
    /// # 1. Simple async example

    template<typename T>
    std::future<T> asyncTask(bool succeed)
    {
        return std::async(std::launch::async, [succeed]() -> T{
            try
            {
                if(!succeed)
                {
                    throw std::runtime_error("Task failed");
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "Task completed successfully.\n";

                if constexpr (!std::is_same_v<T, void>)
                {
                    return T{}; 
                }
            }
            catch(const std::exception& e)
            {
                std::cerr << "Error : " << e.what() << '\n';
                throw;
            }
            
        });
    }

    class Task
    {
    public:
        static void Run1()
        {
            try{
                auto futureVoid = asyncTask<void>(true);
                futureVoid.get();

                auto futureError = asyncTask<void>(false);
                futureError.get();
            }catch(const std::exception& e){
                std::cerr << "Caught exception : " << e.what() << "\n";
            }
            return;
        }
    };

    
    class Tree
    {
    public:
        explicit Tree(int n) : adj(n), depth(n), parent(n, std::vector<int>(20, -1)){}

        void addEdge(int u, int v)
        {
            adj[u].push_back(v);
            adj[v].push_back(u);
        }

        void preprocess(int root = 0)
        {
            dfs(root, -1, 0);
            int n = adj.size();
            for(int j=1;(1<<j) < n; ++j)
            {
                for(int i=0;i<n;++i)
                {
                    if(parent[i][j-1] != -1)
                    {
                        parent[i][j] = parent[parent[i][j-1]][j-1]; // Node i의 j번째 부모 = Node i의 j-1번째 부모의 j-1번째 부모모
                    }
                }
            }
        }

        int lca(int u, int v)
        {
            if(depth[u] < depth[v]) std::swap(u, v);

            int diff = depth[u] - depth[v];

            for(int i=0;diff;++i)
            {
                if(diff & 1) u = parent[u][i];
                diff >>= 1;
            }

            if(u == v) return u;

            for(int i = parent[0].size() - 1;i>=0;--i)
            {
                if(parent[u][i] != parent[v][i])
                {
                    u = parent[u][i];
                    v = parent[v][i];
                }
            }

            return parent[u][0];
        }
    private:
        std::vector<std::vector<int>> adj;
        std::vector<int> depth;
        std::vector<std::vector<int>> parent;

        void dfs(int node, int par, int dep)
        {
            parent[node][0] = par; // node의 0번 째 -> 부모
            depth[node] = dep; // node의 depth -> depth
            for(int neighbor : adj[node])
            {
                if(neighbor != par)
                    dfs(neighbor, node, dep + 1);
            }
        }
    };
}