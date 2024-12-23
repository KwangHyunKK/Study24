#include <iostream>
#include <future>
#include <stdexcept>
#include <thread>
#include <future>

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
    }
}