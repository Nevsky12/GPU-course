cmake_minimum_required(VERSION 3.11)
project(threadpool)

add_executable(${PROJECT_NAME} main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_options(${PROJECT_NAME} PRIVATE -O3 -pedantic -pthread -Wall)
