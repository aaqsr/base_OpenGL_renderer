#pragma once

#include "frontend/camera.hpp"
#include "frontend/window.hpp"

#include <glm/glm.hpp>

#include <GL/glew.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class ImGUIContext
{
  public:
    ImGUIContext(Window& win)
    {
        // Setup ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsLight();
        ImGui_ImplGlfw_InitForOpenGL(win.getWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 410 core");
    }
    ImGUIContext(const ImGUIContext&) = delete;
    ImGUIContext(ImGUIContext&&) = delete;
    ImGUIContext& operator=(const ImGUIContext&) = delete;
    ImGUIContext& operator=(ImGUIContext&&) = delete;
    ~ImGUIContext()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void startImGuiFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
};


struct UIState
{

    //
    // Application
    //
    ImVec4 clearColour = ImVec4(0.2F, 0.3F, 0.3F, 1.0F);

    float timeValue = 0.0F;
    bool showControls = true;

    bool showDemoImGuiWindow = false;

    //
    // Object
    //
    glm::vec3 rotationAxis{0.5F, 1.0F, 0.0F};
    float rotationSpeed = 1.0F;
    bool autoRotate = false;

    float manualRotationX = 0.0F;
    float manualRotationY = 0.0F;
    float manualRotationZ = 0.0F;

    //
    // Camera
    //
    glm::vec3* cameraPosition;
    glm::vec3 initCameraPosition;

    glm::vec3* cameraLookAt;
    glm::vec3 initCameraLookAt;

    float* fov;
    float* nearPlane;
    float* farPlane;

    UIState(Camera& cam)
      : cameraPosition{&cam.position}, initCameraPosition{cam.position},
        cameraLookAt{&cam.target}, initCameraLookAt{cam.target}, fov{&cam.fov},
        nearPlane{&cam.nearPlane}, farPlane{&cam.farPlane}
    {
    }
};

inline void drawImGuiAndUpdateState(UIState& state)
{
    // GUI
    if (state.showControls) {
        ImGui::Begin("Controls", &state.showControls);

        ImGui::Text("Performance");

        ImGuiIO& io = ImGui::GetIO();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0F / io.Framerate, io.Framerate);


        ImGui::Separator();
        if (ImGui::CollapsingHeader("Renderer")) {
            ImGui::ColorEdit3("Clear Color", &state.clearColour.x);
        }


        ImGui::Separator();
        if (ImGui::CollapsingHeader("Main Object")) {

            // Rotation controls
            ImGui::Text("Rotation");
            ImGui::Checkbox("Auto Rotate", &state.autoRotate);

            if (state.autoRotate) {
                ImGui::SliderFloat("Rotation Speed", &state.rotationSpeed, 0.0F,
                                   10.0F);
                ImGui::SliderFloat3("Rotation Axis", &state.rotationAxis.x,
                                    -1.0F, 1.0F);
                ImGui::Text("Time: %.2f", state.timeValue);
                if (ImGui::Button("Reset Time")) {
                    state.timeValue = 0.0F;
                }
            } else {
                ImGui::Text("Manual Rotation (degrees)");
                ImGui::SliderFloat("X Rotation", &state.manualRotationX,
                                   -180.0F, 180.0F);
                ImGui::SliderFloat("Y Rotation", &state.manualRotationY,
                                   -180.0F, 180.0F);
                ImGui::SliderFloat("Z Rotation", &state.manualRotationZ,
                                   -180.0F, 180.0F);
                if (ImGui::Button("Reset Rotation")) {
                    state.manualRotationX = state.manualRotationY =
                      state.manualRotationZ = 0.0f;
                }
            }
        }


        ImGui::Separator();
        if (ImGui::CollapsingHeader("Camera")) {

            ImGui::DragFloat3("Camera Position", &(state.cameraPosition->x),
                              0.1F);
            ImGui::DragFloat3("Camera Look-At", &(state.cameraLookAt->x), 0.1F);
            ImGui::DragFloat("Near Plane", state.nearPlane, 0.1F);
            ImGui::DragFloat("Far Plane", state.farPlane, 0.1F);
            ImGui::SliderFloat("FOV", state.fov, 1.0F, 120.0F);

            if (ImGui::Button("Reset Camera")) {
                *state.cameraPosition = state.initCameraPosition;
                *state.fov = 75.0F;
                *state.nearPlane = 0.1F;
                *state.farPlane = 100.0F;
            }
        }


        ImGui::Separator();
        ImGui::Checkbox("Show ImGui Demo", &state.showDemoImGuiWindow);


        ImGui::End();
    }

    if (state.showDemoImGuiWindow) {
        ImGui::ShowDemoWindow(&state.showDemoImGuiWindow);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

inline void showPerfMonitor()
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
