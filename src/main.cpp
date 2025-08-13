#include "frontend/OpenGLDebug.hpp"
#include "frontend/camera.hpp"
#include "frontend/mesh.hpp"
#include "frontend/shader.hpp"
#include "frontend/vertexLayout.hpp"
#include "frontend/window.hpp"
#include "frontend/worldPose.hpp"
#include "util/error.hpp"
#include "util/logger.hpp"

#include <filesystem>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
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

void showPerfMonitor()
{
    static float fps_history[100] = {};
    static float ms_history[100] = {};
    static int fps_offset = 0;

    float fps = ImGui::GetIO().Framerate;
    float ms = 1000.0f / (fps > 0.0f ? fps : 1.0f);

    // Update history
    fps_history[fps_offset] = fps;
    ms_history[fps_offset] = ms;
    fps_offset = (fps_offset + 1) % IM_ARRAYSIZE(fps_history);

    ImGui::Begin("Perf Monitor");
    ImGui::PlotLines("FPS Instant", fps_history, IM_ARRAYSIZE(fps_history),
                     fps_offset);
    ImGui::Text("FPS: %.1f", fps);
    ImGui::PlotLines("Frame Time (ms)", ms_history, IM_ARRAYSIZE(ms_history),
                     fps_offset);
    ImGui::Text("Frame Time: %.2f ms", ms);
    ImGui::End();
}

struct UIState
{
    ImVec4 clearColour = ImVec4(0.2F, 0.3F, 0.3F, 1.0F);

    glm::vec3 rotationAxis{0.5F, 1.0F, 0.0F};
    float rotationSpeed = 1.0F;

    glm::vec3 cameraPosition;

    float fov;
    float nearPlane;
    float farPlane;

    float manualRotationX = 0.0F;
    float manualRotationY = 0.0F;
    float manualRotationZ = 0.0F;

    float timeValue = 0.0F;

    bool showControls = true;
    bool showDemo = false;
    bool autoRotate = true;

    UIState(const Camera& cam)
      : cameraPosition{cam.position}, fov{cam.fov}, nearPlane{cam.nearPlane},
        farPlane{cam.farPlane}
    {
    }
};

void initImGui(Window& mainWin)
{
    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(mainWin.getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 410 core");
}

void startImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void drawImGuiAndUpdateState(UIState& state)
{
    // GUI
    if (state.showControls) {
        ImGui::Begin("Rainbow Cube Controls", &state.showControls);

        ImGui::Text("Cube Renderer Controls");
        ImGui::Separator();

        // Rendering options
        ImGui::ColorEdit3("Clear Color", (float*)&state.clearColour);
        ImGui::Separator();

        // Rotation controls
        ImGui::Text("Rotation");
        ImGui::Checkbox("Auto Rotate", &state.autoRotate);

        if (state.autoRotate) {
            ImGui::SliderFloat("Rotation Speed", &state.rotationSpeed, 0.0f,
                               10.0f);
            ImGui::SliderFloat3("Rotation Axis", &state.rotationAxis.x, -1.0f,
                                1.0f);
            ImGui::Text("Time: %.2f", state.timeValue);
            if (ImGui::Button("Reset Time")) {
                state.timeValue = 0.0f;
            }
        } else {
            ImGui::Text("Manual Rotation (degrees)");
            ImGui::SliderFloat("X Rotation", &state.manualRotationX, -180.0f,
                               180.0f);
            ImGui::SliderFloat("Y Rotation", &state.manualRotationY, -180.0f,
                               180.0f);
            ImGui::SliderFloat("Z Rotation", &state.manualRotationZ, -180.0f,
                               180.0f);
            if (ImGui::Button("Reset Rotation")) {
                state.manualRotationX = state.manualRotationY =
                  state.manualRotationZ = 0.0f;
            }
        }

        ImGui::Separator();
        ImGui::Text("Camera");

        ImGui::SliderFloat3("Camera Position", &state.cameraPosition.x, -10.0f,
                            10.0f);
        ImGui::SliderFloat("FOV", &state.fov, 1.0f, 120.0f);
        ImGui::SliderFloat("Near Plane", &state.nearPlane, 0.01f, 10.0f);
        ImGui::SliderFloat("Far Plane", &state.farPlane, 10.0f, 200.0f);

        if (ImGui::Button("Reset Camera")) {
            state.cameraPosition = glm::vec3(0.0f, 0.0f, -3.0f);
            state.fov = 75.0f;
            state.nearPlane = 0.1f;
            state.farPlane = 100.0f;
        }

        ImGui::Separator();

        ImGuiIO& io = ImGui::GetIO();

        // Performance info
        ImGui::Text("Performance");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / io.Framerate, io.Framerate);

        ImGui::Separator();
        ImGui::Checkbox("Show ImGui Demo", &state.showDemo);

        ImGui::End();
    }

    // showPerfMonitor();

    if (state.showDemo) {
        ImGui::ShowDemoWindow(&state.showDemo);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void runRainbowCube()
{
    if (glfwInit() == 0) {
        throw IrrecoverableError{"Failed to initialize GLFW."};
    }

    Window mainWin{"Rainbow Cube"};

    setInitialOpenGLRenderConfig();

    printOpenGLInfo();

    struct ColourVertex
    {
        glm::vec3 pos;
        glm::vec3 col;
    } __attribute__((packed));

    // Cube vertices with positions and colors (rainbow faces)
    std::vector<ColourVertex> vertices = {
      // Front face (Red)
      { .pos = {-0.5F, -0.5F, 0.5F}, .col = {1.0F, 0.0F, 0.0F}}, // bottom-left
      {  .pos = {0.5F, -0.5F, 0.5F}, .col = {1.0F, 0.0F, 0.0F}}, // bottom-right
      {   .pos = {0.5F, 0.5F, 0.5F}, .col = {1.0F, 0.0F, 0.0F}}, // top-right
      {  .pos = {-0.5F, 0.5F, 0.5F}, .col = {1.0F, 0.0F, 0.0F}}, // top-left

      // Back face (Green)
      {.pos = {-0.5F, -0.5F, -0.5F}, .col = {0.0F, 1.0F, 0.0F}}, // bottom-left
      { .pos = {0.5F, -0.5F, -0.5F}, .col = {0.0F, 1.0F, 0.0F}}, // bottom-right
      { .pos = {-0.5F, 0.5F, -0.5F}, .col = {0.0F, 1.0F, 0.0F}}, // top-left
      {  .pos = {0.5F, 0.5F, -0.5F}, .col = {0.0F, 1.0F, 0.0F}}, // top-right

      // Left face (Blue)
      {.pos = {-0.5F, -0.5F, -0.5F}, .col = {0.0F, 0.0F, 1.0F}}, // bottom-left
      { .pos = {-0.5F, -0.5F, 0.5F}, .col = {0.0F, 0.0F, 1.0F}}, // bottom-right
      {  .pos = {-0.5F, 0.5F, 0.5F}, .col = {0.0F, 0.0F, 1.0F}}, // top-right
      { .pos = {-0.5F, 0.5F, -0.5F}, .col = {0.0F, 0.0F, 1.0F}}, // top-left

      // Right face (Yellow)
      {  .pos = {0.5F, -0.5F, 0.5F}, .col = {1.0F, 1.0F, 0.0F}}, // bottom-left
      { .pos = {0.5F, -0.5F, -0.5F}, .col = {1.0F, 1.0F, 0.0F}}, // bottom-right
      {  .pos = {0.5F, 0.5F, -0.5F}, .col = {1.0F, 1.0F, 0.0F}}, // top-right
      {   .pos = {0.5F, 0.5F, 0.5F}, .col = {1.0F, 1.0F, 0.0F}}, // top-left

      // Top face (Magenta)
      {  .pos = {-0.5F, 0.5F, 0.5F}, .col = {1.0F, 0.0F, 1.0F}}, // bottom-left
      {   .pos = {0.5F, 0.5F, 0.5F}, .col = {1.0F, 0.0F, 1.0F}}, // bottom-right
      {  .pos = {0.5F, 0.5F, -0.5F}, .col = {1.0F, 0.0F, 1.0F}}, // top-right
      { .pos = {-0.5F, 0.5F, -0.5F}, .col = {1.0F, 0.0F, 1.0F}}, // top-left

      // Bottom face (Cyan)
      {.pos = {-0.5F, -0.5F, -0.5F}, .col = {0.0F, 1.0F, 1.0F}}, // bottom-left
      { .pos = {0.5F, -0.5F, -0.5F}, .col = {0.0F, 1.0F, 1.0F}}, // bottom-right
      {  .pos = {0.5F, -0.5F, 0.5F}, .col = {0.0F, 1.0F, 1.0F}}, // top-right
      { .pos = {-0.5F, -0.5F, 0.5F}, .col = {0.0F, 1.0F, 1.0F}}  // top-left
    };

    std::vector<uint32_t> indices = {// Front face
                                     0, 1, 2, 2, 3, 0,
                                     // Back face
                                     5, 4, 6, 6, 7, 5,
                                     // Left face
                                     8, 9, 10, 10, 11, 8,
                                     // Right face
                                     12, 13, 14, 14, 15, 12,
                                     // Top face
                                     16, 17, 18, 18, 19, 16,
                                     // Bottom face
                                     20, 21, 22, 22, 23, 20};

    VertexLayout colourVertexLayout =
      VertexLayout{}.addAttribute(0, 3, GL_FLOAT).addAttribute(1, 3, GL_FLOAT);

    Mesh cubeMesh{vertices, colourVertexLayout, indices};

    Shader theShader{std::filesystem::path{"shaders/flatColour/vert.glsl"},
                     std::filesystem::path{"shaders/flatColour/frag.glsl"}};

    WorldPose cubeTransform;
    Camera camera{
      .position = {0.0F, 0.0F, 3.0F},
        .target = {0.0F, 0.0F, 0.0F}
    };
    camera.aspectRatio = mainWin.getWidthOverHeight();

    initImGui(mainWin);

    UIState uiState{camera};

    while (!mainWin.shouldClose()) {
        glfwPollEvents();

        startImGuiFrame();

        mainWin.beginUpdate();
        glClearColor(uiState.clearColour.x, uiState.clearColour.y,
                     uiState.clearColour.z, uiState.clearColour.w);

        if (uiState.autoRotate) {
            float deltaAngle = 0.01F * uiState.rotationSpeed;
            glm::quat incrementalRotation =
              glm::angleAxis(deltaAngle, glm::normalize(uiState.rotationAxis));

            cubeTransform.rotation =
              incrementalRotation * cubeTransform.rotation;

            glm::vec3 eulerDegrees =
              glm::degrees(glm::eulerAngles(cubeTransform.rotation));
            uiState.manualRotationX = eulerDegrees.x;
            uiState.manualRotationY = eulerDegrees.y;
            uiState.manualRotationZ = eulerDegrees.z;
        } else {
            cubeTransform.rotation = glm::quat(glm::radians(
              glm::vec3(uiState.manualRotationX, uiState.manualRotationY,
                        uiState.manualRotationZ)));
        }

        glm::mat4 model = cubeTransform.computeTransform();

        camera.position = uiState.cameraPosition;
        camera.fov = uiState.fov;
        camera.nearPlane = uiState.nearPlane;
        camera.farPlane = uiState.farPlane;
        camera.aspectRatio = mainWin.getWidthOverHeight();

        glm::mat4 view = camera.computeViewMatrix();
        glm::mat4 projection = camera.computeProjectionMatrix();

        {
            auto boundShader = theShader.bind();
            boundShader.setUniform("model", model);
            boundShader.setUniform("view", view);
            boundShader.setUniform("projection", projection);

            cubeMesh.draw(boundShader);
        }

        drawImGuiAndUpdateState(uiState);

        mainWin.endUpdate();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

} // namespace

int main()
{
    try {
        runRainbowCube();
    } catch (...) {
        return 1;
    }
}
