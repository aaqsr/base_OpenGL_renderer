#pragma once

#include "util/logger.hpp"

#include <GL/glew.h>

#include <string>

inline void checkOpenGLError(const std::string& operation)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LOG("OpenGL Error after " << operation << ": " << error << '\n');

        switch (error) {
            case GL_INVALID_ENUM: LOG("GL_INVALID_ENUM"); break;
            case GL_INVALID_VALUE: LOG("GL_INVALID_VALUE"); break;
            case GL_INVALID_OPERATION: LOG("GL_INVALID_OPERATION"); break;
            case GL_OUT_OF_MEMORY: LOG("GL_OUT_OF_MEMORY"); break;
            default: LOG("Unknown error code"); break;
        }
    } else {
        LOG(operation << ": OK");
    }
}

inline void printOpenGLInfo()
{
    int major = 0;
    int minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    LOG("\n=== OpenGL Information ==="
        << '\n'
        << "Vendor: " << glGetString(GL_VENDOR) << '\n'
        << "Renderer: " << glGetString(GL_RENDERER) << '\n'
        << "Version: " << glGetString(GL_VERSION) << '\n'
        << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n'
        << "OpenGL Context Version: " << major << "." << minor << '\n'
        << "==========================\n"
        << '\n');
}
