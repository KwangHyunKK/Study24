#include <iostream>
#include <vector>
#include <ranges>

// forward declarations for practice functions
void run_practices1();
void run_coroutines_practice();
void run_chrono_practice();
void run_modules_practice();

int main() {
    std::vector<int> v{1,2,3,4,5,6,7,8,9,10};
    auto even = v | std::views::filter([](int n){ return n % 2 == 0; });

    std::cout << "Even numbers:";
    for (int n : even) std::cout << ' ' << n;
    std::cout << '\n';

    // Call C++20 practice demonstrations
    run_practices1();
    run_coroutines_practice();
    run_chrono_practice();
    run_modules_practice();

    return 0;
}
