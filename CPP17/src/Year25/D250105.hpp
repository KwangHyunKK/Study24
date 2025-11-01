#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Y25M1
{
    class ThreadPool
    {
    public:
        ThreadPool(size_t num_threads);
        ~ThreadPool();

        void EnqueueJob(std::function<void()> job);

    private:
        size_t num_threads_;
        std::vector<std::thread> worker_threads_;
        std::queue<std::function<void()>> jobs_;
        std::condition_variable cv_job_q_;
        std::mutex m_job_q_;

        bool stop_all;

        void WorkerThread();
    };

    ThreadPool::ThreadPool(size_t num_threads)
        : num_threads_(num_threads), stop_all(false)
    {
        // # TODO : Find out hardware concurrency counts and 
        worker_threads_.reserve(num_threads_);
        for(size_t i = 0;i<num_threads_;++i)
        {   
            worker_threads_.emplace_back([this]() {this->WorkerThread(); });
        }
    }

    void ThreadPool::WorkerThread()
    {
        // # TODO : In this case, there should be error-handling code 
        while(true)
        {
            std::unique_lock<std::mutex> lock(m_job_q_);
            cv_job_q_.wait(lock, [this]() { return !this->jobs_.empty() || stop_all; });
            if(stop_all && this->jobs_.empty())return;
        
            std::function<void()> job = std::move(jobs_.front());
            jobs_.pop();
            lock.unlock();

            job();
        }
    }

    ThreadPool::~ThreadPool()
    {
        stop_all = true;
        cv_job_q_.notify_all();

        for(auto& t : worker_threads_)t.join();
    }

    void ThreadPool::EnqueueJob(std::function<void()> job)
    {
        if(stop_all)
        {
            throw std::runtime_error("ThreadPool 사용 중지");
        }

        {
            std::lock_guard<std::mutex> lock(m_job_q_);
            jobs_.push(std::move(job));
        }
        cv_job_q_.notify_one();
    }

    namespace D250105
    {
        void work(int t, int id)
        {
            // # TODO : add thread id and process id
            printf("%d start \n", id);
            std::this_thread::sleep_for(std::chrono::seconds(t));
            printf("%d end after %ds\n", id, t);
        }
    }
}