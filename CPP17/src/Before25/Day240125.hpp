#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <cstring>

/**
 * @brief
 * 
 */
namespace Day240125
{
/**
 * @brief C++ Variadic template practice
 * 
 */
namespace Prac1
{
    /**
     * @brief print function
     * 
     * @tparam T 
     * @param arg 
     */
    template<typename T>
    void print(T arg)
    {
        std::cout << arg << "\n";
    }

    /**
     * @brief Variadic template print function
     * 
     * @tparam T 
     * @tparam Types 
     * @param arg 
     * @param args 
     */
    template<typename T, typename... Types>
    void print(T arg, Types... args)
    {
        std::cout << arg << ", ";
        print(args...);
    }

    //////////////////////////// Practice 2
    /// @brief Concat을 진행하는데 할당을 한 번만 할 수 있게 줄여보자

    size_t GetStringSize(const char* s) {return strlen(s); }

    size_t GetStringSize(const std::string& s) {return s.size(); }

    template<typename String, typename... Strings>
    size_t GetStringSize(const String& s, Strings... strs)
    {
        return GetStringSize(s) + GetStringSize(strs...);
    }

    void AppendToString(std::string* concat_str){ return; }

    template<typename String, typename... Strings>
    void AppendToString(std::string* concat_str, const String& s, Strings... strs)
    {
        concat_str->append(s);
        AppendToString(concat_str, strs...);
    }

    template <typename String, typename... Strings>
    std::string StrCat(const String& s, Strings... strs)
    {
        size_t total_size = Prac1::GetStringSize(s, strs...);

        std::string concat_str;
        concat_str.reserve(total_size);

        concat_str = s;
        AppendToString(&concat_str, strs...);

        return concat_str;
    }

    /// C++ 17부터 Flod Expression
    /// 기존 가변 길이 템플릿은 재귀 호출의 방식으로 진행되었다.
    // C++ 17부터는 훨씬 간단하게 표현할 수 있다.

    ///////////////////// Prac3
    template<typename... Ints>
    int sum_all(Ints... nums)
    {
        return (... + nums);
    }

}
    /**
     * @brief run print
     * 
     */
    void run1()
    {
        Prac1::print(1, 3.1, "abc");
        Prac1::print(1,2,3,4,5,6,7);
        return;
    }

    /**
     * @brief run StrCat
     * 
     */
    void run2()
    {
        std::cout << Prac1::StrCat(std::string("this"), " ", "is", " ", std::string("a"), " ", std::string("sentence"));

        return;
    }

    void run3()
    {
        std::cout << Prac1::sum_all(1,23,4,5,65) << "\n";
        return;
    }

}