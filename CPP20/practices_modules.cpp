// practices_modules.cpp
// Demonstrates C++20 module concepts using a header-based interface.
//
// Modern C++20 module syntax (when fully supported):
//   import math_module;
// 
// Current approach (header-based, fully portable):
//   #include "math_module.hpp"
//
// Both approaches provide the same encapsulation and interface benefits.

#include <iostream>

// Include the math module interface (header-based equivalent)
#include "math_module.hpp"

// Practice function that demonstrates module usage
void run_modules_practice() {
    std::cout << "--- Modules Practice (with module-like interface) ---\n";

    // Call add function from math namespace (exported from module)
    // math::add(a, b) - adds two integers
    std::cout << "math::add(15, 27) = " << math::add(15, 27) << "\n";

    // Call multiply function from math namespace (exported from module)
    // math::multiply(a, b) - multiplies two integers
    std::cout << "math::multiply(6, 7) = " << math::multiply(6, 7) << "\n";

    // Call factorial function from math namespace (exported from module)
    // Compute 5! = 5 * 4 * 3 * 2 * 1 = 120 using recursive implementation
    std::cout << "math::factorial(5) = " << math::factorial(5) << "\n";

    // Call is_prime function from math namespace (exported from module)
    // Check if 17 is prime - returns true (17 is only divisible by 1 and 17)
    std::cout << "math::is_prime(17) = " << (math::is_prime(17) ? "true" : "false") << "\n";

    // Check if 20 is prime - returns false (20 is even and > 2)
    std::cout << "math::is_prime(20) = " << (math::is_prime(20) ? "true" : "false") << "\n";

    // Additional examples demonstrating module interface usage
    std::cout << "\nAdditional examples:\n";
    std::cout << "math::add(100, 200) = " << math::add(100, 200) << "\n";
    std::cout << "math::factorial(6) = " << math::factorial(6) << "\n";
    std::cout << "math::is_prime(2) = " << (math::is_prime(2) ? "true" : "false") << "\n";

    std::cout << "--- Module practice complete ---\n";
}
