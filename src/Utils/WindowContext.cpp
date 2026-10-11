#include "WindowContext.h"
#include <iostream>
#include <cstdlib>

// Implementación de constructor y destructor
WindowContext::WindowContext(int width, int height, const char *title)
{
    glfwSetErrorCallback([](int error, const char *description)
                         { std::cerr << "GLFW Error " << error << ": " << description << std::endl; });

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
}

WindowContext::~WindowContext()
{
    if (window)
    {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}