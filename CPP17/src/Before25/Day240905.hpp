#include <iostream>
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <thread>
#include <future>
#include <condition_variable>
#include <mutex>

namespace D240905
{
    using namespace std;

    // Dispatcher class
    class Dispatcher
    {
    public:
        using EventHandler = std::function<void()>; // Event -> no argument, no return value

        // registerEvent
        void registerEvent(const std::string& eventName, EventHandler handler)
        {
            eventHandlers[eventName].push_back(handler);
        }

        // unregisterEvent
        void unregisterEvent(const std::string& eventName)
        {
            eventHandlers.erase(eventName);
        }

        // event 
        void dispatch(const std::string& eventName)
        {
            auto it = eventHandlers.find(eventName);
            if(it != eventHandlers.end())
            {
                for(const auto& handler : it->second)
                {
                    handler(); // call registered handler
                }
            }
            else
            {
                std::cout << "No handlers for event : " << eventName << "\n";
            }
        }
    private:
        // Map
        std::unordered_map<std::string, std::vector<EventHandler>> eventHandlers;
    };

    int run1()
    {
        Dispatcher dispatcher;

        // Register Event 
        dispatcher.registerEvent("Event 1", [](){
            std::cout <<" Handler 1 for Event1 executed.\n";
        });

        dispatcher.registerEvent("Event 1", [](){
            std::cout << "Handler 2 for Event 1 executed.\n";
        });

        dispatcher.registerEvent("Event 2", [](){
            std::cout << "Handler for Event 2 executed.\n";
        });

        // call event
        std::cout << "Dispatching Event1 : \n";
        dispatcher.dispatch("Event1");

        std::cout << "Dispatching Event 2 : \n";
        dispatcher.dispatch("Event2");

        std::cout << "Dispatching Event3 (no handlers) : \n";
        dispatcher.dispatch("Event3");

        return 0;
    }

    // upgrade Dispatcher to use multithread
    class Dispatcher2
    {
    public:
        using EventHandler = std::function<void()>; 

        // register Event
        void registerEvent(const std::string& eventName, EventHandler handler)
        {
            std::lock_guard<std::mutex> lock(mutex_);

            eventHandlers[eventName].push_back(handler);
        }

        // unregister
        void unregisterEvent(const std::string& eventName)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            eventHandlers.erase(eventName);
        }

        // Event call & async handler call
        void dispatch(const std::string& eventName)
        {
            std::vector<EventHandler> handlers;

            {
                std::lock_guard<std::mutex> lock(mutex_);
                auto it = eventHandlers.find(eventName);
                if(it != eventHandlers.end())
                {
                    handlers = it->second;
                }
                else
                {
                    std::cout << "No handlers for event : " << eventName << "\n";
                    return;
                }
            }

            // To execute each handler asynchronously
            std::vector<std::thread> threads;
            std::vector<std::promise<void>> promises(handlers.size());
            std::vector<std::future<void>> futures;

            for(auto& promise : promises)
            {
                futures.push_back(promise.get_future());
            }

            for(size_t i = 0;i<handlers.size();++i)
            {
                threads.emplace_back([&, i](){
                    handlers[i](); // call handler
                    promises[i].set_value(); // if handler finishes job, promise 
                });

                for(auto& future : futures)future.get(); 

                for(auto& thread : threads)thread.join();
            }
        }

    private:
        std::unordered_map<std::string, std::vector<EventHandler>> eventHandlers;
        std::mutex mutex_; // mutex for eventhandler
    };


    int run2()
    {
        Dispatcher2 dispatcher;

        dispatcher.registerEvent("Event1", []() {
            std::cout << "Handler 1 for Event1 executed on thread " << std::this_thread::get_id() << std::endl;
        });

        dispatcher.registerEvent("Event1", []() {
            std::cout << "Handler 2 for Event1 executed on thread " << std::this_thread::get_id() << std::endl;
        });

        dispatcher.registerEvent("Event2", []() {
            std::cout << "Handler for Event2 executed on thread " << std::this_thread::get_id() << std::endl;
        });

        std::thread t1([&dispatcher](){
            dispatcher.dispatch("Event1");
        });

        std::thread t2([&dispatcher](){
            dispatcher.dispatch("Event2");
        });

        if(t1.joinable())t1.join();
        if(t2.joinable())t2.join();
        
        return 0;
    }
    int run()
    {
        run1();
        run2();

        std::cout << "Fin\n";
        return 0;
    }
}