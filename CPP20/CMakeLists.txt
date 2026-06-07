cmake_minimum_required(VERSION 3.15)

project(CPP20_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# GCC C++20 modules (experimental) notes:
# GCC 11+ supports C++20 modules with -fmodules-ts flag.
# However, CMake support and module caching is still evolving.
# For a fully portable example, we use header-based module interface (math_module.hpp)
# which is equivalent in practice and works across all C++ compilers.
#
# To use true C++20 modules when GCC/CMake support is mature:
#   1. Replace math_module.hpp with math_module.cppm (module interface)
#   2. Use: add_library(math_module OBJECT math_module.cppm)
#   3. Use: target_compile_options(...PRIVATE -fmodules-ts)
#   4. Link: target_link_libraries(target PRIVATE math_module)

add_executable(main main.cpp practices_1.cpp practices_coroutines.cpp practices_chrono.cpp practices_modules.cpp)

# Individual practice executables for isolated testing
add_executable(practices1_exec main_practices1.cpp practices_1.cpp)
add_executable(coroutines_exec main_coroutines.cpp practices_coroutines.cpp)
add_executable(chrono_exec main_chrono.cpp practices_chrono.cpp)
# modules_exec: uses header-based module interface (math_module.hpp)
add_executable(modules_exec main_modules.cpp practices_modules.cpp)

# Optionally set properties for ease of use (e.g., output directories)
set_target_properties(practices1_exec coroutines_exec chrono_exec modules_exec PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
