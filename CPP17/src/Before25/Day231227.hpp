// include for program
#include <thread>
#include <atomic>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <future>
#include <numeric>
#include <vector>
#include <functional>

namespace Day231227
{
    std::atomic<int> global; // 전역 원자성 변수
    #define MAX_THREADS 2

    // thread1
    void func1(int func, int loops){
        // 인수로 제공된 loop만큼 지속적으로 1를 global 변수에 힘한다.
        for(int i=0;i<loops;++i){
            ++global; // 전위 단항 증가 연산자를 사용, 전역 변수를 하나 증가
            if((i%10) == 0)std::this_thread::yield();
        }

        printf("%d. global = %d\n", func, global.load());
    }

    void run4(){
        std::thread t[MAX_THREADS];
        // 원자성 변수를 초기화
        global.store(0, std::memory_order_relaxed);
        for(int i=0;i<MAX_THREADS;++i){
            t[i] = std::thread(func1, i, 100000);
        }

        for(int i=0;i<MAX_THREADS;++i){
            t[i].join();
        }

        printf("global = %d\n", global.load());
    }

    void func2(int func, int loops){
        for(int i=0;i<loops;++i){
            atomic_fetch_add(&global, 1);
            if((i%10) == 0)std::this_thread::yield();
        }

        printf("%d. global = %d\n", func, atomic_load(&global));
    }

    void run5(){
        std::thread t[MAX_THREADS];
        atomic_store(&global, 0);
        for(int i=0;i<MAX_THREADS;++i){
            t[i] = std::thread(func2, i, 10000);
        }

        for(int i=0;i<MAX_THREADS;++i){
            t[i].join();
        }

        printf("global = %d\n", atomic_load(&global));
    }
    
    void run6()
    {
        std::mutex mlock;

        std::thread th01([&](){
            for(int i=0;i<5;++i){
                // guard 변수의 선언과 동시에 뮤텍스 잠금을 설정
                // Wrapper 클래스처럼 기존 기능에 새로운 기능 추가 
                std::lock_guard<std::mutex> guard(mlock);
                std::cout << i << " ";
            } // unlock() 호출되고 뮤텍스 잠금 해제
            std::cout << "\n";
        });

        th01.join();
    }

    class ThreadSafeCounter
    {
    public:
        ThreadSafeCounter() = default;

        // 아래 함수는 다수의 스레드로부터 멤버 변수의 데이터를 읽을 때 사용
        unsigned int get() const{
            std::shared_lock lock(mutex);
            return value;
        }

        void increment(){
            std::unique_lock lock(mutex);
            value++;
        }

    private:
        mutable std::shared_mutex mutex;
        unsigned int value = 0;
    };

    void run7(){
        std::shared_mutex guard;
        ThreadSafeCounter counter;
        std::thread t[3];

        auto increment_and_print = [&](){
            for(int i=0;i<3;++i){
                counter.increment();
                unsigned int c = counter.get();

                guard.lock(); // 배타적 잠금을 설정
                std::cout << std::this_thread::get_id() << ", " << c << "\n";
                guard.unlock(); // 배타적 잠금 해제
            }
        };

        for(int i=0;i<MAX_THREADS;++i){
            t[i] = std::thread(increment_and_print);
        }

        for(int i=0;i<MAX_THREADS;++i){
            t[i].join();
        }
    }

    std::mutex mtx; // mutex
    std::condition_variable cv; // condition_variable
    static int ranking = 1; // 작업 순서 

    void my_print(int id){
        std::unique_lock<std::mutex> umtx(mtx); // unique_lock => condition_variable lock

        cv.wait(umtx); // wait condition_variable
        std::cout << ranking++ << ". thread" << id << "\n";
    }

    int run8(){
        std::thread threads[10];

        for(int i=0;i<10;++i){
            threads[i] = std::thread(my_print, i);
        }

        std::cout << "10개의 스레드가 레이스 경쟁\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        {
            std::unique_lock<std::mutex> umtx(mtx);
            cv.notify_all(); // Send signal to every thread.
        } // unlock mutex

        for(auto& th : threads) th.join();
        return 0;
    }

    void accumulate(std::vector<int>::iterator first,
                    std::vector<int>::iterator last, 
                    std::promise<int>&& accumulate_promise){
        int sum = std::accumulate(first, last, 0);
        accumulate_promise.set_value(sum);
    }

    std::future<int> launch_promise(std::vector<int>::iterator first,
                                    std::vector<int>::iterator last){
        // promise 객체 생성
        std::promise<int> accumulate_promise;
        // future 객체 반호나
        std::future<int> result = accumulate_promise.get_future();
        // thread 객체 생성 함수의 인수로 std::promise 객체 전달
        std::thread work_thread(accumulate, first, last, std::move(accumulate_promise));
        work_thread.detach();
        return result;
    }

    int run9(){
        int total = 0;
        std::vector<int> numbers = {1,2,3,4,5,6,7,8,9,10};

        // promise 객체가 작업할 환경을 만들고 future 객체를 반환받는다.
        std::future<int> s[2];
        s[0] = launch_promise(numbers.begin(), numbers.end() + 6);
        s[1] = launch_promise(numbers.begin() + 6, numbers.end());

        for(int i=0;i<2;++i){
            int w = s[i].get();
            std::cout << "promise-" << i << " : " << w << "\n";
            total += w;
        }

        std::cout << "합계 : " << total << "\n";

        // promise 객체를 전달하는 인수는 반드시 rvalue 참조를 사용
        // 그렇지 않으면 중복, 에러 발생
        return 0;
    }
}