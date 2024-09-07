#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <memory>
#include <future>
#include <deque>
#include <string>
#include <chrono>
#include <random>
#include <thread>
using namespace std;

namespace d240907
{
    // # 1. Newton-Raphson sqrt
    double newtonRaphsonSqrt(double value, double epsilon = 1e-6)
    {
        double guess = value /2;
        while(std::abs(guess * guess - value) > epsilon)
            guess = (guess + value / guess) /2;

        return guess;
    }

    double calculateHyptenuseOptimized(double a, double b)
    {
        double sumOfSuares = a * a + b * b;
        return newtonRaphsonSqrt(sumOfSuares);
    }

    int run1()
    {
        double a = 3.0;
        double b = 4.0;
        double c = calculateHyptenuseOptimized(a, b);

        std::cout << "c : " << c << "\n";
        return 0;
    }


    int run()
    {
        std::thread t1(run1);

        if(t1.joinable())t1.join();

        return 0;
    }
}

