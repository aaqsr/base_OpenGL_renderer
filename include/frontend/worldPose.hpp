#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct WorldPose
{
    glm::vec3 position{0.0F};
    glm::quat rotation{1.0F, 0.0F, 0.0F, 0.0F}; // Identity quaternion
    glm::vec3 scale{1.0F};

    [[nodiscard]] glm::mat4 computeTransform() const
    {
        glm::mat4 t = glm::translate(glm::mat4(1.0F), position);
        glm::mat4 r = glm::mat4_cast(rotation);
        glm::mat4 s = glm::scale(glm::mat4(1.0F), scale);
        return t * r * s;
    }

    void translate(const glm::vec3& delta)
    {
        position += delta;
    }

    // pitch, yaw, roll applied in that order (pitch first, roll last)
    void rotateEuler(float pitch, float yaw, float roll)
    {
        rotation = glm::quat(glm::vec3(pitch, yaw, roll)) * rotation;
        rotation = glm::normalize(rotation);
    }

    void rotateAxis(float angle, const glm::vec3& axis)
    {
        rotation = glm::angleAxis(angle, axis) * rotation;
        rotation = glm::normalize(rotation);
    }
};
