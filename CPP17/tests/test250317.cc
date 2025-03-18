#include <iostream>
#include <vector>
#include <gtest/gtest.h>
#include "../src/Year25/March/D250317.hpp"
using namespace Y25M3;

TEST(Test250317, RegisterAndDispatch)
{
    EventDispatcher_v1 dispatcher;
    int result = 0;

    dispatcher.RegisterHandler(1, [&result](int data)
    {
        result = data;
    });

    dispatcher.Dispatch(1, 42);

    EXPECT_EQ(result, 42);
}

TEST(Test250317, UnregisterHandler) {
    EventDispatcher_v1 dispatcher;
    int result = 0;

    dispatcher.RegisterHandler(1, [&result](int data) {
        result = data;
    });

    dispatcher.UnRegisterHandler(1);
    dispatcher.Dispatch(1, 42);
    EXPECT_EQ(result, 0);  // Handler가 삭제되었으므로 값이 변하지 않아야 함
}

TEST(Test250317, MultipleHandlers) {
    EventDispatcher_v1 dispatcher;
    int count = 0;

    dispatcher.RegisterHandler(1, [&count](int) { count++; });
    dispatcher.RegisterHandler(1, [&count](int) { count++; });

    dispatcher.Dispatch(1, 10);
    EXPECT_EQ(count, 2);  // 두 개의 핸들러가 호출되어야 함
}

TEST(Test250317, NoHandleForEvent) {
    EventDispatcher_v1 dispatcher;
    int result = 0;

    dispatcher.Dispatch(99, 42);  // 존재하지 않는 이벤트 ID
    EXPECT_EQ(result, 0);  // 아무런 영향이 없어야 함
}