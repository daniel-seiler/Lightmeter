cmake_minimum_required(VERSION 3.0.0)
set(PRJ_NAME "lightmeter")
project(${PRJ_NAME} CXX)

add_executable(${PRJ_NAME} main.cpp)

target_link_arduino_libraries(${PRJ_NAME} PRIVATE core)
target_enable_arduino_upload(${PRJ_NAME})
