cmake_minimum_required(VERSION 3.20)
project(6-8_lab/src)

add_definitions( -Wall -Werror -Wextra)
add_executable(control control_node.cpp)
add_executable(calculation_node calculation_node.cpp)
target_link_libraries(control zmq)
target_link_libraries(calculation_node zmq)