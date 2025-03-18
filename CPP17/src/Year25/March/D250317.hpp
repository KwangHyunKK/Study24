// Requirement
// make message handler using Dispatch and invoke function
// with C++
// and verified by gtest
#include <iostream>
#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>

// in first, it will be used in single thread.

namespace Y25M3
{
    class EventDispatcher_v1
    {
    public:
        using Handler = std::function<void(int)>;

        void RegisterHandler(int _eventId, Handler _handler)
        {
            std::lock_guard<std::mutex> locK(mutex_);
            handlers_[_eventId].push_back(_handler);
        }

        void UnRegisterHandler(int _eventId)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            handlers_.erase(_eventId);
        }

        void Dispatch(int _eventId, int _data)
        {
            std::vector<Handler> copy;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if(handlers_.find(_eventId) == handlers_.end())
                {
                    return;
                }
                copy = handlers_[_eventId];
            }

            for(const auto& handler : copy)
            {
                Invoke(handler, _data);
            }
        }
    private:
        void Invoke(const Handler& _handler, int _data)
        {
            _handler(_data);
        }

        std::unordered_map<int, std::vector<Handler>> handlers_;
        std::mutex mutex_;
    };
}

