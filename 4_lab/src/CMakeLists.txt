cmake_minimum_required(VERSION 3.17)
project(4_lab C)

set(CMAKE_C_STANDARD 99)

set(THREADS_PREFER_PTHREAD_FLAG ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
find_package(Threads REQUIRED)

add_executable(4_lab_child_0 child_0.c shrmem.h)
add_executable(4_lab_child_1 child_1.c shrmem.h)
add_executable(4_lab main.c shrmem.h)
target_link_libraries(4_lab PRIVATE Threads::Threads rt)
target_link_libraries(4_lab_child_0 PRIVATE Threads::Threads rt)
target_link_libraries(4_lab_child_1 PRIVATE Threads::Threads rt)

add_dependencies(4_lab 4_lab_child_0)
add_dependencies(4_lab_child_0 4_lab_child_1)