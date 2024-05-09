#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

struct Window
{
    GLFWmonitor *monitor;
    GLFWvidmode const *mode;
    GLFWwindow *pointer;
    
    float xscale, yscale;

    Window();
    ~Window();

    bool shouldClose() const noexcept
    {
        glfwSwapBuffers(pointer);
        glfwPollEvents();
        glfwMakeContextCurrent(pointer);
        return glfwWindowShouldClose(pointer);
    }
};

inline Window::~Window()
{
    glfwDestroyWindow(pointer);
    glfwTerminate();
}
inline Window::Window()
{
    {
        glfwSetErrorCallback(+[](int error, const char* description)
        {
            std::cerr << "GLFW3 error #" << error << ": " << description << std::endl;
        });

        if(false == glfwInit())
            throw std::runtime_error("glfwInit");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    }

    monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);

    mode = glfwGetVideoMode(monitor);
    pointer = glfwCreateWindow(mode->width, mode->height, "The Window", nullptr, nullptr);
  //pointer = glfwCreateWindow(mode->width, mode->height, "The Window", monitor, nullptr); // fullscreen
    if(nullptr == pointer)
        throw std::runtime_error("glfwCreateWindow");

    glfwMakeContextCurrent(pointer);
    glfwSwapInterval(1); // vsync

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Failed to initialize GLAD");

    glfwSetFramebufferSizeCallback(pointer, +[](GLFWwindow *, int width, int height) noexcept
    {
        glViewport(0, 0, width, height);
    });
}
