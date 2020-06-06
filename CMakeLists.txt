cmake_minimum_required(VERSION 3.14)
project(bluetooth_client_server C)

set(CMAKE_C_STANDARD 11)
add_executable(bluetooth_client_server server.c server.h)
target_link_libraries(bluetooth_client_server bluetooth)
target_link_libraries(bluetooth_client_server pthread)