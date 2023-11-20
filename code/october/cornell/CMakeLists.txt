cmake_minimum_required(VERSION 3.11)
project(cornell)

find_package(OpenEXR REQUIRED)

add_subdirectory(../utils utils)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE utils OpenEXR::IlmImf)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE thirdparty)
