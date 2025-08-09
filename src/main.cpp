#include "frontend/window.hpp"
#include "util/error.hpp"
#include "util/logger.hpp"

#include <GLFW/glfw3.h>

namespace
{

void run()
{
    if (glfwInit() == 0) {
        throw IrrecoverableError{"Failed to initialize GLFW."};
    }

    Window mainWin;

    while (!mainWin.shouldClose()) {
        glfwPollEvents();

        mainWin.beginUpdate();

        mainWin.endUpdate();
    }

    glfwTerminate();
}

} // namespace

int main()
{
    try {
        run();
    } catch (...) {
        return 1;
    }
}
