#pragma once 

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <string>

class WindowContext {
public:
    GLFWwindow* window = nullptr;

    WindowContext(int width, int height, const char* title);
    ~WindowContext();

    // Deshabilitar copia
    WindowContext(const WindowContext&) = delete;
    WindowContext& operator=(const WindowContext&) = delete;
};