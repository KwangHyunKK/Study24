# Minimum CMake version
cmake_minimum_required(VERSION 3.14)
project(Prac VERSION 0.1.0 LANGUAGES C CXX)

# set C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options to enable or disalbe main executable and tests
option(GTEST "Build the tests using Google Test" ON)
# set(GTEST true)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")



if(GTEST)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Enable testing
    include(CTest)
    enable_testing()

    # Collect test sources
    file(GLOB_RECURSE TEST_SOURCES "${TEST_DIR}/*.cc" "${TEST_DIR}/*.cpp")

    # # header files from src/ dir
    # file(GLOB_RECURSE HEADER_FILES "${SRC_DIR}/*.hpp")

    # Add the test executable
    message(STATUS "TEST_SOURCES : ${TEST_DIR} , ${SRC_DIR}")
    add_executable(run_tests ${TEST_SOURCES})

    include_directories(${SRC_DIR})
    add_test(NAME run_tests COMMAND $<TARGET_FILE:gtest>)

    # link the test executable with google test and the source code
    target_link_libraries(run_tests PRIVATE GTest::gtest GTest::gtest_main PUBLIC)

else()
    add_executable(Prac main.cpp)
    include_directories(SRC_DIR)
endif()

# add_executable(Prac ${SOURCES})

add_compile_options(-lpthread)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
