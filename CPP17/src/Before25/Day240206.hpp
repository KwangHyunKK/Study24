#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
#include <condition_variable>
#include <queue>
#include <string>

namespace Day240206
{
    void worker(int& result, std::mutex& m)
    {
        for(int i=0;i<1000;++i)
        {
            std::unique_lock<std::mutex> lck(m);
            result += 1;
        }
    }

    int run1()
    {
        int counter = 0;
        std::mutex m;

        std::vector<std::thread>  workers;
        for(int i=0;i<4;++i)
        {
            workers.push_back(std::thread(worker, std::ref(counter), std::ref(m)));
        }

        for(int i=0;i<4;++i)workers[i].join();

        std::cout << "Counter 최종 : " << counter << "\n";
        return 0;
    }

    void producer(std::queue<std::string>* download_pages, std::mutex* m, int index, std::condition_variable* cv)
    {
        for(int i=0;i<5;++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * index));
            std::string content = "웹사이트 : " + std::to_string(i) + " from thread(" + std::to_string(index) + ")\n";

            std::lock_guard<std::mutex>(*m);
            download_pages->push(content);
            cv->notify_one();
        }
    }

    void consumer(std::queue<std::string>* downloaded_pages, std::mutex* m, int *num_processed, std::condition_variable* cv)
    {
        while(*num_processed < 25)
        {
            std::unique_lock<std::mutex> lk(*m);

            cv->wait(lk, [&]{return !downloaded_pages->empty() || *num_processed == 25; });

            if(*num_processed == 25){
                lk.unlock();
                return;
            }

            std::string content = downloaded_pages->front();
            downloaded_pages->pop();

            (*num_processed)++;
            lk.unlock();

            std::cout << content;
            std::this_thread::sleep_for(std::chrono::microseconds(80));
        }
    }

    int run2()
    {
        std::queue<std::string> downloaded_pages;
        std::mutex m;
        std::condition_variable cv;

        std::vector<std::thread> producers;
        for(int i=0;i<5;++i)
        {
            producers.push_back(std::thread(producer, &downloaded_pages, &m, i+1, &cv));
        }
        int num_processed = 0;
        std::vector<std::thread> consumers;
        for (int i = 0; i < 3; i++) {
            consumers.push_back(
                std::thread(consumer, &downloaded_pages, &m, &num_processed, &cv));
        }

        for (int i = 0; i < 5; i++) {
            producers[i].join();
        }

        cv.notify_all();

        for (int i = 0; i < 3; i++) {
            consumers[i].join();
        }
        return 0;
    }
}