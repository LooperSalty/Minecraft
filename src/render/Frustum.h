#pragma once
#include <glm/glm.hpp>

namespace voxelforge {

class Frustum {
public:
    void update(const glm::mat4& viewProjection);
    bool isBoxVisible(const glm::vec3& minPt, const glm::vec3& maxPt) const;
private:
    glm::vec4 m_planes[6];
};

} // namespace voxelforge
