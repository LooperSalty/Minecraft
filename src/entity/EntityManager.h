#pragma once
#include "Entity.h"
#include "../render/Shader.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace voxelforge {

class EntityManager {
public:
    void init();
    void cleanup();

    void update(float dt, const glm::vec3& playerPos, BlockAccessor getBlock,
                float dayProgress);
    void render(const glm::mat4& viewProj);

    // Returns total damage dealt to player this frame
    float checkPlayerDamage(const glm::vec3& playerPos);

    int getCount() const { return static_cast<int>(m_entities.size()); }

private:
    void trySpawn(const glm::vec3& playerPos, BlockAccessor getBlock, float dayProgress);
    void renderEntity(const Entity& e, const glm::mat4& vp);

    std::vector<Entity> m_entities;
    Shader m_shader;
    GLuint m_cubeVAO = 0, m_cubeVBO = 0;
    float  m_spawnTimer = 0.0f;

    static constexpr int   MAX_ENTITIES   = 60;
    static constexpr float SPAWN_INTERVAL = 3.0f;
    static constexpr float DESPAWN_DIST   = 96.0f;
};

} // namespace voxelforge
