#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>
#include <vector>
#include <chrono>
#include <map>

namespace D241204
{
    // Message Structure
    struct Message
    {
        int id; // Producer/Consumer - specific ID
        string content; // Message content
        Message(int _id, string _content) : id(_id), content(_content){}
    };

    // Template based message communication code # 1
    template<typename T = Message>
    class MessageQueue
    {
    private:
        map<int, queue<Message>> queues; // Map of ID-specific queues
        std::mutex mx;
        std::condition_variable cv;
    public:
        // Produce a message for a specific ID
        bool produce(int id, const Message& message, int timeout_ms = 1000)
        {
            std::unique_lock<std::mutex> lock(mx, std::defer_lock);
            // if(!lock.try_lock_for(chrono::milliseconds(timeout_ms)))
            // {
            //     throw runtime_error("Mutex lock timeout during produce!");
            // }
            queues[id].push(message);
            cv.notify_all(); // Notify all consumers
            return true;
        }

        // consume message for a specific ID
        T consume(int id, int timeout_ms = 1000)
        {
            std::unique_lock<std::mutex> lock(mx);
            if(!cv.wait_for(lock, chrono::milliseconds(timeout_ms), [this, id](){
                return queues.find(id) != queues.end() && !queues[id].empty();
            })){
                throw runtime_error("Timeout while waiting for a message!");
            }

            Message message = queues[id].front();
            queues[id].pop();
            if(queues[id].empty())
            {
                queues.erase(id);
            }
            return message;
        }
    };

    // Logger class
    class Logger
    {
    private:
        std::mutex log_mx;

        // Get current timestamp in string format
        string get_timestamp()
        {
            auto now = chrono::system_clock::now();
            auto in_time_t = chrono::system_clock::to_time_t(now);
            auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;
            stringstream ss;
            ss << put_time(localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << "." << ms.count();
            return ss.str();
        }
    public:
        void log(const std::string& message)
        {
            std::lock_guard<std::mutex> lock(log_mx);
            std::cout << "[" << get_timestamp() << "] [LOG] : " << message << "\n";
        }
    };

    class Task2
    {
    private:
        // Producer function
        static void producer(MessageQueue<Message>& mq, Logger& logger, int id, int num_messages)
        {
            for(int i=0;i<num_messages;++i)
            {
                try
                {
                    Message message(id, "Message " + to_string(i + 1));
                    logger.log("Producer + " + to_string(id) + " produced : " + message.content);
                    mq.produce(id, message);
                    this_thread::sleep_for(chrono::milliseconds(91));
                }catch(const exception& e)
                {
                    logger.log("Producer err : " + string(e.what()));
                }
            }
        }

        // Consumer function
        static void consumer(MessageQueue<Message>& mq, Logger& logger, int id, int num_messages)
        {
            for(int i=0;i<num_messages;++i)
            {
                try
                {
                    Message message = mq.consume(id);
                    logger.log("consumer + " + to_string(id) + " consumed : " + message.content);
                }catch(const exception& e)
                {
                    logger.log("Consumer error : " + string(e.what()));
                }
                this_thread::sleep_for(chrono::milliseconds(100)); 
            }
        }

    public:
        static int Run()
        {
            MessageQueue mq;
            Logger logger;
            int num_processes = 3;
            int num_consumers = 3;
            int num_messages = 5;

            vector<thread> threads;

            for(int i=0;i<num_consumers;++i)
            {
                threads.emplace_back(thread(consumer, ref(mq), ref(logger), i + 1, num_messages));
            }

            for(int i=0;i<num_processes;++i)
            {
                threads.emplace_back(thread(producer, ref(mq), ref(logger), i + 1, num_messages));
            }

            for(auto& thread : threads)
            {
                if(thread.joinable())thread.join();
            }

            cout << "All threads finished \n";
            return 0;
        }
    };
}
