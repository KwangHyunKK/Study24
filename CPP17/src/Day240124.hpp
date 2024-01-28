#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <string>
#include <functional>
#include <vector>
#include <queue>

namespace Day240124{

auto currentCPUThreadCounts(){
    return std::thread::hardware_concurrency();
}
namespace ThreadPool1{

class ThreadPool{
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    // job 추가
    void EnqueueJob(std::function<void()> job);

private:
    // 총 Worker thread 개수
    size_t num_threads_;

    // Worker 스레드 보관 벡터
    std::vector<std::thread> worker_threads_;

    // 한일들을 보관하는 job queue
    std::queue<std::function<void()>> jobs_;

    // 위의 job 큐를 위한 cv와 m
    std::condition_variable cv_job_q_;
    std::mutex m_job_q_;

    // 모든 쓰레드 종료
    bool stop_all;

    // Worker Thread
    void WorkerThread();
};

/// @brief 생성자 : worker_threads_에 WorkerThread()를 추가시켜주기만 한다. 
/// @param num_threads 
ThreadPool::ThreadPool(size_t num_threads = std::thread::hardware_concurrency())
    : num_threads_(num_threads), stop_all(false)
{
        worker_threads_.reserve(num_threads_);
        for(size_t i=0;i<num_threads_;++i){
            worker_threads_.emplace_back([this](){this->WorkerThread();});
        }
}

/// @brief jobs_에 작업 추가될 때까지 대기, 작업 추가되면 받아서 처리
void ThreadPool::WorkerThread()
{
    while(true)
    {
        std::unique_lock<std::mutex> lock(m_job_q_);
        cv_job_q_.wait(lock, [this](){return !this->jobs_.empty() || stop_all; });
        if(stop_all && this->jobs_.empty())
        {
            return;
        }

        // 맨 앞의 job 제거
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

    for(auto& t : worker_threads_)
    {
        t.join();
    }
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
    cv_job_q_.notify_all();
}

void work(int t, int id)
{
    std::cout << id << ", start\n";
    std::this_thread::sleep_for(std::chrono::seconds(t));
    std::cout << id << " end after " << t << " s\n";
}

}

int run1()
{
    ThreadPool1::ThreadPool pool(3);
    std::cout << "Run1=========================\n\n";
    std::cout << std::thread::hardware_concurrency() << "\n";
    for(int i=0;i<10;++i)
    {
        pool.EnqueueJob([i](){ThreadPool1::work(i%3 + 1, i); });
    }
    return 0;
}

namespace ThreadPool2
{
class ThreadPool
{
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    template<class F, class... Args>
    std::future<typename std::result_of<F(Args...)>::type> EnqueueJob(F&& f, Args&&... args);

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
    :num_threads_(num_threads), stop_all(false)
{
    worker_threads_.reserve(num_threads_);
    for(size_t i=0;i<num_threads_;++i)
    {
        worker_threads_.emplace_back([this]() {this->WorkerThread(); });
    }
}

void ThreadPool::WorkerThread(){
    while(true)
    {
        std::unique_lock<std:: mutex> lock(m_job_q_);
        cv_job_q_.wait(lock, [this](){return !this->jobs_.empty() || stop_all; });
        if(stop_all && this->jobs_.empty()){
            return;
        }

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

    for(auto& t : worker_threads_)
    {
        t.join();
    }
}

template<class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::EnqueueJob(
    F&& f, Args&&... args)
{
    if(stop_all)std::runtime_error("threadpool 사용 중지");

    using return_type = typename std::result_of<F(Args...)>::type;
    auto job = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> job_result_future = job->get_future();
    {
        std::lock_guard<std::mutex> lock(m_job_q_);
        jobs_.push([job](){(*job)();});
    }
    cv_job_q_.notify_all();
    
    return job_result_future;
}

int work(int t, int id)
{
    std::cout << "start " << id << "\n";
    std::this_thread::sleep_for(std::chrono::seconds(t));
    std::cout << id << " : end after " << t << "s\n"; 
    return t * id;
}
}

int run2()
{
    ThreadPool2::ThreadPool pool(3);

    std::vector<std::future<int>> futures;
    for(int i=0;i<10;++i)
    {
        futures.emplace_back(pool.EnqueueJob(ThreadPool2::work, i % 3 + 1, i));
    }
    for(auto& f : futures)
    {
        std::cout << "result : " << f.get() << "\n";
    }
    return 0;
}

}