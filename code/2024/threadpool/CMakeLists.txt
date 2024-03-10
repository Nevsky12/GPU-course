cmake_minimum_required(VERSION 3.11)
project(threadpool)

find_package(TBB REQUIRED COMPONENTS tbb)

add_executable(${PROJECT_NAME} meg.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_options(${PROJECT_NAME} PRIVATE -O3 -g -pedantic -pthread -Wall)
target_link_libraries(${PROJECT_NAME} PRIVATE -ltbb)
