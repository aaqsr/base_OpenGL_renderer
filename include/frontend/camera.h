#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera
{
    glm::vec3 position{0.0F, 0.0F, 3.0F};
    glm::vec3 target{0.0F, 0.0F, 0.0F};
    glm::vec3 up{0.0F, 1.0F, 0.0F};

    float fov = 75.0F;
    float aspectRatio = 16.0F / 9.0F;
    float nearPlane = 0.1F;
    float farPlane = 100.0F;

    // Camera() = default;
    // Camera(const glm::vec3& pos, const glm::vec3& tgt)
    //   : position(pos), target(tgt)
    // {
    // }

    [[nodiscard]] glm::mat4 computeViewMatrix() const
    {
        return glm::lookAt(position, target, up);
    }

    [[nodiscard]] glm::mat4 computeProjectionMatrix() const
    {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane,
                                farPlane);
    }

    [[nodiscard]] glm::vec3 computeForward() const
    {
        return glm::normalize(target - position);
    }

    [[nodiscard]] glm::vec3 computeRight() const
    {
        return glm::normalize(glm::cross(computeForward(), up));
    }
};

// // Convenience for shader uniforms - templated to work with any shader type
// template <typename ShaderType>
// inline void setCameraUniforms(const Camera& camera, ShaderType& shader)
// {
//     shader.setUniform("view", getViewMatrix(camera));
//     shader.setUniform("projection", computeProjectionMatrix(camera));
// }
