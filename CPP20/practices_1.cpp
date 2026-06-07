// practices_1.cpp
// Examples of several C++20 language/library features with line-level comments.

#include <iostream>
#include <vector>
#include <ranges>
#include <span>
#include <compare>
#include <string>

// consteval: function is evaluated at compile-time when given constant expressions.
consteval int consteval_square(int n) { return n * n; }

// constinit: ensures the variable is zero-initialized at program start and
// prevents dynamic initialization order issues.
constinit int global_counter = 0;

// Concept definition: 'Addable' requires that 'a + b' is a valid expression.
template<typename T>
concept Addable = requires(T a, T b) { a + b; };

// Function constrained by the 'Addable' concept. Uses abbreviated template syntax.
auto add(Addable auto a, Addable auto b) {
    return a + b; // returns sum for types that support +
}

// Defaulted three-way comparison (spaceship operator) example.
// 'operator<=>' is defaulted so compiler generates ordering and equality.
struct Point {
    int x{}; // x coordinate
    int y{}; // y coordinate
    auto operator<=>(const Point&) const = default; // enables <, >, ==, etc.
};

// Abbreviated template for multiplication (works for arithmetic types).
auto multiply(auto a, auto b) {
    return a * b;
}

// Public entry to run this set of practice snippets.
void run_practices1() {
    std::cout << "--- C++20 Practices 1 ---\n";

    // consteval: evaluated at compile time and usable in constexpr contexts.
    constexpr int s = consteval_square(5); // computed at compile-time
    std::cout << "consteval_square(5) = " << s << "\n";

    // constinit: ensures 'global_counter' is initialized before dynamic code runs.
    global_counter = 1; // runtime assignment (initialization guaranteed earlier)
    std::cout << "global_counter = " << global_counter << "\n";

    // Concepts + abbreviated templates in action with integers
    std::cout << "add(2,3) = " << add(2,3) << "\n"; // uses add<int>

    // Concepts work with user types like std::string (operator+ defined)
    std::string a = "Hello, "; // string left operand
    std::string b = "World";   // string right operand
    std::cout << "add(strings) = " << add(a,b) << "\n"; // concatenation

    // Three-way comparison example
    Point p1{1,2}, p2{1,3};
    if (p1 < p2) std::cout << "p1 < p2\n"; // compares x then y
    else if (p1 == p2) std::cout << "p1 == p2\n";
    else std::cout << "p1 > p2\n";

    // std::span: lightweight non-owning view over contiguous data
    int arr[] = {10,20,30,40};
    std::span<int> sp(arr); // view over 'arr'
    std::cout << "span contents:";
    for (auto v : sp) std::cout << ' ' << v; // iterate without copying
    std::cout << '\n';

    // Ranges: compose views - filter even numbers then square them
    std::vector<int> v{1,2,3,4,5,6};
    auto rng = v
               | std::views::filter([](int n){ return n % 2 == 0; }) // keep evens
               | std::views::transform([](int n){ return n * n; });  // square

    std::cout << "squared even numbers:";
    for (int x : rng) std::cout << ' ' << x; // will print 4 16 36
    std::cout << "\n";

    // Abbreviated template multiply: generic multiplication
    std::cout << "multiply(3,4) = " << multiply(3,4) << "\n";

    std::cout << "--- End practices ---\n";
}
