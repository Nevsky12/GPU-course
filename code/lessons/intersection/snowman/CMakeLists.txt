cmake_minimum_required(VERSION 3.11)
project(firstblood)

add_subdirectory(../../../utils utils)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE utils)
