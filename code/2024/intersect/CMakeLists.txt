cmake_minimum_required(VERSION 3.11)
project(intersect)

add_subdirectory(../utils utils)

#add_executable(${PROJECT_NAME} orig.cpp)
add_executable(${PROJECT_NAME} main2.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE utils)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_options(${PROJECT_NAME} PRIVATE -O3 -march=native)
