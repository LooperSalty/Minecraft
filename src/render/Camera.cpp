#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace voxelforge {

Camera::Camera(const glm::vec3& position)
    : m_position(position)
    , m_front(0.0f, 0.0f, -1.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_right(1.0f, 0.0f, 0.0f)
{
    updateVectors();
}

void Camera::update(float dt, bool fwd, bool back, bool left, bool right,
                    bool up, bool down, bool sprint, const glm::vec2& mouseDelta)
{
    // Mouse look
    m_yaw   += mouseDelta.x * m_sensitivity;
    m_pitch += mouseDelta.y * m_sensitivity;
    m_pitch  = std::clamp(m_pitch, -89.9f, 89.9f);
    updateVectors();

    // Movement on XZ plane
    float spd = m_speed * (sprint ? m_sprintMul : 1.0f) * dt;
    glm::vec3 flatFront = glm::normalize(glm::vec3(m_front.x, 0.0f, m_front.z));
    glm::vec3 flatRight = glm::normalize(glm::cross(flatFront, m_worldUp));

    if (fwd)   m_position += flatFront * spd;
    if (back)  m_position -= flatFront * spd;
    if (left)  m_position -= flatRight * spd;
    if (right) m_position += flatRight * spd;
    if (up)    m_position.y += spd;
    if (down)  m_position.y -= spd;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(m_fov), aspect, m_near, m_far);
}

void Camera::updateVectors() {
    float yawRad   = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);
    m_front = glm::normalize(glm::vec3(
        std::cos(yawRad) * std::cos(pitchRad),
        std::sin(pitchRad),
        std::sin(yawRad) * std::cos(pitchRad)
    ));
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up    = glm::normalize(glm::cross(m_right, m_front));
}

} // namespace voxelforge
