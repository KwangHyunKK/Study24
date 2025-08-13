cmake_minimum_required(VERSION 3.24)
project(gtk4_calendar_notes CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

add_executable(calendar main.cpp)

target_include_directories(calendar PRIVATE ${GTK4_INCLUDE_DIRS})
target_link_libraries(calendar PRIVATE ${GTK4_LIBRARIES})
target_compile_options(calendar PRIVATE ${GTK4_CFLAGS_OTHER})