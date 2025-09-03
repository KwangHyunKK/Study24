cmake_minimum_required(VERSION 3.20)

# ---- Project ----
project(QuickTDD CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default build type = Debug (if not specified)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ---- Options ----
option(BUILD_TESTING "Build tests" ON) # `include(CTest)` 와 연동
option(WARN_AS_ERRORS "Treat warnings as errors" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer (Clang/GCC)" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer (Clang/GCC)" OFF)
option(ENABLE_COVERAGE "Enable gcov/llvm-cov coverage (Debug, Clang/GCC)" OFF)

# ---- Warnings ----
function(target_set_warnings tgt)
    if(MVSC)
        target_compile_options(${tgt} PRIVATE /W4 /permissive-)
        if(WARN_AS_ERRORS)
            target_compile_options(${tgt} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${tgt} PRIVATE -Wall -Wextra -Wpedantic)
        if(WARN_AS_ERRORS)
            target_compile_options(${tgt} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# ---- Production code (optional) ----
# src/*.cpp가 있으면 core lib으로 묶는다
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
if(CORE_SOURCES)
    add_library(core ${CORE_SOURCES})
    target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_set_warnings(core)
endif()

# ---- Sanitizers / Coverage ----
function(target_enable_sanitizers tgt)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        if(ENABLE_ASAN)
            target_compile_options(${tgt} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
            target_link_options(${tgt} PRIVATE -fsanitize=address)
        endif()
        if(ENABLE_UBSAN)
            target_compile_options(${tgt} PRIVATE -fsanitize=undefined -fno-omit-frame-pointer)
            target_link_options(${tgt} PRIVATE -fsanitize=undefined)
        endif()
        if(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(${tgt} PRIVATE --coverage)
            target_link_options(${tgt} PRIVATE --coverage)
        endif()
    endif()
endfunction()

# ---- Testing (GoogleTest via FetchContext) ----
include(CTest) # BUILD_Testing option 
if(BUILD_TESTING)
    include(FetchContent)
    # fixed version
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    )
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GEST OFF CACHE BOOL "" FORCE)
    # MSVC CRT 일관성
    set(gtest_forece_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Test source find
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/tests/*_tests.cpp
        ${CMAKE_SOURCE_DIR}/tests/*Tests.cpp)    

    if(TEST_SOURCES)
        add_executable(unit_tests ${TEST_SOURCES})
        target_set_warnings(unit_tests)
        target_enable_sanitizers(unit_tests)
        target_link_libraries(unit_tests PRIVATE GTest::gtest GTest::gtest_main)
        if(TARGET core)
            target_link_libraries(unit_tests PRIVATE core)
            target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
        endif()

        include(GoogleTest)
        # Test auto register
        gtest_discover_tests(unit_tests
            DISCOVERY_TIMEOUT 60
            PROPERTIES
                LABELS "unit"
                ENVIRONMENT "GTEST_COLOR=1"
        )

        # CTest default option
        set(CTEST_OUTPUT_ON_FAILURE ON)
    else()
        message(WARNING "No test sources found under tests/. Add *_test.cpp files.")
    endif()
endif()