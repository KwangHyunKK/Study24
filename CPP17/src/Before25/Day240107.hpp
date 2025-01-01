#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <string_view>
#include <future>
#include <condition_variable>
#include <mutex>
#include <exception>
#include <chrono>

namespace Date240107
{
    // string View : C++17부터 도입된 새로운 문자열 타입
    // 읽기 전용 뷰 : 원본 문자열 데이터 수정. 읽기 전용 뷰 제공
    // 복사 없음 : 문자열 데이터를 복사하지 않는다. 문자열 처리 성능 향상
    // 메모리 안정성 : 문자열 데이터를 복사하지 않는다. 메모리 안전성 보장
    // 호환성 : 다른 문자열 타입과 호환되는 타입으로 변환 가능
    int func1()
    {
        std::string_view view("Hello, World!");
        std::size_t index = view.find("World");
        if(index != std::string_view::npos){
            std::cout << "Found 'World' at index" << index << "\n";
        }else{
            std::cout << "Not found\n";
        }
        return 0;
    }

    void worker(std::promise<std::string>* p)
    {
        p->set_value("some data");
        std::cout << "worker is ending!!\n";
    }

    int func2()
    {
        std::promise<std::string> p;

        // 미래에 string 데이터를 돌려주겠다.
        std::future<std::string> data = p.get_future();

        std::thread t(worker, &p);

        std::cout << "worker 일해라!\n";

        // 미래에 약속된 데이터를 받을 때까지 대기
        data.wait();

        std::cout << "받은 데이터 : " << data.get() << "\n";

        t.join();

        return 0;
    }

    std::condition_variable cv;
    std::mutex m;
    bool done = false;
    std::string info;

    void worker2(){
        {
            std::lock_guard<std::mutex> lk(m);
            info = "some data";
            done = true;
        }
        cv.notify_all();
    }


    int func3()
    {
        std::thread t(worker2);

        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, []{return done;}); // data.wait
        lk.unlock();

        std::cout << "받은 데이터 : " << info << "\n";

        t.join();
        return 0;
    }

    void worker4(std::promise<void>* p){
        std::this_thread::sleep_for(std::chrono::seconds(10));
        p->set_value();
    }

    int func4()
    {
        std::promise<void> p;

        std::future<void> data = p.get_future();
        std::thread t(worker4, &p);

        while(true)
        {
            std::future_status status = data.wait_for(std::chrono::seconds(1));

            // 아직 준비가 안됨
            if(status == std::future_status::timeout){
                std::cerr << ">";
            }else if(status == std::future_status::ready){
                break;
            }
        }
        t.join();

        return 0;
    }

    void runner(std::shared_future<void> start)
    {
        start.get();
        std::cout << "출발!\n";
    }

    int func5()
    {
        std::promise<void> p;
        std::shared_future<void> start = p.get_future();

        std::thread t1(runner, start);
        std::thread t2(runner, start);
        std::thread t3(runner, start);
        std::thread t4(runner, start);

        // 참고로 cerr는 std::cout 과 다르게 버퍼를 사용하지 않기 때문에 터미널에
        // 바로 출력
        std::cerr << "준비...";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cerr << "땅!\n";
        
        p.set_value();

        t1.join();
        t2.join();
        t3.join();
        t4.join();

        return 0;
    }

}