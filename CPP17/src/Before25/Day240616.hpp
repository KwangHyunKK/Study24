#include <iostream>
#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <string>

namespace Day240616
{
    template<typename T>
    class Channel
    {
    public:
        Channel() : closed(false) {}

        void Write(const T& value)
        {
            std::unique_lock<std::mutex> locK(mutex);
            queue.push(value);
            condition.notify_one();
        }

        bool Read(T& value)
        {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [&]{return !queue.empty() || closed; });
            if(!queue.empty())
            {
                value = queue.front();
                queue.pop();
                return true;
            }
            return false;
        }

        void Close()
        {
            std::unique_lock<std::mutex> lock(mutex);
            closed = true;
            condition.notify_all();
        }

    private:
        std::queue<T> queue;
        std::mutex mutex;
        std::condition_variable condition;
        std::atomic<bool> closed;
    };

    class Producer
    {
    public:
        Producer(Channel<std::string>& _channel) : channel(_channel) {}

        void ProduceData()
        {
            for(int i=0;i<5;++i)
            {
                std::string message = "Data " + std::to_string(i);
                channel.Write(message);
                std::cout <<"Producer : Sent " << message << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            channel.Close();
        }
    private:
        Channel<std::string>& channel;

    };

    class Consumer
    {
    public:
        Consumer(Channel<std::string>& _channel) : channel(_channel){}

        void ConsumeData()
        {
            std::string message;
            while(channel.Read(message))
            {
                std::cout << "Consumer : Received " << message << "\n";
            }
            std::cout << "Consumer : Channel closed " << "\n";
        }
        private:
            Channel<std::string>& channel;
    };

    void run()
    {
        Channel<std::string> channel;

        Producer producer(channel);
        Consumer consumer(channel);

        std::thread producerThread(&Producer::ProduceData, &producer);
        std::thread consumerThread(&Consumer::ConsumeData, &consumer);

        producerThread.join();
        consumerThread.join();

        return;
    }
};

