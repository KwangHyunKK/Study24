#include <iostream>



namespace D241222
{
    /// @brief # 1 Information of compiler, architecture and C++ version
    void printCompilerInfo()
    {
        std::cout << "== Compiler Information === \n";

        // Compiler 
        #if defined(__clang__)
            std::cout << "Compiler : Clang/LLVM\n";
            std::cout << "Version: " << __clang_major__ << "." << __clang_patchlevel__ << "\n"; 
        #elif defined(__GNUC__) || defined(__GNUG__)
            std::cout << "Compiler : CNU GCC/C++\n";
            std::cout << "version : " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "\n";
        #elif defined(_MSC_VER)
            std::cout << "Compiler : Microsoft Visual Studio\n";
            std::cout << "Version : " << _MSC_VER << "\n";
        #else
            std::cout << "Compiler : Unknown\n";
        #endif

        // Standard C++ Version
        std::cout << "=== C++ Standard Version ===\n";
        #if __cplusplus == 199711L
            std::cout << "C++98/03\n";
        #elif __cplusplus == 201103L
            std::cout << "C++11\n";
        #elif __cplusplus == 201402L
            std::cout << "C++14\n";
        #elif __cplusplus == 201703L
            std::cout << "C++17\n";
        #elif __cplusplus == 202002L
            std::cout << "C++20 or later\n";
        #else   
            std::cout << "Pre-standard C++ or unknown version\n";
        #endif
        
        // Architecture information
        std::cout << "=== Architecture ===\n";
        #if defined(__x86_64__) || defined(_M_X64)
            std::cout << "x64 (64-bit)\n";
        #elif defined(__i386__) || defined(_M_IX86)
            std::cout << "x86 (32-bit)\n";
        #elif defined(__arm__) || defined(_M_ARM)
            std::cout << "ARM\n";
        #elif defined(__aarch64__)
            std::cout << "ARM64\n";
        #else
            std::cout << "Unknown architecture" << std::endl;
        #endif

        // OS information
        std::cout << "=== Operating System===\n";
        #if defined(_WIN32) || defined(_WIN64)
            std::cout << "Windows\n";
        #elif defined(__linux__)
            std::cout << "Linux\n";
        #elif defined(__APPLE__) || defined(__MACH__)
            std::cout << "MacOS\n";
        #elif defined(__unix__)
            std::cout << "Unix-like OS\n";
        #else
            std::cout << "Unknown OS\n";
        #endif
    }

    class Task
    {
    public:
        static int Run1()
        {
            printCompilerInfo();
            return 0;
        }
    };
}