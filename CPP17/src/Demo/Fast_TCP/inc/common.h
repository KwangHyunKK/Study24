#ifndef _DEMO_FAST_TCP_INC_COMMON_H_
#define _DEMO_FAST_TCP_INC_COMMON_H_

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <chrono>
#include <cstring>
#include <cassert>
#include <future>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <cassert>

namespace Demo
{
    namespace Fast_Tcp
    {
        constexpr unsigned int SERVER_PORT = 5535;
        constexpr unsigned int MAX_CONNECTION = 64;
        constexpr unsigned int MAX_WORKER_THREADS = 0;
        constexpr unsigned int BUFFER_SIZE = 4096;
        constexpr unsigned int TEST_CLIENT_COUNT = 16;
        constexpr unsigned int TEST_DURATION_SECONDS = 5;
    } // namespace Fast_Tcp
} // namespace Demo

#endif // _DEMO_FAST_TCP_INC_COMMON_H_