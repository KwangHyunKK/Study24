#pragma once
#include <iostream>
#include <future>
#include <vector>
#include <numeric>


int accumulate_block_worker_ret(int* data, size_t count){
    int result = 0;
    for(size_t i = 0;i<count;++i)result += *(data + i);
    return result;
}

int run1(){
    std::vector<int> v{1,2,3,4,5,6,7,8,9,10};

    // async으로 대처하기 위해 다음과 같이 프로그래밍
    std::future<int> fut = std::async(std::launch::async, accumulate_block_worker_ret, v.data(), v.size());

    // future 클래스가 제공하는 get() 함수를 사용한다면 작업 결과를 반환 받기 위해
    // async() 함수가 종료될 때까지 대기한다.
    // async() 함수는 스레드의 함수를 실행시키는 역할과 함께 future 객체를 생성하고 반환하는 함수이다.
    std::cout << "배열의 합은 " << fut.get() << "\n";
    return 0;
}

int accumulate_block_worker(int* data, size_t count){
    return std::accumulate(data, data + count, 0);
}

using int_futures = std::vector<std::future<int>>;

int_futures launch_split_workers(std::vector<int>& v){
    int_futures futures;
    // 비동기 스레드 객체를 생성, 벡터 객체에 입력
    size_t length = v.size() / 2;
    futures.push_back(std::async(std::launch::async, accumulate_block_worker, v.data(), length));
    futures.push_back(std::async(std::launch::async, accumulate_block_worker, v.data() + length, length));
    return futures;
}

int run2(){
    std::vector<int> v{1,2,3,4,5,6,7,8,9,10};

    int_futures futures = launch_split_workers(v);
    int result0 = futures[0].get();
    int result1 = futures[1].get();
    std::cout << "각 배열의 합은 " << result0 << " 과" << result1 << "\n";
    std::cout << "두 개 배열의 합은 " << result1 + result0 << "\n";
    return 0;
}


