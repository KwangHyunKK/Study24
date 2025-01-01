#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

class EventHandler {
public:
    EventHandler() : event_received(false), retry_count(0) {}

    // 이벤트를 기다리는 함수
    bool waitForEvent() {
        std::unique_lock<std::mutex> lock(mtx);

        // 100ms 동안 이벤트 대기
        if (cv.wait_for(lock, std::chrono::milliseconds(100), [this] { return event_received; })) {
            // 이벤트를 정상적으로 받음
            event_received = false;
            return true;
        } else {
            // 타임아웃 발생
            return false;
        }
    }

    // 이벤트를 받는 콜백 함수
    void onEventReceived() {
        std::lock_guard<std::mutex> lock(mtx);
        event_received = true;
        cv.notify_one();
    }

    // 이벤트 전송을 시도하는 함수
    bool sendEvent() {
        while (retry_count < 3) {
            std::cout << "이벤트 전송 시도 #" << retry_count + 1 << std::endl;

            if (waitForEvent()) {
                std::cout << "이벤트 수신 성공!" << std::endl;
                return true;
            } else {
                retry_count++;
                std::cout << "이벤트 수신 실패, 재시도..." << std::endl;
            }
        }

        std::cout << "3회 재시도 실패, 이벤트 전송 실패" << std::endl;
        return false;
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    bool event_received;
    int retry_count;
};

// 이벤트를 발생시키는 예제 함수
void triggerEvent(EventHandler& handler) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 이벤트가 발생하는 데 50ms 소요
    handler.onEventReceived();
}

int run() {
    EventHandler handler;

    // 이벤트를 발생시키는 스레드 생성
    std::thread event_thread(triggerEvent, std::ref(handler));

    // 이벤트 전송 시도
    if (handler.sendEvent()) {
        std::cout << "이벤트 처리 완료." << std::endl;
    } else {
        std::cout << "이벤트 처리 실패." << std::endl;
    }

    event_thread.join(); // 이벤트 발생 스레드 종료 대기

    return 0;
}