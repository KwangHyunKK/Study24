#include <atomic>
#include <cassert>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <random>

namespace Y25M7
{
    namespace D250710
    {
        class WorkStealingThreadPool
        {
        public:
            WorkStealingThreadPool(size_t n)
                : queues_(n), done_(false)
                {
                    for(size_t i=0;i<n;++i)
                    {
                        workers_.emplace_back([this, i]{this->worker_loop(i); });
                    }
                }

            ~WorkStealingThreadPool()
            {
                done_.store(true);

                for(auto& cv : cvs_)cv.notify_all();
                for(auto& t : workers_)t.join();
            }

            void submit(std::function<void()> job)
            {
                // round-robin 
                auto idx = submit_counter_++ % queues_.size();
                {
                    std::lock_guard<std::mutex> lk(mtxs_[idx]);
                    queues_[idx].push_front(std::move(job));
                }

                cvs_[idx].notify_one();
            }
        private:
            using Job = std::function<void()>;

            void worker_loop(size_t idx)
            {
                std::mt19937_64 rd(std::random_device{}());
                std::uniform_int_distribution<size_t> dist(0, queues_.size() -1);

                while(!done_.load())
                {
                    Job job;

                    // 1. Get job from my queue
                    {
                        std::unique_lock<std::mutex> lk(mtxs_[idx]);
                        cvs_[idx].wait_for(lk, std::chrono::milliseconds(50), [&]{
                            return !queues_[idx].empty() || done_.load();
                        });

                        if(!queues_[idx].empty())
                        {
                            job = std::move(queues_[idx].front());
                            queues_[idx].pop_front();
                        }
                    }

                    // 2. If my queue is empty -> steal from random worker
                    if(!job)
                    {
                        size_t victim = dist(rd);
                        if(victim != idx)
                        {
                            std::lock_guard<std::mutex> lk(mtxs_[victim]);
                            if(!queues_[victim].empty())
                            {
                                job = std::move(queues_[victim].back());
                                queues_[victim].pop_back();
                            }
                        }
                    }

                    // 3. Execute
                    if(job)job();
                }
            }
            std::vector<std::thread> workers_;
            std::vector<std::deque<Job>> queues_;
            std::vector<std::mutex> mtxs_{queues_.size()};
            std::vector<std::condition_variable> cvs_{queues_.size()};
            std::atomic<bool> done_;
            std::atomic<size_t> submit_counter_{0};
        }; // WorkStealingThreadPool
    } // namespace D250710
} // namespace Y25M7