#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform
{
    glm::vec3 position{0.0F};
    glm::quat rotation{1.0F, 0.0F, 0.0F, 0.0F}; // Identity quaternion
    glm::vec3 scale{1.0F};

    // Transform() = default;
    // Transform(const glm::vec3& pos) : position(pos)
    // {
    // }
    // Transform(const glm::vec3& pos, const glm::quat& rot)
    //   : position(pos), rotation(rot)
    // {
    // }
    // Transform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3&
    // scl)
    //   : position(pos), rotation(rot), scale(scl)
    // {
    // }

    [[nodiscard]] glm::mat4 computeModelMatrix() const
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

    // TODO: pitch, yaw, roll in WHAT ORDER??
    void rotateEuler(float pitch, float yaw, float roll)
    {
        rotation = glm::quat(glm::vec3(pitch, yaw, roll)) * rotation;
    }

    void rotateAxis(float angle, const glm::vec3& axis)
    {
        rotation = glm::angleAxis(angle, axis) * rotation;
    }
};
