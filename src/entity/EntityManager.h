#pragma once
#include "Entity.h"
#include "../render/Shader.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace voxelforge {

struct EntityHit {
    bool hit = false;
    int entityIndex = -1;
    float distance = 0.0f;
};

class EntityManager {
public:
    void init();
    void cleanup();

    void update(float dt, const glm::vec3& playerPos, BlockAccessor getBlock,
                float dayProgress);
    void render(const glm::mat4& viewProj);

    // Returns total damage dealt to player this frame
    float checkPlayerDamage(const glm::vec3& playerPos);

    // Combat: raycast against entity bounding boxes
    EntityHit raycastEntity(const glm::vec3& origin, const glm::vec3& dir, float maxDist) const;

    // Combat: damage an entity by index
    void damageEntity(int index, float amount, const glm::vec3& knockbackDir);

    int getCount() const { return static_cast<int>(m_entities.size()); }

private:
    void trySpawn(const glm::vec3& playerPos, BlockAccessor getBlock, float dayProgress);

    // Multi-part mob rendering
    void renderEntity(const Entity& e, const glm::mat4& vp);
    void drawCube(const glm::mat4& vp, const glm::mat4& model, const glm::vec3& color,
                  float damageFlash);

    // Specific mob body renderers
    void renderHumanoid(const Entity& e, const glm::mat4& vp, float bodyW, float bodyH,
                        float bodyD, float headSize, float limbW, float limbH,
                        bool hasArms, float armAngle);
    void renderCow(const Entity& e, const glm::mat4& vp);
    void renderPig(const Entity& e, const glm::mat4& vp);
    void renderSheep(const Entity& e, const glm::mat4& vp);
    void renderChicken(const Entity& e, const glm::mat4& vp);
    void renderZombie(const Entity& e, const glm::mat4& vp);
    void renderSkeleton(const Entity& e, const glm::mat4& vp);
    void renderCreeper(const Entity& e, const glm::mat4& vp);
    void renderSpider(const Entity& e, const glm::mat4& vp);

    std::vector<Entity> m_entities;
    Shader m_shader;
    GLuint m_cubeVAO = 0, m_cubeVBO = 0;
    float  m_spawnTimer = 0.0f;

    static constexpr int   MAX_ENTITIES   = 30;
    static constexpr float SPAWN_INTERVAL = 3.0f;
    static constexpr float DESPAWN_DIST   = 64.0f;
};

} // namespace voxelforge
