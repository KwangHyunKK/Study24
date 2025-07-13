cmake_minimum_required(VERSION 3.10)
project(example-1 C)

# 1. pkg-config module load
find_package(PkgConfig REQUIRED)

# 2. gtk 4 package find
pkg_check_modules(GTK4 REQUIRED gtk4)

# 3. include path, definition, library directory setting
include_directories(${GTK4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})
add_definitions (${GTK_4_CFLAGS_OTHER})

# 4. executable target 
add_executable(example-1 example_1.c)

# 5. target_link_libs
target_link_libraries(example-1 ${GTK4_LIBRARIES})