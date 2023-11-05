cmake_minimum_required(VERSION 3.11)
project(test)

#find_package(OpenEXR REQUIRED)
add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(../ gltf-parser/)

target_link_libraries(${PROJECT_NAME} PRIVATE gltf-parser)# OpenEXR::OpenEXR)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_link_options(${PROJECT_NAME} PRIVATE -static-libstdc++)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wextra -pedantic -Wshadow -Wconversion -Wsign-conversion)
endif()
