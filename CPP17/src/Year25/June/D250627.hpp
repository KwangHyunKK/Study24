#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <iostream>

namespace Y25M6
{

    namespace TaskGraph
    {
        enum class TaskStatus
        {
            Pending,
            Running,
            Completed,
            Failed,
            Suspended
        };

        struct Task
        {
            std::string id;
            std::function<void()> func; // # 1. Improvement : Need input and output
            // # 2. This function is only synchronous function. Async function.
            std::function<bool()> completionCondition;
            std::function<void()> checkpoint;
            std::vector<std::string> deps;
            std::vector<std::string> children;
            std::atomic<TaskStatus> status{TaskStatus::Pending};
        }; // Task struct

        class TaskGraph
        // Use Topology graph, make Task dependencies and order.
        {
        public:
            // Add a new task with its dependencies
            // # 3. In runtime, this TaskGraph can't be updated.
            // # 4. Need to check to detect cycle.
            void addTask(const std::string &id, std::function<void()> func, std::function<bool()> completionCondition,
                         std::function<void()> checkpoint, const std::vector<std::string> &dependencies)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                Task &t = tasks_[id];
                t.id = id;
                t.func = std::move(func);
                t.completionCondition = std::move(completionCondition);
                t.checkpoint = std::move(checkpoint);
                t.deps = dependencies;
                indegree_[id] = dependencies.size();
                for (auto &d : dependencies)
                {
                    tasks_[d].children.push_back(id);
                }
            }

            // Execute all tasks in parallel respecting dependencies
            void execute()
            {
                // Initialize ready queue
                for (auot &kv : tasks_)
                {
                    if (indegree_[kv.first] == 0)
                    {
                        readyQueue_.push(kv.first);
                    }

                    // Launch worker threads
                    unsigned nthreads = std::thread::hardware_concurrency(); // static count of thread.
                    for (unsigned i = 0; i < nthreads; ++i)
                    {
                        threads_.emplace_back(&TaskGraph::worker, this);
                    }

                    // Wait for all work to finish
                    {
                        std::unique_lock<std::mutex> lock(mtx_);
                        cv_done_.wait(lock, [this]
                                      { return finishedTasks_ == tasks_.size(); });
                    }

                    // Stop workers
                    stop_ = true;
                    cv_.notify_all();
                    for (auto &t : threads_)
                        t.join();
                }

                // Return list of successfully completed tasks
                std::vector<std::string> getCompletedTasks() const
                {
                    std::lock_guard<std::mutex> lock(mtx_);
                    return completedList_;
                }
            }

        private:
            void worker()
            {
                while (true)
                {
                    std::string id;
                    {
                        std::unique_lock<std::mutex> lock(mtx_);
                        cv_.wait(lock, [this]
                                 { return !readyQueue_.empty() || stop_; });
                        if (stop_ && readyQueue_.empty())
                            return;

                        id = readyQueue_.front();
                        readyQueue_.pop();
                        tasks_[id].status = TaskStatus::Running;
                    }

                    // Optional Checkpoint
                    try
                    {
                        tasks_[id].checkpoint();
                        tasks_[id].func();
                        bool ok = tasks_[id].completionCondition();
                        tasks_[id].status = ok ? TaskStatus::Completed : TaskStatus::Suspended;
                    }
                    catch (...)
                    {
                        tasks_[id].status = TaskStatus::Failed;
                    }

                    // After run ,record success
                    // # 5. If task fails, it should restart task.
                    // # 6. Or if task need to run as transaction, rollback will be needed.
                    {
                        std::lock_guard<std::mutex> lock(mtx_); // # 2. This lock will make other thread stop.
                        // This will be improved by stealing algorithm.
                        if (tasks_[id].status == TaskStatus::Completed)
                        {
                            completedList_.push_back(id);
                        }
                        ++finishedTasks_;
                        // Notify dependencies
                        for (auto &child : tasks_[id].children)
                        {
                            if (--indgree_[child] == 0)
                            {
                                readyQueue_.push(child);
                                cv_.notify_one();
                            }
                        }
                        if (finishedTasks_ == tasks_.size())
                            cv_done_.notify_one();
                    }
                }
            }

            mutable std::mutex mtx_;
            std::condition_variable cv_;
            std::condition_variable cv_done_;
            std::unordered_map<std::string, Task> tasks_;
            std::unordered_map<std::string, size_t> indegree_;
            std::queue<std::string> readyQueue_;
            std::vector<std::thread> threads_;
            bool stop_{false};

            size_t finishedTasks_{0};
            std::vector<std::string> completedList_;
        }; // TaskGraph class

        void Run()
        {
            TaskGraph graph;
            graph.addTask("A", []
                          { std::cout << "Running A\n"; }, []
                          { return true; }, []
                          { std::cout << "Checkpoint A\n"; }, {});
            graph.addTask("B", []
                          { std::cout << "Running B\n"; }, []
                          { return true; }, [] {}, {"A"});
            graph.addTask("C", []
                          { std::cout << "Running C\n"; }, []
                          { return false; }, // simulate suspend
                          [] {}, {"A"});
            graph.addTask("D", []
                          { std::cout << "Running D\n"; }, []
                          { return true; }, [] {}, {"B", "C"});

            graph.execute();
            auto done = graph.getCompletedTasks();
            std::cout << "Completed tasks:";
            for (auto &id : done)
                std::cout << " " << id;
            std::cout << "\n";
            return 0;
        }
    }; // TaskGraph namespace
}