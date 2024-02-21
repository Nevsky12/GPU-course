cmake_minimum_required(VERSION 3.11)
project(utils)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE .)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
