cmake_minimum_required(VERSION 3.11)
project(gltf-parser)

include(FetchContent)
FetchContent_Declare(simdjson
    GIT_REPOSITORY https://github.com/simdjson/simdjson.git
    GIT_TAG v3.2.0
    GIT_SHALLOW TRUE
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/simdjson
)
FetchContent_MakeAvailable(simdjson)

set(HEADERS
    ./include/gltf/accessor.h
    ./include/gltf/asset.h
    ./include/gltf/binary_file.h
    ./include/gltf/buffer.h
    ./include/gltf/buffer_view.h
    ./include/gltf/image.h
    ./include/gltf/json.h
    ./include/gltf/material.h
    ./include/gltf/mesh.h
    ./include/gltf/node.h
    ./include/gltf/sampler.h
    ./include/gltf/scene.h
    ./include/gltf/texture.h
    ./include/gltf/as/binary_heap_bvh.h
    ./include/gltf/as/bottom_level.h
    ./include/gltf/as/top_level.h
    ./include/gltf/view/buffer.h
    ./include/gltf/view/geometry.h
    ./include/gltf/utils/aabb.h
    ./include/gltf/utils/camera.h
    ./include/gltf/utils/encode.h
    ./include/gltf/utils/offset_point.h
    ./include/gltf/utils/ray.h
    ./include/gltf/utils/triangle.h
)
set(SOURCES
    ./src/accessor.cpp
    ./src/asset.cpp
    ./src/binary_file.cpp
    ./src/buffer.cpp
    ./src/buffer_view.cpp
    ./src/image.cpp
    ./src/json.cpp
    ./src/material.cpp
    ./src/mesh.cpp
    ./src/node.cpp
    ./src/sampler.cpp
    ./src/scene.cpp
    ./src/texture.cpp
    ./src/utils/image.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${simdjson_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty)

target_link_libraries(${PROJECT_NAME} PRIVATE simdjson)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wextra -pedantic -Wshadow -Wconversion -Wsign-conversion)
endif()
