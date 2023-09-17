cmake_minimum_required(VERSION 3.11)
project(utils)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE .)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME} INTERFACE -Werror -Wall -Wextra -Wconversion -pedantic -Wshadow -Wsign-conversion)
endif()
