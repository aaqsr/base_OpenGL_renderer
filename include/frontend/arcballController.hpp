#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct ArcballController
{
    glm::vec3 target{0.0F, 0.0F, 0.0F};
    float distance = 5.0F;
    float azimuth = 0.0F;   // Horizontal rotation around Y axis
    float elevation = 0.0F; // Vertical angle (0 = horizon, π/2 = top)

    // Constraints
    float minDistance = 0.5F;
    float maxDistance = 50.0F;
    float minElevation = -1.5F; // Slightly above horizon
    float maxElevation = 1.5F;  // Just below straight up

    // Input state
    bool wasMouseDown = false;
    glm::vec2 lastMousePos{0.0F};

    [[nodiscard]] glm::vec3 getPosition() const
    {
        float x = target.x + (distance * cos(elevation) * cos(azimuth));
        float y = target.y + (distance * sin(elevation));
        float z = target.z + (distance * cos(elevation) * sin(azimuth));
        return {x, y, z};
    }

    void zoom(float delta)
    {
        distance = glm::clamp(distance + delta, minDistance, maxDistance);
    }

    void rotate(float deltaAzimuth, float deltaElevation)
    {
        azimuth += deltaAzimuth;
        elevation =
          glm::clamp(elevation + deltaElevation, minElevation, maxElevation);
    }

    void setFromPositionAndTarget(const glm::vec3& position,
                                  const glm::vec3& targetPos)
    {
        target = targetPos;
        glm::vec3 offset = position - target;
        distance = glm::length(offset);

        if (distance > 0.001F) {
            glm::vec3 dir = glm::normalize(offset);
            azimuth = atan2(dir.z, dir.x);
            elevation = asin(glm::clamp(dir.y, -1.0F, 1.0F));
        }
    }
};

// call once per frame
inline void updateArcball(ArcballController& arcball, bool mouseDown,
                          glm::vec2 mousePos, float zoomDelta = 0.0F,
                          float sensitivity = 0.005F)
{
    glm::vec2 delta = mousePos - arcball.lastMousePos;
    // Handle mouse rotation
    if (mouseDown) {
        if (arcball.wasMouseDown) {
            arcball.rotate(delta.x * sensitivity, delta.y * sensitivity);
        }
        arcball.lastMousePos = mousePos;
    }
    arcball.wasMouseDown = mouseDown;

    // Handle zoom
    if (zoomDelta != 0.0F) {
        arcball.zoom(delta.y);
    }
}
