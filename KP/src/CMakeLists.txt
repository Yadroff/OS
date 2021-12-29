cmake_minimum_required(VERSION 3.20)
project(KP)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(client client.cpp)
add_executable(server server.cpp)
target_link_libraries(client Threads::Threads)
target_link_libraries(server Threads::Threads)
