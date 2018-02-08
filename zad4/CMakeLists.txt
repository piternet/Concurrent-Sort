cmake_minimum_required(VERSION 2.8.9)
project(concurrent-sort)

set(CMAKE_C_FLAGS "-std=c11 -D_POSIX_C_SOURCE" CACHE STRING "" FORCE)

add_executable(sort sort.h sort.c)

target_link_libraries(sort rt pthread)
