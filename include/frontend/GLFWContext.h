#pragma once

#include "util/error.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class GLFWContext
{
  public:
    GLFWContext()
    {
        if (glfwInit() == 0) {
            throw IrrecoverableError{"Failed to initialize GLFW."};
        }
    }
    GLFWContext(const GLFWContext&) = delete;
    GLFWContext(GLFWContext&&) = delete;
    GLFWContext& operator=(const GLFWContext&) = delete;
    GLFWContext& operator=(GLFWContext&&) = delete;
    ~GLFWContext()
    {

        glfwTerminate();
    }
};
