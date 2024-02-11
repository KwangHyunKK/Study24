cmake_minimum_required(VERSION 3.0.0)
project(Prac VERSION 0.1.0 LANGUAGES C CXX)

# set C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CTest)
enable_testing()

# include_directories(src)
# file(GLOB SOURCES "src/*.hpp")

add_executable(Prac main.cpp)
# add_executable(Prac ${SOURCES})

add_compile_options(-lpthread)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
