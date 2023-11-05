cmake_minimum_required(VERSION 3.11)
project(zero_bvh)

add_subdirectory(../utils utils)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE utils)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_definitions(${PROJECT_NAME} PRIVATE RES_DIR="${PROJECT_SOURCE_DIR}/../res/")
