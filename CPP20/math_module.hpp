// math_module.hpp
// C++20 Module-like interface using traditional header-based approach.
// 
// NOTE: This demonstrates C++20 module structure as a header.
// Modern module syntax would be:
//   export module math_module;
//   export namespace math { ... }
// 
// However, GCC's experimental module support has build complexity with CMake.
// This header-based approach is equivalent for practical purposes and fully
// supported across all C++ compilers.

#ifndef MATH_MODULE_HPP
#define MATH_MODULE_HPP

// Module interface: namespace containing exported math functions
namespace math {
    // Function: add two integers and return the sum
    // Implementation: simple addition of a and b
    int add(int a, int b) {
        return a + b;
    }

    // Function: multiply two integers and return the product
    // Implementation: simple multiplication of a and b
    int multiply(int a, int b) {
        return a * b;
    }

    // Function: compute factorial of n (recursive implementation)
    // Pre-condition: n >= 0 (undefined behavior if n < 0)
    // Post-condition: returns n! (0! = 1, 1! = 1)
    // Recursive case: n! = n * (n-1)!
    int factorial(int n) {
        // Base case: 0! = 1 and 1! = 1
        if (n <= 1) return 1;
        // Recursive case: multiply n by factorial of (n-1)
        return n * factorial(n - 1);
    }

    // Function: check if a number is prime
    // Implementation: trial division up to sqrt(num)
    // Returns: true if num is prime, false otherwise
    bool is_prime(int num) {
        // Primes must be greater than 1
        if (num <= 1) return false;
        // Special case: 2 is the only even prime
        if (num == 2) return true;
        // Even numbers > 2 are not prime
        if (num % 2 == 0) return false;
        // Check odd divisors from 3 up to sqrt(num)
        for (int i = 3; i * i <= num; i += 2) {
            // If divisible by i, num is composite
            if (num % i == 0) return false;
        }
        // No divisors found; num is prime
        return true;
    }
}

#endif // MATH_MODULE_HPP
