#pragma once
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <initializer_list>

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

