#include <iostream>
#include <thread>
#include <future>
#include <functional>
#include <cstddef>
#include <initializer_list>
#include <numeric>
#include <stdexcept>


namespace Y25M7
{
    namespace D250723
    {
        namespace Prac1
        {
            int square(int x)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500)); // sleep with latency
                return x * x;
            }

            void Run1()
            {
                std::packaged_task<int(int)> task(square);
                std::future<int> result = task.get_future();

                std::thread worker(std::move(task), 10);

                std::cout << "Calculating...\n";

                int value = result.get();
                std::cout << "result : " << value << "\n";
                if(worker.joinable()) worker.join();
            }

            void RunwithCancelFlag()
            {
                std::atomic<bool> cancelFlag{false};

                auto longCompute = [&](int steps) -> int {
                    int result = 0;
                    for(int i=0;i<steps;++i)
                    {
                        if(cancelFlag.load())
                        {
                            std::cout << "[Worker] failure, exception\n";
                            throw std::runtime_error("job canceled");
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        result += (i + 1);
                        std::cout << "[Worker] step " << (i + 1) << "/" << steps << ", partial result = " << result << "\n";
                    }
                    return result;
                };

                std::packaged_task<int()> task(std::bind(longCompute, 5));

                std::future<int> fut = task.get_future();
                std::thread worker(std::move(task));

                std::this_thread::sleep_for(std::chrono::milliseconds(550));
                std::cout << "[Main] 0.55 second, cancel\n";
                cancelFlag.store(true);

                try{
                    int value = fut.get();
                    std::cout << "[Main] Success Done. Result = " << value << "\n";
                }catch(const std::exception &e)
                {
                    std::cout << "[Main] Fail with exception : " << e.what() << "\n";
                }  
                if(worker.joinable())  worker.join();
                return;
            } // namespace Prac1

            namespace Vector
            {   
                template<typename T, std::size_t N>
                class NDArray
                {
                public:
                    // dims : {dim0, dim1, ... dimN-1}
                    NDArray(const std::array<std::size_t, N>& dims)
                        : dims_(dims), data_(computeTotalSize(dims)), strides_(computeStrides(dims)) {}

                    // variable index access : arr(i0, i1, ...)
                    template<typename... Idx>
                    T& operator()(Idx... idx)
                    {
                        static_assert(sizeof...(Idx) == N, "index count is different from dims");
                        std::array<std::size_t> indices { std::size_t(idx)... };
                        std::size_t lin = 0;
                        for(std::size_t i=0;i<N;++i)
                        {
                            if(indices[i] >= dims_[i])throw std::out_of_range("Out of Index");
                            lin += indices[i] * strides_[i];
                        }
                        return data_[lin];
                    }

                private:
                    std::array<std::size_t, N> dims_; // if dims == 3, (x : 2,y : 3,z : 4) 
                    std::vector<T> data_; // the real arrays which contains data.
                    std::array<std::size_t, N> strides_; // The const value to get value without calculation.

                    static std::size_t computeTotalSize(const std::array<std::size_t, N>& d)
                    {
                        return std::accumulate(begin(d), end(d), std::size_t{1}, std::multiplies<>());
                    }

                    static std::array<std::size_t, N> computeStrides(const std::array<std::size_t, N>& d)
                    {
                        std::array<std::size_t, N> s;
                        s[N-1] = 1;
                        for(int i=int(N) -2;i >=0; --i)
                        {
                            s[i] = s[i + 1] * d[i + 1];
                        }
                        return s;
                    }
                };
            }
        } // namespace Prac1
    } // namespace D250723
}