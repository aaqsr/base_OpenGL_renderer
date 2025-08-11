#include "frontend/camera.h"
#include "frontend/mesh.hpp"
#include "frontend/transform.hpp"
#include "frontend/window.hpp"
#include "util/error.hpp"
#include "util/logger.hpp"

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

void checkOpenGLError(const std::string& operation)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "OpenGL Error after " << operation << ": " << error
                  << '\n';
        switch (error) {
            case GL_INVALID_ENUM: std::cout << "GL_INVALID_ENUM" << '\n'; break;
            case GL_INVALID_VALUE:
                std::cout << "GL_INVALID_VALUE" << '\n';
                break;
            case GL_INVALID_OPERATION:
                std::cout << "GL_INVALID_OPERATION" << '\n';
                break;
            case GL_OUT_OF_MEMORY:
                std::cout << "GL_OUT_OF_MEMORY" << '\n';
                break;
            default: std::cout << "Unknown error code" << '\n'; break;
        }
    } else {
        std::cout << operation << ": OK" << '\n';
    }
}

void printOpenGLInfo()
{
    std::cout << "\n=== OpenGL Information ===" << '\n';
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << '\n';
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';
    std::cout << "Version: " << glGetString(GL_VERSION) << '\n';
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
              << '\n';

    int major = 0;
    int minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cout << "OpenGL Context Version: " << major << "." << minor << '\n';
    std::cout << "==========================\n" << '\n';
}

void runWindowCreate()
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

namespace
{

const char* vertexShaderSource = R"(
#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 410 core
in vec3 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

uint32_t compileShader(uint32_t type, const char* source)
{
    uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        throw IrrecoverableError{"Shader compilation failed: " +
                                 std::string(infoLog)};
    }

    return shader;
}

uint32_t createShaderProgram()
{
    uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    uint32_t fragmentShader =
      compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    uint32_t shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (success == 0) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        throw IrrecoverableError{"Shader program linking failed: " +
                                 std::string(infoLog)};
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

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

    uint32_t shaderProgram = createShaderProgram();

    Transform cubeTransform;
    Camera camera{
      .position = {0.0F, 0.0F, 3.0F},
        .target = {0.0F, 0.0F, 0.0F}
    };
    camera.aspectRatio = mainWin.getWidthOverHeight();

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(mainWin.getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 410 core");

    // UI state
    bool show_controls = true, show_demo = false, auto_rotate = true;
    float rotation_speed = 1.0f, fov = 75.0f, near_plane = 0.1f,
          far_plane = 100.0f;
    glm::vec3 rotation_axis{0.5f, 1.0f, 0.0f};
    glm::vec3 camera_position = camera.position;
    float manual_rotation_x = 0.0f, manual_rotation_y = 0.0f,
          manual_rotation_z = 0.0f;
    float timeValue = 0.0f;
    ImVec4 clear_color = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);

    while (!mainWin.shouldClose()) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (auto_rotate)
            timeValue += 0.01f * rotation_speed;

        mainWin.beginUpdate();
        glClearColor(clear_color.x, clear_color.y, clear_color.z,
                     clear_color.w);
        glUseProgram(shaderProgram);

        if (auto_rotate) {
            cubeTransform.rotation = glm::angleAxis(
              timeValue * rotation_speed, glm::normalize(rotation_axis));
        } else {
            cubeTransform.rotation = glm::quat(glm::vec3(
              glm::radians(manual_rotation_x), glm::radians(manual_rotation_y),
              glm::radians(manual_rotation_z)));
        }

        glm::mat4 model = cubeTransform.computeModelMatrix();

        camera.position = camera_position;
        camera.fov = fov;
        camera.nearPlane = near_plane;
        camera.farPlane = far_plane;
        camera.aspectRatio = mainWin.getWidthOverHeight();

        glm::mat4 view = camera.computeViewMatrix();
        glm::mat4 projection = camera.computeProjectionMatrix();

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1,
                           GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1,
                           GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1,
                           GL_FALSE, glm::value_ptr(projection));

        cubeMesh.draw();

        // GUI
        if (show_controls) {
            ImGui::Begin("Rainbow Cube Controls", &show_controls);

            ImGui::Text("Cube Renderer Controls");
            ImGui::Separator();

            // Rendering options
            ImGui::ColorEdit3("Clear Color", (float*)&clear_color);
            ImGui::Separator();

            // Rotation controls
            ImGui::Text("Rotation");
            ImGui::Checkbox("Auto Rotate", &auto_rotate);

            if (auto_rotate) {
                ImGui::SliderFloat("Rotation Speed", &rotation_speed, 0.0f,
                                   10.0f);
                ImGui::SliderFloat3("Rotation Axis", &rotation_axis.x, -1.0f,
                                    1.0f);
                ImGui::Text("Time: %.2f", timeValue);
                if (ImGui::Button("Reset Time")) {
                    timeValue = 0.0f;
                }
            } else {
                ImGui::Text("Manual Rotation (degrees)");
                ImGui::SliderFloat("X Rotation", &manual_rotation_x, 0.0f,
                                   360.0f);
                ImGui::SliderFloat("Y Rotation", &manual_rotation_y, 0.0f,
                                   360.0f);
                ImGui::SliderFloat("Z Rotation", &manual_rotation_z, 0.0f,
                                   360.0f);
                if (ImGui::Button("Reset Rotation")) {
                    manual_rotation_x = manual_rotation_y = manual_rotation_z =
                      0.0f;
                }
            }

            ImGui::Separator();
            ImGui::Text("Camera");

            ImGui::SliderFloat3("Camera Position", &camera_position.x, -10.0f,
                                10.0f);
            ImGui::SliderFloat("FOV", &fov, 1.0f, 120.0f);
            ImGui::SliderFloat("Near Plane", &near_plane, 0.01f, 10.0f);
            ImGui::SliderFloat("Far Plane", &far_plane, 10.0f, 200.0f);

            if (ImGui::Button("Reset Camera")) {
                camera_position = glm::vec3(0.0f, 0.0f, -3.0f);
                fov = 75.0f;
                near_plane = 0.1f;
                far_plane = 100.0f;
            }

            ImGui::Separator();

            // Performance info
            ImGui::Text("Performance");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / io.Framerate, io.Framerate);

            ImGui::Separator();
            ImGui::Checkbox("Show ImGui Demo", &show_demo);

            ImGui::End();
        }

        // showPerfMonitor();

        if (show_demo) {
            ImGui::ShowDemoWindow(&show_demo);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
        // runWindowCreate();
        runRainbowCube();
    } catch (...) {
        return 1;
    }
}
