#include "Frustum.h"
#include <cmath>

namespace voxelforge {

void Frustum::update(const glm::mat4& vp) {
    // Extract frustum planes using row extraction method
    // Left:   row3 + row0
    m_planes[0] = glm::vec4(
        vp[0][3] + vp[0][0],
        vp[1][3] + vp[1][0],
        vp[2][3] + vp[2][0],
        vp[3][3] + vp[3][0]);
    // Right:  row3 - row0
    m_planes[1] = glm::vec4(
        vp[0][3] - vp[0][0],
        vp[1][3] - vp[1][0],
        vp[2][3] - vp[2][0],
        vp[3][3] - vp[3][0]);
    // Bottom: row3 + row1
    m_planes[2] = glm::vec4(
        vp[0][3] + vp[0][1],
        vp[1][3] + vp[1][1],
        vp[2][3] + vp[2][1],
        vp[3][3] + vp[3][1]);
    // Top:    row3 - row1
    m_planes[3] = glm::vec4(
        vp[0][3] - vp[0][1],
        vp[1][3] - vp[1][1],
        vp[2][3] - vp[2][1],
        vp[3][3] - vp[3][1]);
    // Near:   row3 + row2
    m_planes[4] = glm::vec4(
        vp[0][3] + vp[0][2],
        vp[1][3] + vp[1][2],
        vp[2][3] + vp[2][2],
        vp[3][3] + vp[3][2]);
    // Far:    row3 - row2
    m_planes[5] = glm::vec4(
        vp[0][3] - vp[0][2],
        vp[1][3] - vp[1][2],
        vp[2][3] - vp[2][2],
        vp[3][3] - vp[3][2]);

    // Normalize each plane
    for (int i = 0; i < 6; ++i) {
        float len = std::sqrt(m_planes[i].x * m_planes[i].x +
                              m_planes[i].y * m_planes[i].y +
                              m_planes[i].z * m_planes[i].z);
        if (len > 0.0f) {
            m_planes[i] /= len;
        }
    }
}

bool Frustum::isBoxVisible(const glm::vec3& minPt, const glm::vec3& maxPt) const {
    for (int i = 0; i < 6; ++i) {
        const auto& plane = m_planes[i];

        // Find the p-vertex: the point most in the direction of the plane normal
        glm::vec3 pVertex(
            (plane.x >= 0.0f) ? maxPt.x : minPt.x,
            (plane.y >= 0.0f) ? maxPt.y : minPt.y,
            (plane.z >= 0.0f) ? maxPt.z : minPt.z);

        // If p-vertex is outside this plane, the box is entirely outside
        float dist = plane.x * pVertex.x + plane.y * pVertex.y +
                     plane.z * pVertex.z + plane.w;
        if (dist < 0.0f) {
            return false;
        }
    }
    return true;
}

} // namespace voxelforge
