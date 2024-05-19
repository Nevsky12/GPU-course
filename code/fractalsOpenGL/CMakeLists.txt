cmake_minimum_required(VERSION 3.0)
project(hello_rectangle)

add_library(glad glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PRIVATE imgui/)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(glfw3  REQUIRED)
target_link_libraries(imgui PUBLIC glfw glad)
target_compile_features(imgui PUBLIC cxx_std_23)

set(HEADERS
    gui.h
    shader.h
    window.h
)
set(SOURCES
    main.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_compile_definitions(${PROJECT_NAME} PRIVATE   FONT_DIR="${PROJECT_SOURCE_DIR}/imgui/misc/fonts/")
target_compile_definitions(${PROJECT_NAME} PRIVATE SHADER_DIR="${PROJECT_SOURCE_DIR}/shader/")

target_include_directories(${PROJECT_NAME} PRIVATE imgui/ ./)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui ${CMAKE_DL_LIBS})

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wextra -pedantic -Wshadow)
endif()

option(ASAN     "Use address sanitizer" OFF)
option(UBSAN    "Use UB sanitizer"      OFF)
option(THRSAN   "Use thread sanitizer"  OFF)

if(ASAN)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
    target_link_options   (${PROJECT_NAME} PRIVATE -fsanitize=address)
endif()
if(UBSAN)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=undefined)
    target_link_options   (${PROJECT_NAME} PRIVATE -fsanitize=undefined)
endif()
if(THRSAN)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=thread)
    target_link_options   (${PROJECT_NAME} PRIVATE -fsanitize=thread)
endif()
