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
#version 330 core
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
#version 330 core
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

void runRainbowCube()
{
    if (glfwInit() == 0) {
        throw IrrecoverableError{"Failed to initialize GLFW."};
    }

    Window mainWin{"Rainbow Cube"};

    //
    // OpenGL configuration
    //
    glEnable(GL_CULL_FACE);  // don't draw back faces
    glEnable(GL_DEPTH_TEST); // Depth buffer
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE); // allow writing to depth buffer
    glDepthRange(0.0, 1.0);
    glClearDepth(1.0);   // clear depth buffer to 1.0 (far plane)
    glfwSwapInterval(1); // enable VSync

    printOpenGLInfo();

    // Cube vertices with positions and colors (rainbow faces)
    float vertices[] = {
      // Front face (Red)
      -0.5F, -0.5F, 0.5F, 1.0F, 0.0F, 0.0F, // bottom-left
      0.5F, -0.5F, 0.5F, 1.0F, 0.0F, 0.0F,  // bottom-right
      0.5F, 0.5F, 0.5F, 1.0F, 0.0F, 0.0F,   // top-right
      -0.5F, 0.5F, 0.5F, 1.0F, 0.0F, 0.0F,  // top-left

      // Back face (Green)
      -0.5F, -0.5F, -0.5F, 0.0F, 1.0F, 0.0F, // bottom-left
      0.5F, -0.5F, -0.5F, 0.0F, 1.0F, 0.0F,  // bottom-right
      -0.5F, 0.5F, -0.5F, 0.0F, 1.0F, 0.0F,  // top-left
      0.5F, 0.5F, -0.5F, 0.0F, 1.0F, 0.0F,   // top-right

      // Left face (Blue)
      -0.5F, -0.5F, -0.5F, 0.0F, 0.0F, 1.0F, // bottom-left
      -0.5F, -0.5F, 0.5F, 0.0F, 0.0F, 1.0F,  // bottom-right
      -0.5F, 0.5F, 0.5F, 0.0F, 0.0F, 1.0F,   // top-right
      -0.5F, 0.5F, -0.5F, 0.0F, 0.0F, 1.0F,  // top-left

      // Right face (Yellow)
      0.5F, -0.5F, 0.5F, 1.0F, 1.0F, 0.0F,  // bottom-left
      0.5F, -0.5F, -0.5F, 1.0F, 1.0F, 0.0F, // bottom-right
      0.5F, 0.5F, -0.5F, 1.0F, 1.0F, 0.0F,  // top-right
      0.5F, 0.5F, 0.5F, 1.0F, 1.0F, 0.0F,   // top-left

      // Top face (Magenta)
      -0.5F, 0.5F, 0.5F, 1.0F, 0.0F, 1.0F,  // bottom-left
      0.5F, 0.5F, 0.5F, 1.0F, 0.0F, 1.0F,   // bottom-right
      0.5F, 0.5F, -0.5F, 1.0F, 0.0F, 1.0F,  // top-right
      -0.5F, 0.5F, -0.5F, 1.0F, 0.0F, 1.0F, // top-left

      // Bottom face (Cyan)
      -0.5F, -0.5F, -0.5F, 0.0F, 1.0F, 1.0F, // bottom-left
      0.5F, -0.5F, -0.5F, 0.0F, 1.0F, 1.0F,  // bottom-right
      0.5F, -0.5F, 0.5F, 0.0F, 1.0F, 1.0F,   // top-right
      -0.5F, -0.5F, 0.5F, 0.0F, 1.0F, 1.0F   // top-left
    };

    uint32_t indices[] = {// Front face
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

    // Create and bind VAO, VBO, EBO
    uint32_t VAO = 0;
    uint32_t VBO = 0;
    uint32_t EBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    // Colour attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    uint32_t shaderProgram = createShaderProgram();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(mainWin.getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // ImGui variables
    bool show_controls = true;
    bool show_demo = false;
    bool auto_rotate = true;
    float rotation_speed = 1.0f;
    glm::vec3 rotation_axis(0.5f, 1.0f, 0.0f);
    glm::vec3 camera_position(0.0f, 0.0f, -3.0f);
    float fov = 45.0f;
    float near_plane = 0.1f;
    float far_plane = 100.0f;
    ImVec4 clear_color = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);
    static float manual_rotation_x = 0.0f;
    static float manual_rotation_y = 0.0f;
    static float manual_rotation_z = 0.0f;
    static float timeValue = 0.0f;

    while (!mainWin.shouldClose()) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Update time for auto rotation
        if (auto_rotate) {
            timeValue += 0.01f * rotation_speed;
        }

        mainWin.beginUpdate();

        // Set clear color from ImGui
        glClearColor(clear_color.x, clear_color.y, clear_color.z,
                     clear_color.w);

        // Use our shader program
        glUseProgram(shaderProgram);

        // Create transformation matrices
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        // Apply rotations
        if (auto_rotate) {
            model = glm::rotate(model, timeValue, rotation_axis);
        } else {
            model = glm::rotate(model, glm::radians(manual_rotation_x),
                                glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(manual_rotation_y),
                                glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(manual_rotation_z),
                                glm::vec3(0.0f, 0.0f, 1.0f));
        }

        // Set camera position
        view = glm::translate(view, camera_position);

        // Create perspective projection
        projection = glm::perspective(glm::radians(fov), mainWin.getWidthOverHeight(),
                                      near_plane, far_plane);

        // Pass matrices to shader
        uint32_t modelLoc = glGetUniformLocation(shaderProgram, "model");
        uint32_t viewLoc = glGetUniformLocation(shaderProgram, "view");
        uint32_t projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Draw the cube
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // === ImGui UI ===

        // Main controls window
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
                ImGui::SliderFloat("Speed", &rotation_speed, 0.0f, 5.0f);
                ImGui::SliderFloat3("Rotation Axis",
                                    glm::value_ptr(rotation_axis), -1.0f, 1.0f);
                ImGui::Text("Time: %.2f", timeValue);
                if (ImGui::Button("Reset Time")) {
                    timeValue = 0.0f;
                }
            } else {
                ImGui::Text("Manual Rotation (degrees)");
                ImGui::SliderFloat("X Rotation", &manual_rotation_x, -180.0f,
                                   180.0f);
                ImGui::SliderFloat("Y Rotation", &manual_rotation_y, -180.0f,
                                   180.0f);
                ImGui::SliderFloat("Z Rotation", &manual_rotation_z, -180.0f,
                                   180.0f);
                if (ImGui::Button("Reset Rotation")) {
                    manual_rotation_x = manual_rotation_y = manual_rotation_z =
                      0.0f;
                }
            }

            ImGui::Separator();

            // Camera controls
            ImGui::Text("Camera");
            ImGui::SliderFloat3("Position", glm::value_ptr(camera_position),
                                -10.0f, 10.0f);
            ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f);
            ImGui::SliderFloat("Near Plane", &near_plane, 0.01f, 1.0f);
            ImGui::SliderFloat("Far Plane", &far_plane, 10.0f, 1000.0f);

            if (ImGui::Button("Reset Camera")) {
                camera_position = glm::vec3(0.0f, 0.0f, -3.0f);
                fov = 45.0f;
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

        // Show ImGui demo window if requested
        if (show_demo) {
            ImGui::ShowDemoWindow(&show_demo);
        }

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        mainWin.endUpdate();
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

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
