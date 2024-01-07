#include <iostream>
#include "src/Day231224.hpp"
#include "src/Day231227.hpp"
#include "src/Day240107.hpp"

int main(int, char**){
    // std::cout << "Hello, from Prac!\n";
    // run1();
    // std::cout << "=========================\n cast 연산\n===========================\n";
    // run2();
    // std::cout << "dynamic cast\n";
    // run3();

    // std::cout << "\n================Day231227.hpp=============\n";
    // Day231227::run4();
    // std::cout << "\n";
    // Day231227::run5();

    // std::cout << "\n---------------lock_guard : 클래스 선언으로 뮤텍스 선언 ------\n";
    // Day231227::run6();
    // std::cout << "\n--------------- shared_mutex ------\n";
    // Day231227::run7();

    // std::cout << "\n-------------------condition_variable---------\n";
    // Day231227::run8();

    std::cout << "\n=========Day240107.hpp===========\n";
    Date240107::func1();
    std::cout << "\n=============func1()===========\n";
    Date240107::func2();
    std::cout << "\n=============func2()===========\n";
    Date240107::func3();
    std::cout << "\n=============func3()===========\n";
    Date240107::func4();
    std::cout << "\n=============func4()===========\n";
    Date240107::func5();
    std::cout << "\n=============func5()===========\n";
    return 0;
}
