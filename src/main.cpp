#include "frontend/GLFWContext.h"
#include "frontend/OpenGLDebug.hpp"
#include "frontend/window.hpp"

#include "frontend/UI.hpp"
#include "frontend/arcballController.hpp"
#include "frontend/camera.hpp"
#include "frontend/loadedObj.hpp"
#include "frontend/shader.hpp"
#include "frontend/worldPose.hpp"

#include <tiny_obj_loader.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <string>

namespace
{

void setInitialOpenGLRenderConfig()
{
    glEnable(GL_CULL_FACE);  // don't draw back faces
    glEnable(GL_DEPTH_TEST); // Depth buffer
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE); // allow writing to depth buffer
    glDepthRange(0.0, 1.0);
    glClearDepth(1.0);   // clear depth buffer to 1.0 (far plane)
    glfwSwapInterval(1); // enable VSync
}

void run()
{
    GLFWContext glfwContext;

    Window mainWin{"Loaded Object"};
    setInitialOpenGLRenderConfig();
    printOpenGLInfo();

    ImGUIContext imGuiContext{mainWin};

    LoadedObject mainModel =
      LoadedObject("assets/models/shaderBall/shaderBall.obj");
    mainModel.pose.scale = {0.01F, 0.01F, 0.01F};
    mainModel.pose.position = {0.0F, 0.0F, 0.0F};

    Shader mainShader{
      std::filesystem::path{"shaders/simpleDiffuseTexturedPhong/vert.glsl"},
      std::filesystem::path{"shaders/simpleDiffuseTexturedPhong/frag.glsl"}};

    Camera playerCamera{
      .position = {0.0F, 2.5F, 3.0F},
        .target = {0.0F, 0.0F, 0.0F}
    };
    playerCamera.aspectRatio = mainWin.getWidthOverHeight();

    ArcballController arcball;
    arcball.setFromPositionAndTarget(playerCamera.position,
                                     mainModel.pose.position +
                                       glm::vec3{0.0F, 1.0F, 0.0F});

    UIState uiState{playerCamera};

    {
        auto boundShader = mainShader.bind();
        mainModel.setInitUniforms(boundShader);
    }

    float lastFrameTime = glfwGetTime();

    while (!mainWin.shouldClose()) {
        glfwPollEvents();
        imGuiContext.startImGuiFrame();

        mainWin.beginUpdate();
        glClearColor(uiState.clearColour.x, uiState.clearColour.y,
                     uiState.clearColour.z, uiState.clearColour.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float deltaTime = glfwGetTime() - lastFrameTime;

        // Update model's rotation from UI
        // TODO: fix usage of deltaTime different from UI
        if (uiState.autoRotate) {
            float deltaAngle = deltaTime * uiState.rotationSpeed;
            glm::quat incrementalRotation =
              glm::angleAxis(deltaAngle, glm::normalize(uiState.rotationAxis));
            mainModel.pose.rotation =
              incrementalRotation * mainModel.pose.rotation;

            glm::vec3 eulerDegrees =
              glm::degrees(glm::eulerAngles(mainModel.pose.rotation));
            uiState.manualRotationX = eulerDegrees.x;
            uiState.manualRotationY = eulerDegrees.y;
            uiState.manualRotationZ = eulerDegrees.z;
        } else {
            mainModel.pose.rotation = glm::quat(glm::radians(
              glm::vec3(uiState.manualRotationX, uiState.manualRotationY,
                        uiState.manualRotationZ)));
        }

        // Update camera
        playerCamera.aspectRatio = mainWin.getWidthOverHeight();

        // Update arcball controller camera
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            double mouseX = 0.0;
            double mouseY = 0.0;
            glfwGetCursorPos(mainWin.getWindow(), &mouseX, &mouseY);
            bool mouseDown =
              glfwGetMouseButton(mainWin.getWindow(), GLFW_MOUSE_BUTTON_LEFT) ==
              GLFW_PRESS;
            updateArcball(arcball, mouseDown, glm::vec2(mouseX, mouseY));
            playerCamera.position = arcball.getPosition();
            playerCamera.target = arcball.target;
        }

        glm::mat4 model = mainModel.pose.computeTransform();
        glm::mat4 view = playerCamera.computeViewMatrix();
        glm::mat4 projection = playerCamera.computeProjectionMatrix();

        {
            auto boundShader = mainShader.bind();
            boundShader.setUniform("model", model);
            boundShader.setUniform("view", view);
            boundShader.setUniform("projection", projection);
            boundShader.setUniform("viewPos", playerCamera.position);

            // The draw call now handles binding textures and drawing the mesh
            mainModel.draw(boundShader);
        }

        drawImGuiAndUpdateState(uiState);
        mainWin.endUpdate();
        lastFrameTime = glfwGetTime();
    }
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
