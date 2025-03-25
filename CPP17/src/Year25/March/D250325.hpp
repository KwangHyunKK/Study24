// Purpose : Progress bar 
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <random>

namespace Y25M3
{
    class ProgressBar
    {
    public:
        ProgressBar() : progress_(0.0f), running_(true) {}

        ~ProgressBar()
        {
            if(updateThread_.joinable())updateThread_.join();
        }

        void start() 
        {
            updateThread_ = std::thread(&ProgressBar::updateLoop, this);
        }

        void stop()
        {
            running_ = false;
        }

        void setProgress(float value)
        {
            progress_ = std::clamp(value, 0.0f, 1.0f);
        }

        void bindingProgress(std::function<float()> updater)
        {
            progressUpdater_ = updater;
        }

    private:
        std::atomic<float> progress_;
        std::function<float()> progressUpdater_;
        std::atomic<bool> running_;
        std::mutex printMutex_;
        std::thread updateThread_;

        void updateLoop()
        {
            while(running_)
            {
                auto start_time = std::chrono::steady_clock::now();
                if(progressUpdater_)
                {
                    setProgress(progressUpdater_());
                }

                displayProgress();

                auto target_duration = std::chrono::milliseconds(16); // 60Hz
                auto elapsed  = std::chrono::steady_clock::now() - start_time;

                if(elapsed < target_duration)
                {
                    auto remaining = target_duration - elapsed;
                    std::this_thread::sleep_for(remaining);
                }
            }
        }

        void displayProgress()
        {
            std::lock_guard<std::mutex> lock(printMutex_);
            int barWidth = 50;
            int pos = static_cast<int>(barWidth * progress_.load());

            std::cout << "\r[";
            for(int i=0;i<barWidth;++i)
            {
                if(i < pos) std::cout << "=";
                else if(i==pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << int(progress_.load() * 100.0) << " %";
            std::cout.flush();
        }
    };

    void Run()
    {
        ProgressBar bar;
        float currentProgress = 0.0f;

        bar.bindingProgress([&]()->float{
            return currentProgress;
        });

        bar.start();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(50, 100);
        int random_value = dist(gen);
        
        for(int i=0;i<=100;++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(random_value));
            currentProgress = i / 100.0f;
        }

        bar.stop();
        std::cout << "\n완료!\n";
        return;
    }
}