#include <iostream>
#include <thread>
#include <future>
#include <functional>
#include <vector>
#include <mutex>
#include <atomic>
#include <algorithm>

class EventSystem {
public:
    using HandlerID = std::size_t;

    EventSystem() : handlerCounter_(0) {}

    HandlerID registerHandler(const std::function<void()>& handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        HandlerEntry entry(++handlerCounter_, handler);
        handlers_.push_back(std::move(entry));
        return handlerCounter_;
    }

    void unregisterHandler(HandlerID id) {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_.erase(std::remove_if(handlers_.begin(), handlers_.end(),
            [id](const HandlerEntry& entry) { return entry.id == id; }), handlers_.end());
    }

    void notify() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& entry : handlers_) {
            if (entry.handler) {
                entry.handler();
                entry.promise.set_value();
            }
        }
    }

    std::future<void> getFuture(HandlerID id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find_if(handlers_.begin(), handlers_.end(),
            [id](const HandlerEntry& entry) { return entry.id == id; });
        if (it != handlers_.end()) {
            return it->promise.get_future();
        }
        throw std::runtime_error("Handler ID not found");
    }

private:
    struct HandlerEntry {
        HandlerID id;
        std::function<void()> handler;
        std::promise<void> promise;

        HandlerEntry(HandlerID id, std::function<void()> handler)
            : id(id), handler(std::move(handler)) {}
    };

    std::vector<HandlerEntry> handlers_;
    std::mutex mutex_;
    std::atomic<HandlerID> handlerCounter_;
};

void exampleHandler() {
    std::cout << "Event triggered!" << std::endl;
}

int main() {
    EventSystem eventSystem;

    auto handlerID = eventSystem.registerHandler(exampleHandler);

    std::thread notifier([&eventSystem]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Notify is calling" << std::endl;
        eventSystem.notify();
    });

    std::thread waiter([&eventSystem, handlerID]() {
        auto future = eventSystem.getFuture(handlerID);
        future.wait();
        std::cout << "Handler was called" << std::endl;
    });

    notifier.join();
    waiter.join();

    eventSystem.unregisterHandler(handlerID);

    return 0;
}