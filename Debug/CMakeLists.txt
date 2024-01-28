cmake_minimum_required(VERSION 3.0.0)
project(Debug VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(Debug main.cpp)

set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
