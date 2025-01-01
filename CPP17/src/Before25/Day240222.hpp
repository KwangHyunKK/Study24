#include <iostream>
#include <memory>
#include <functional>
#include <string>

namespace Day240222
{
    class P
    {
        int *data;
    public:
        P() : data(nullptr)
        {
            data = new int[100];
            std::cout << "Constructor\n";
        }

        ~P() 
        {
            std::cout << "Destructor\n";
            delete[] data;
        }

        void some() {std::cout << "일반 포인터와 동일하게 사용\n"; }
    };

    void do_something() { P *pa = new A(); delete pa; }

    void run1()
    {
        do_something();
        // 메모리 누수가 발생. delete pa 가 없다면
    }

    void do_otherthing()
    {
        std::unique_ptr<P> pa(new P());
        pa->some();

        // 삭제된 함수 사용. : attempting to reference a deleted function
        // unique_ptr의 복사 생성자는 명시적으로 삭제. 객체를 유일하게 소유.
        // 복사 생성 할 수 없다. 단, 이동 생성할 수 있다.
        // std::unique_ptr<P> pb = pa;

        std::unique_ptr<A> pb = std::move(pa);
        std;:cout << "pb : ";
        pb->some(); 
    }

    // unique_ptr를 함수 인자로 전달.
    // 함수 레퍼런스로 전달하면 문제

    void do_dothing(P* ptr) {ptr->some();}

    void run2()
    {
        std::unique_ptr<P> pp(new P());
        do_dothing(pp.get()); // get을 통해서 일시적으로 접근 권한을 줄 수 있다.
    }

    int some_func1(const std::string& s)
    {
        std::cout << "Func1 호출 : " << s << "\n";
        return 0;
    }

    struct S
    {
        void operator()(char c) {std::cout << "Func2 호출!\n"; }
    };

    void run3()
    {
        std::function<int(const std::string&)> f1 = some_func1;
        std::function<void(char)> f2 = S();
        std::function<void()> f3 = []() { std::cout << "Func3 호출!\n"; }
    }

}