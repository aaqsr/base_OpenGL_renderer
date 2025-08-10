#pragma once

#include "util/perf.hpp"
#include <cstdint>
#include <functional>
#include <string>

struct GLFWwindow;

class Window
{
    GLFWwindow* window;

    IterationsPerSecondCounter framerateCounter;

    uint32_t width;
    uint32_t height;

    std::string title;

    void swapBuffers();
    void makeContextCurrent();

  public:
    Window(std::string title = "OpenGL", uint32_t initWidth = 800,
           uint32_t initHeight = 600);

    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    ~Window();

    GLFWwindow* getWindow();

    [[nodiscard]] bool shouldClose() const;

    // Could be an RAII object, but I'd rather be explicit to make it clearer
    void beginUpdate();
    void endUpdate();

    [[nodiscard]] float getWidthOverHeight() const;
};
