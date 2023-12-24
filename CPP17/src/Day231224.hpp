#pragma once
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <initializer_list>

/// @brief Array  - iterator
/// @tparam Type 
template<typename Type>
class Array{
    public:
        typedef int size_type;

    class iterator{
        public:
            // 아래 타입은 반드시 선언
            typedef Type value_type;
            typedef Type& reference;
            typedef Type* pointer;
            typedef std::forward_iterator_tag iterator_category;
            typedef std::ptrdiff_t difference_type;

            iterator(pointer ptr = nullptr) : _ptr(ptr) {}

            iterator operator++() {++_ptr; return *this; }
            iterator operator++(int) {iterator tmp(*this); ++_ptr; return tmp; }

            bool operator==(const iterator& rhs) { return _ptr == rhs._ptr; }
            bool operator!=(const iterator& rhs) { return _ptr == rhs._ptr; }

            reference operator*() { return *_ptr; }
            pointer operator->() { return _ptr; }

        private:
            Type* _ptr;
    };

    // Array 객체
    Array(size_type size = 0) : _size(size){
        if(0 == size) _data = nullptr;
        else _data = new Type[_size](); // 초기화

    }

    Array(std::initializer_list<Type> init) : _size(init.size()){
        _data = new Type[_size];
        std::copy(init.begin(), init.end(), _data);
    }

    template<typename _InputIterator>
    constexpr Array(_InputIterator _first, _InputIterator _last){
        size_type size = _last - _first;
        _data = new Type[size];

        std::copy(_first, _last, _data);
        _size = size;
    }

    ~Array(){
        if(_data){
            delete[] _data;
            _size = 0;
        }
    }

    constexpr size_type size() const { return _size; }

    Type& operator[](size_type index){
        if(index >= _size || index < 0)
            std::terminate(); // throw std::runtime_error("index가 잘못되었습니다.");
        return _data[index];
    }

    const Type& operator[](size_type index) const{
        if(index >= _size || index <0)
            throw std::runtime_error("index가 잘못되었습니다.");
        return _data[index];
    }

    // 범위 기반 for문을 위해서 다음과 같이 begin() 함수와 end() 함수 진행
    iterator begin(){
        return iterator(_data);
    }

    iterator end(){
        return iterator(_data + _size);
    }

private:
    Type* _data;
    size_type _size;
};

struct Account{
public:
    Account() = default;

    Account(const char* id, const char* name, int bal){
        strcpy(this->account, id);
        strcpy(this->name, name);
        this->balance = bal;
    }
    char account[20];
    char name[20];
    int balance;
};

int run1()
{
    Array<double> array = {2.3, 3.2, 4.3, 16.78, 9.4, 3.14};
    // for(Array<double>::iterator i = array.begin(); i != array.end();++i){
    //     std::cout << *i << " ";
    // } // 위의 구문이 실행되지 않는다.
    for(int i=0;i<array.size();++i)
    {
        std::cout << array[i] <<  " ";
    }
    std::cout << "\n";

    Array<Account> list = {
        { "120-324-549053", "홍길동", 50000}, 
        { "120-324-542393", "이순신", 32400}
    };

    // for(auto& i : list){
    //     printf(" 계좌 : %s, 소유자 : %s", i.account, i.name);
    //     printf(", 잔액 : %i\n", i.balance);
    // }

    for(int i=0;i <list.size();++i)
    {
        std::cout << list[i].account << ", " << list[i].name << "\n";
    }

    std::vector<double> vec;

    std::copy(array.begin(), array.end(), std::back_inserter(vec));

    for(std::vector<double>::iterator i = vec.begin();i != vec.end();++i){
        std::cout << *i << " ";
    }

    std::cout << "안녕\n";
    return 0;
}


////////////////////// 타입 변환 연산자

// for enum

enum Days {Sunday = 0, Monday = Sunday + 1, Tuesday, wednesday, Thursday, Friday, Saturday = Sunday + 6};
enum class Months : short {Jan = 1, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec };
const char* week[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

const char* DayName(Days day)
{
    if(day < Sunday || day > Saturday)return "뭐라는거야 ";
    return week[day];
}

/// 1. static_cast
int run2()
{
    /// 일반 변환 ///
    int n = 100;
    void* nv = &n; // 암시적 변환 작업
    
    // long 타입의 포인터가 정수 타입의 변수를 가리키도록
    long* n01 = static_cast<long*>(nv);
    int* n02 = static_cast<int*>(&n);

    std::cout << "n = " << n << ", *n01 = " << *n01 << ", *n02 = " << *n02 << "\n";

    std::vector<long> e = {1, 23, 10};
    void* voidp = &e;

    std::vector<int> * vec = static_cast<std::vector<int>*>(voidp);\

    std::vector<int> vr = *vec;
    for(int a: vr)
        std::cout << a << " ";
    std::cout << "\n";


    ///  열거형 변환 ///
 
    Months thisMonth = Months::Dec;

    Days today = static_cast<Days>(thisMonth);
    std::cout << DayName(today) << "는 " << today + 1 << "번째 요일이다.\n";

    Months nextMonth = (Months)((short)Months::Dec - 12 + 1);
    today = static_cast<Days>(nextMonth);
    std::cout << DayName(today) << "는 " << today + 1 << "번째 요일이다.\n";
    
    // 열거형 타입의 요소로부터 int 또는 float으로 변환은 다음과 같다.
    float one = static_cast<float>(today);
    std::cout << DayName(today) << "는 " << one + 1 << "번째 요일\n";
    

    /// 일반 변수를 rvalue로 변환 ///
    int s = 10;
    const int& cs = s;
    std::cout << static_cast<int&&>(s); 
    return 0;
}   

// 3. dynamic_cast

class Base{
    int state;
public:
    Base() : state(0) {}
    Base(int i) : state(i) {}
    virtual int dummy() {
        std::cout << "Base dummy() 호출\n";
        return state;
    }
};

// Derived class 
class Derived : public Base{
public:
    Derived(int i) : Base(i) {}
    void SetNumber(int i){
        number = i;
    }
    int number;
};

int run3()
{
    Derived* pd;
    // 암시적 변환으로 Derived 객체를 Base로 만든다
    Base* pba = new Derived(23);

    pd = dynamic_cast<Derived*>(pba);
    if(nullptr == pd)std::cout << "첫 번째 pba 객체의 변환 실패\n";

    pd->SetNumber(10);
    std::cout << pd->number << "\n";
    std::cout << pd->dummy() << "\n";
    delete pba;

    Base* pbb = new Base(568);
    pd = dynamic_cast<Derived*>(pbb);
    if(nullptr == pd)std::cout << "두 번째 pbb 객체의 변환 실패\n";

    pd = static_cast<Derived*>(pbb);
    pd->SetNumber(110);
    std::cout << pd->number << "\n";
    std::cout << pd->dummy() << "\n";
    delete pbb;
    return 0;
}


