cmake_minimum_required(VERSION 3.20)
project(example2 CXX)

# default settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# warning level
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# option : SAN / Coverage
option(ENABLE_SANITIZERS "Enable Address/UB sanitizers on supported compilers" ON)
option(ENABLE_CODE_COVERAGE "Enable code coverage flags (GCC/Clang, Debug)" OFF)

# lib (production code)
add_library(core STATIC
    src/add.cpp
)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Google Test (FetchContent)
include(FetchContent)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # MSVC runtime consistenty
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# enable test
enable_testing()
add_subdirectory(tests)

# ctest
add_custom_target(test-all
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run all tests (ctest)"
)