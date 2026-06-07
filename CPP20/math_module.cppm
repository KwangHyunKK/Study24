// math_module.cppm
// C++20 Module interface unit: defines and exports math functions.
// Compiled as a module with -fmodules-ts flag on GCC.

// Export module declaration - makes this a primary module interface unit
export module math_module;

// Export namespace containing our math operations
export namespace math {
    // Function: add two integers and return the sum
    // Exported so importers can call it after importing the module
    int add(int a, int b) {
        return a + b; // simple addition
    }

    // Function: multiply two integers and return the product
    int multiply(int a, int b) {
        return a * b; // simple multiplication
    }

    // Function: compute factorial of n (recursive)
    // Pre-condition: n >= 0
    // Post-condition: returns n! (or 1 if n == 0)
    int factorial(int n) {
        // Base case: 0! = 1 and 1! = 1
        if (n <= 1) return 1;
        // Recursive case: n! = n * (n-1)!
        return n * factorial(n - 1);
    }

    // Function: check if a number is prime
    // Returns true if num is prime, false otherwise
    bool is_prime(int num) {
        // Primes must be greater than 1
        if (num <= 1) return false;
        // 2 is the only even prime
        if (num == 2) return true;
        // Even numbers > 2 are not prime
        if (num % 2 == 0) return false;
        // Check odd divisors up to sqrt(num)
        for (int i = 3; i * i <= num; i += 2) {
            if (num % i == 0) return false;
        }
        // num is prime
        return true;
    }
}
