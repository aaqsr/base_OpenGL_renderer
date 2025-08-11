#include "frontend/window.hpp"
#include "util/error.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

Window::Window(std::string _title, uint32_t initWidth, uint32_t initHeight)
  : framerateCounter{"Window '" + _title + "'", "FPS", "frame"},
    width{initWidth}, height{initHeight}, title{std::move(_title)}
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height),
                              title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        throw IrrecoverableError{"Failed to create GLFW window"};
    }

    glfwMakeContextCurrent(window);

    glewExperimental = 1;
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw IrrecoverableError{"Failed to initialize GLEW."};
    }

    glfwSetWindowUserPointer(window, this);

    // Callback when window resizes
    glfwSetFramebufferSizeCallback(
      window, [](GLFWwindow* pWindow, int newWidth, int newHeight) {
          auto* win = static_cast<Window*>(glfwGetWindowUserPointer(pWindow));

          win->width = newWidth;
          win->height = newHeight;

          glViewport(0, 0, newWidth, newHeight);
      });

    // Use framebuffer size, not window size for viewport.
    // Why? Because they might differ... (example: on HiDPI or Retina
    // displays, framebuffer is typically 2x the window size)
    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    glClearColor(0.2F, 0.3F, 0.3F, 1.0F); // Set a colour background
}

Window::Window(Window&& other) noexcept
  : window(other.window), framerateCounter(std::move(other.framerateCounter)),
    width(other.width), height(other.height), title(std::move(other.title))
{
    other.window = nullptr;

    if (window != nullptr) {
        // re-bind to this which has moved
        glfwSetWindowUserPointer(window, this);
    }
}

Window& Window::operator=(Window&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    if (window != nullptr) {
        glfwDestroyWindow(window);
    }

    window = other.window;
    title = std::move(other.title);
    width = other.width;
    height = other.height;
    framerateCounter = std::move(other.framerateCounter);

    other.window = nullptr;

    if (window != nullptr) {
        // re-bind to this which has moved
        glfwSetWindowUserPointer(window, this);
    }

    return *this;
}

Window::~Window()
{
    if (window != nullptr) {
        glfwDestroyWindow(window);
    }
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(window) == GL_TRUE;
}

void Window::swapBuffers()
{
    glfwSwapBuffers(window);
}

void Window::beginUpdate()
{
    // TODO: this kills performance. and it's only useful if more than 1 window.
    // If ever >1 window: needs to be reconsidered.
    // makeContextCurrent();

    // TODO: not needed for now since ImGUI can show it. Can we refactor
    // framerateCounter to be better and not spam console every second with a
    // new line?
    // framerateCounter.tick();

    // Not clearing the back buffer causes trails
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::endUpdate()
{
    // Double buffering.
    // The front buffer contains the final output image that is
    // shown at the screen. Whilst all the rendering commands
    // draw to the back buffer. We swap the back buffer to the
    // front buffer so the image can be displayed without still
    // being rendered to.
    swapBuffers();
}

GLFWwindow* Window::getWindow()
{
    return window;
}

void Window::makeContextCurrent()
{
    glfwMakeContextCurrent(window);
}

float Window::getWidthOverHeight() const
{
    return static_cast<float>(width) / static_cast<float>(height);
}
