#include <iostream>
#include <functional>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <gtest/gtest.h>
#include "../src/Year25/D250105.hpp"
using namespace Y25M1;

TEST(TestD250105, ThreadPoolTest1)
{
    ThreadPool pool(3);

    for(int i=0;i<10;++i)
    {
        pool.EnqueueJob([i]() { D250105::work(i % 3 + 1, i); });
    }
}
