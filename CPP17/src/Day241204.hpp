#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>
#include <vector>

namespace D241204
{
    // Template based message communication code # 1
    template<typename T>
    class MessageQueue
    {
    private:
        std::queue<T> queue; // message queue
        std::mutex mx;
        std::condition_variable cv;
    public:
        // append message
        void produce(const T& message)
        {
            std::unique_lock<std::mutex> lock(mx);
            queue.push(message);
            cv.notify_one();
        }

        // consume message
        T consume()
        {
            std::unique_lock<std::mutex> lock(mx);
            cv.wait(lock, [this]() {return !queue.empty(); });
            T message = queue.front();
            queue.pop();
            return message;
        }
    };

    // Logger class
    class Logger
    {
    private:
        std::mutex log_mx;
    public:
        void log(const std::string& message)
        {
            std::lock_guard<std::mutex> lock(log_mx);
            std::cout << "[LOG] : " << message << "\n";
        }
    };

    class Task1
    {
    private:
        static void producer(MessageQueue<std::string>& mq, Logger& logger, int id)
        {
            for(int i=0;i<5;++i)
            {
                std::string message = "Message " + std::to_string((i + 1) * id) + " from Producer " + std::to_string(id);
                mq.produce(message);
                logger.log("Produced: " + message);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        static void consumer(MessageQueue<std::string>& mq, Logger& logger, int id)
        {
            for(int i=0;i<5;++i)
            {
                auto message = mq.consume();
                logger.log("Consumer " + std::to_string((i + 1) * id) + " consumed : " + message);
                std::this_thread::sleep_for(std::chrono::milliseconds(130));
            }
        }
    public:
        static int Run()
        {
            MessageQueue<std::string> mq;
            Logger logger;

            std::vector<std::thread> threads;
            threads.emplace_back(std::thread(producer, std::ref(mq), std::ref(logger), 1));
            threads.emplace_back(std::thread(producer, std::ref(mq), std::ref(logger), 2));
            threads.emplace_back(std::thread(consumer, std::ref(mq), std::ref(logger), 1));
            threads.emplace_back(std::thread(consumer, std::ref(mq), std::ref(logger), 2));

            for(auto& thread : threads)
            {
                if(thread.joinable())thread.join();
            }

            logger.log("All threads finished \n");

            return 0;
        }
    };
}
