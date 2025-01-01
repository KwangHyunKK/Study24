#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <mutex>

// 이벤트 큐
std::queue<int> eventQueue;
std::mutex queueMutex;
std::condition_variable cv;
bool finished = false;

// 이벤트를 생성하는 생산자
void producer(std::promise<void> eventPromise) {
    for (int i = 0; i < 5; ++i) { // 5개의 이벤트를 생성
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 이벤트 발생 시점을 시뮬레이션
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            eventQueue.push(i); // 이벤트 큐에 데이터 추가
        }
        cv.notify_all(); // 모든 소비자에게 알림
    }
    eventPromise.set_value(); // 모든 이벤트 발생 완료를 알림
}

// 이벤트를 소비하는 소비자
void consumer(std::future<void> eventFuture, int consumerId) {
    std::cout << "Consumer " << consumerId << " waiting for event..." << std::endl;
    eventFuture.wait(); // 모든 이벤트가 발생할 때까지 대기
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, []{ return !eventQueue.empty() || finished; }); // 이벤트가 있거나 작업이 완료될 때까지 대기
        if (eventQueue.empty()) {
            break; // 큐가 비어 있으면 종료
        }
        int event = eventQueue.front();
        eventQueue.pop();
        lock.unlock();
        std::cout << "Consumer " << consumerId << " processed event: " << event << std::endl;
    }
}

int run1() {
    std::promise<void> eventPromise;
    std::future<void> eventFuture = eventPromise.get_future();

    std::thread producerThread(producer, std::move(eventPromise));

    std::vector<std::thread> consumerThreads;
    int numConsumers = 3;
    for (int i = 0; i < numConsumers; ++i) {
        consumerThreads.emplace_back(consumer, eventFuture, i + 1);
    }

    producerThread.join();
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        finished = true;
    }
    cv.notify_all(); // 모든 소비자에게 작업이 완료되었음을 알림

    for (auto& consumerThread : consumerThreads) {
        consumerThread.join();
    }

    return 0;
}

// KMP algorithm
#include <iostream>
#include <vector>
#include <string>

// 패턴의 부분 일치 테이블을 계산하는 함수
std::vector<int> computeLPSArray(const std::string& pattern) {
    int M = pattern.length();
    std::vector<int> lps(M);
    int length = 0;
    lps[0] = 0; // lps[0]은 항상 0

    int i = 1;
    while (i < M) {
        if (pattern[i] == pattern[length]) {
            length++;
            lps[i] = length;
            i++;
        } else {
            if (length != 0) {
                length = lps[length - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
    return lps;
}

// KMP 알고리즘을 이용하여 패턴을 텍스트에서 검색하는 함수
void KMPSearch(const std::string& pattern, const std::string& text) {
    int M = pattern.length();
    int N = text.length();

    std::vector<int> lps = computeLPSArray(pattern);

    int i = 0; // text의 인덱스
    int j = 0; // pattern의 인덱스
    while (i < N) {
        if (pattern[j] == text[i]) {
            i++;
            j++;
        }

        if (j == M) {
            std::cout << "Found pattern at index " << i - j << std::endl;
            j = lps[j - 1];
        } else if (i < N && pattern[j] != text[i]) {
            if (j != 0) {
                j = lps[j - 1];
            } else {
                i++;
            }
        }
    }
}

int run2() {
    std::string text = "ABABDABACDABABCABAB";
    std::string pattern = "ABABCABAB";
    KMPSearch(pattern, text);
    return 0;
}


// 0-1 knapsack problem

#include <iostream>
#include <vector>
#include <algorithm>

// 배낭 문제를 해결하는 함수
int knapsack(int capacity, const std::vector<int>& weights, const std::vector<int>& values) {
    int n = weights.size();
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(capacity + 1, 0));

    for (int i = 1; i <= n; ++i) {
        for (int w = 1; w <= capacity; ++w) {
            if (weights[i - 1] <= w) {
                dp[i][w] = std::max(dp[i - 1][w], dp[i - 1][w - weights[i - 1]] + values[i - 1]);
            } else {
                dp[i][w] = dp[i - 1][w];
            }
        }
    }

    return dp[n][capacity];
}

int run3() {
    int capacity = 50;  // 배낭의 용량
    std::vector<int> weights = {10, 20, 30};  // 각 물건의 무게
    std::vector<int> values = {60, 100, 120};  // 각 물건의 가치

    int max_value = knapsack(capacity, weights, values);
    std::cout << "배낭에 담을 수 있는 최대 가치: " << max_value << std::endl;

    return 0;
}

