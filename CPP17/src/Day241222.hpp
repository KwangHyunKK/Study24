#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <map>
#include <atomic>
#include <chrono>
#include <future> // For async


namespace D241222
{
    /// @brief # 1 Information of compiler, architecture and C++ version
    void printCompilerInfo()
    {
        std::cout << "== Compiler Information === \n";

        // Compiler 
        #if defined(__clang__)
            std::cout << "Compiler : Clang/LLVM\n";
            std::cout << "Version: " << __clang_major__ << "." << __clang_patchlevel__ << "\n"; 
        #elif defined(__GNUC__) || defined(__GNUG__)
            std::cout << "Compiler : CNU GCC/C++\n";
            std::cout << "version : " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "\n";
        #elif defined(_MSC_VER)
            std::cout << "Compiler : Microsoft Visual Studio\n";
            std::cout << "Version : " << _MSC_VER << "\n";
        #else
            std::cout << "Compiler : Unknown\n";
        #endif

        // Standard C++ Version
        std::cout << "=== C++ Standard Version ===\n";
        #if __cplusplus == 199711L
            std::cout << "C++98/03\n";
        #elif __cplusplus == 201103L
            std::cout << "C++11\n";
        #elif __cplusplus == 201402L
            std::cout << "C++14\n";
        #elif __cplusplus == 201703L
            std::cout << "C++17\n";
        #elif __cplusplus == 202002L
            std::cout << "C++20 or later\n";
        #else   
            std::cout << "Pre-standard C++ or unknown version\n";
        #endif
        
        // Architecture information
        std::cout << "=== Architecture ===\n";
        #if defined(__x86_64__) || defined(_M_X64)
            std::cout << "x64 (64-bit)\n";
        #elif defined(__i386__) || defined(_M_IX86)
            std::cout << "x86 (32-bit)\n";
        #elif defined(__arm__) || defined(_M_ARM)
            std::cout << "ARM\n";
        #elif defined(__aarch64__)
            std::cout << "ARM64\n";
        #else
            std::cout << "Unknown architecture" << std::endl;
        #endif

        // OS information
        std::cout << "=== Operating System===\n";
        #if defined(_WIN32) || defined(_WIN64)
            std::cout << "Windows\n";
        #elif defined(__linux__)
            std::cout << "Linux\n";
        #elif defined(__APPLE__) || defined(__MACH__)
            std::cout << "MacOS\n";
        #elif defined(__unix__)
            std::cout << "Unix-like OS\n";
        #else
            std::cout << "Unknown OS\n";
        #endif
    }


    /// @brief # 2 actor class
    // Actor class with async message handling
    class Actor
    {
    public:
        Actor(int id = 0) : actor_id(id), stop(false)
        {
            std::cout << actor_id << ": " << __PRETTY_FUNCTION__ << "\n";
            worker = std::thread([this]() { this->process_messages(); });
        }
        ~Actor()
        {
            std::cout << actor_id << ": " << __PRETTY_FUNCTION__ << "\n";
            // signal stop and wait for the thread to join
            send_message([this](){ stop = true; });
            if(worker.joinable())
            {
                worker.join();
            }
        }

        // Send a message to the actor
        std::future<void> send_message(std::function<void()> msg)
        {
            std::cout << actor_id << ": " << __PRETTY_FUNCTION__ << "\n";
            auto task = std::make_shared<std::packaged_task<void()>>(msg);
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                message_queue.push([task]() { (*task)(); });
            }
            cv.notify_one();
            return task->get_future();
        }

        // Check if the actor is running
        bool is_running() const { return !stop; }

    private:
        int actor_id;
        std::queue<std::function<void()>> message_queue; // message_queue
        std::mutex queue_mutex; // Mutex for queue
        std::condition_variable cv; // cv
        std::thread worker;
        std::atomic<bool> stop; // Stop flag;

        void process_messages()
        {
            std::cout << actor_id << ": " << __PRETTY_FUNCTION__ << "\n";
            while(true)
            {
                std::function<void()> msg;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    cv.wait(lock, [this]() { return !message_queue.empty(); });

                    msg = message_queue.front();
                    message_queue.pop();
                }

                // In this case, msg is not processed and client will never get result about this message???
                // I should check this problem.
                // How to make right test to solve this problem?
                // if(stop.load())break;

                try
                {
                    msg();
                    if(stop.load())break;
                }
                catch(const std::exception& e)
                {
                    std::cerr << "Actor encountered an exception: " << e.what() << "\n";
                }
                catch(...)
                {
                   std::cerr << "Actor encountered an unknown exception." << std::endl;
                }
            }
        }
    };

    // Supervisor class to manager actors
    class Supervisor
    {
    public:
        using ActorPtr = std::shared_ptr<Actor>;

        Supervisor() = default;
        ~Supervisor()
        {
            stop_all_actors();
        }

        // Add a new actor and start managing it
        int add_actor()
        {
            static int actor_id_counter = 0;
            int actor_id = ++actor_id_counter;
            auto actor = std::make_shared<Actor>(actor_id);
            {
                std::lock_guard<std::mutex> lock(mutex);
                actors.emplace(actor_id, actor);
            }
            std::cout << "Actor" << actor_id << " started.\n";
            return actor_id;
        }

        // Send a mesasge to a specific actor asynchronously
        std::future<void> send_message_to_actor(int actor_id, std::function<void()> msg)
        {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = actors.find(actor_id);
            if(it != actors.end() && it->second->is_running())
            {
                return it->second->send_message(std::move(msg));
            }
            else
            {
                std::cerr << "Actor " << actor_id << "is not available.\n";
                std::promise<void> p;
                p.set_exception(std::make_exception_ptr(std::runtime_error("Actor not available")));
                return p.get_future();
            }
        }

        // Stop a specific actor
        void stop_actor(int actor_id)
        {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = actors.find(actor_id);
            if(it != actors.end()){
                it->second.reset();
                actors.erase(it);
                std::cout << "Actor " << actor_id << " stopped.\n";
            }
        }

        // Stop all actors
        void stop_all_actors()
        {
            std::lock_guard<std::mutex> lock(mutex);
            for(auto& [id, actor] : actors)
            {
                actor.reset();
                std::cout << "Actor : " << id << " stopped.\n";
            }
            actors.clear();
        }

    private:
        std::map<int, ActorPtr> actors; // Map of actors
        std::mutex mutex; // Mutex to protect actor map
    };
    class Task
    {
    public:
        static int Run1()
        {
            printCompilerInfo();
            return 0;
        }
    };
}