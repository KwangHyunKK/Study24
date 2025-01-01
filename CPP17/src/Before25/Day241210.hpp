#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <mutex>
#include <numeric>
#include <functional>
#include <algorithm>
#include <mutex>
#include <stdexcept>
// #include <span> // span can used in C++ 20.

namespace D241210
{
    /// # 1 MapReduce

    std::mutex mtx;
    // Log
    class Logger
    {
    public:
        static void logError(const std::string& message)
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cerr << "[Error] " << message << "\n";
        }
    };
    template<typename InputType, typename MapFunc, typename ReduceFunc>
    class MapReduce
    {
    public:
        // ResultType은 ReduceFunc에 맞춰져야 하지 않나?
        using ResultType = typename std::invoke_result_t<MapFunc, InputType, InputType>;

        MapReduce(MapFunc _mapFunc, ReduceFunc _reduceFunc, size_t _numThreads = std::thread::hardware_concurrency())
            :mapFunc(_mapFunc), reduceFunc(_reduceFunc), numThreads(_numThreads) {}

        template <typename Container>
        ResultType operator()(const Container& data)
        {
            // auto span = std::move(data); // Zero-copy using std::span
            // size_t dataSize = span.size();
            // size_t chunkSize = dataSize / numThreads;

            try
            {
                size_t dataSize = data.size();
                if(dataSize == 0)
                {
                    throw std::invalid_argument("Input data can't be empty.");
                }

                size_t numThreads = std::min((size_t)std::thread::hardware_concurrency(), dataSize);
                size_t chunkSize = dataSize / numThreads;

                // Map
                std::vector<std::future<ResultType>> futures;
                for(size_t i =0;i<numThreads;++i)
                {
                    auto startIt = data.begin() + i * chunkSize;
                    auto endIt = (i == numThreads - 1) ? data.end() : startIt + chunkSize;

                    futures.emplace_back(std::async(std::launch::async, [=](){
                        try{
                            std::cerr << std::this_thread::get_id() << "\n";
                            return mapFunc(startIt, endIt);
                        }catch(const std::exception& e){
                            Logger::logError(e.what());
                            throw; // Re-throw to propagate error
                        }
                    }));
                }

                // Reduce
                std::vector<ResultType> intermediateResults;
                for(auto& fut : futures)
                {
                    intermediateResults.push_back(fut.get());
                }

                return std::accumulate(intermediateResults.begin(), intermediateResults.end(), ResultType(), reduceFunc);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
            
            // // Map 
            // std::vector<std::future<ResultType>> futures;
            // for(size_t i=0;i<numThreads;++i)
            // {
            //     size_t startIdx = i * chunkSize;
            //     size_t endIdx = (i == numThreads - 1) ? dataSize : startIdx + chunkSize;

            //     futures.emplace_back(std::async(std::launch::async, [= , &span](){
            //         ResultType localResult = mapFunc(std::vector(span.begin() + startIdx, span.begin() + endIdx));
            //         return localResult;
            //     }));
            // }

            // // Reduce
            // std::vector<ResultType> intermediateResults;
            // for(auto& fut : futures)
            // {
            //     intermediateResults.push_back(fut.get());
            // }

            // return std::accumulate(intermediateResults.begin(), intermediateResults.end(), ResultType(), reduceFunc);
        }
    private:
        MapFunc mapFunc;
        ReduceFunc reduceFunc;
        size_t numThreads;
    };

    class Task1
    {
    public:
        static int Run1()
        {
            std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

            auto mapFunc = [](auto startIt, auto endIt) -> int {
                return std::accumulate(startIt, endIt, 0, [](int acc, int x) { return acc + x * x; });
            };

            auto reduceFunc = [](int acc, int value) { return acc + value; };

            MapReduce<int, decltype(mapFunc), decltype(reduceFunc)> mapReduce(mapFunc, reduceFunc);
            auto result = mapReduce(data);
            std::cout << "result : " << result << ", answer : 385\n";
            return 0;
        }
    };
}