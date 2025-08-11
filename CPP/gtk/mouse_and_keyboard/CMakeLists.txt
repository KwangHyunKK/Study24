cmake_minimum_required(VERSION 3.24)
project(Mouse_and_Keyboard)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE PkgConfig::GTK4)