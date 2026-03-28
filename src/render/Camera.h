#pragma once
#include <glm/glm.hpp>

namespace voxelforge {

class Camera {
public:
    explicit Camera(const glm::vec3& position = glm::vec3(0.0f, 80.0f, 0.0f));

    void update(float dt, bool fwd, bool back, bool left, bool right,
                bool up, bool down, bool sprint, const glm::vec2& mouseDelta);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;
    const glm::vec3& getPosition() const { return m_position; }

private:
    void updateVectors();

    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp{0.0f, 1.0f, 0.0f};

    float m_yaw   = -90.0f;
    float m_pitch = 0.0f;
    float m_fov   = 70.0f;
    float m_sensitivity    = 0.1f;
    float m_speed          = 4.317f;  // blocs/s (Minecraft walk speed)
    float m_sprintMul      = 1.3f;
    float m_near           = 0.05f;
    float m_far            = 1000.0f;
};

} // namespace voxelforge
