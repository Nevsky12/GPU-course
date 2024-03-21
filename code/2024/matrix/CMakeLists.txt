cmake_minimum_required(VERSION 3.11)
project(matrix)

find_package(TBB REQUIRED COMPONENTS tbb)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfma")
add_executable(${PROJECT_NAME} main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_options(${PROJECT_NAME} PRIVATE -O3 -g -pedantic -pthread -Wall)
target_link_libraries(${PROJECT_NAME} PRIVATE -ltbb)
