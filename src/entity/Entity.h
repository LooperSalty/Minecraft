#pragma once
#include "../world/Block.h"
#include <glm/glm.hpp>
#include <functional>
#include <cstdint>

namespace voxelforge {

using BlockAccessor = std::function<BlockType(int, int, int)>;

enum class MobType : uint8_t {
    Cow, Pig, Sheep, Chicken,
    Zombie, Skeleton, Creeper, Spider,
    COUNT
};

enum class AIState : uint8_t {
    Idle, Wander, Chase, Flee, Attack, CreeperFuse
};

struct MobInfo {
    const char* name;
    float maxHealth;
    float speed;
    float width, height;
    glm::vec3 bodyColor;
    glm::vec3 headColor;
    bool hostile;
    float attackDamage;
};

const MobInfo& getMobInfo(MobType type);

class Entity {
public:
    Entity(MobType type, const glm::vec3& pos);

    void update(float dt, const glm::vec3& playerPos, BlockAccessor getBlock,
                float dayProgress);
    void damage(float amount, const glm::vec3& knockbackDir);
    bool isAlive() const { return m_health > 0.0f; }
    bool isHostile() const;

    MobType getType() const { return m_type; }
    const glm::vec3& getPosition() const { return m_position; }
    float getYaw() const { return m_yaw; }
    float getHealth() const { return m_health; }
    float getMaxHealth() const { return getMobInfo(m_type).maxHealth; }
    const MobInfo& getInfo() const;
    AIState getAIState() const { return m_aiState; }

    // Animation state (read by renderer)
    float getWalkTime() const { return m_walkTime; }
    float getDamageFlash() const { return m_damageFlash; }
    float getCreeperFuse() const { return m_creeperFuse; }
    float getHeadYaw() const { return m_headYaw; }

    bool overlapsPlayer(const glm::vec3& playerPos, float pw, float ph) const;
    float m_attackTimer = 0.0f;

    // AABB for raycast hit testing
    glm::vec3 getAABBMin() const;
    glm::vec3 getAABBMax() const;

private:
    void aiIdle(float dt);
    void aiWander(float dt, BlockAccessor getBlock);
    void aiChase(float dt, const glm::vec3& target, BlockAccessor getBlock);
    void aiFlee(float dt, const glm::vec3& threat, BlockAccessor getBlock);
    void aiCreeperFuse(float dt, const glm::vec3& playerPos);
    void applyPhysics(float dt, BlockAccessor getBlock);
    bool collidesAt(const glm::vec3& pos, BlockAccessor getBlock) const;
    bool hasGroundAhead(const glm::vec3& dir, BlockAccessor getBlock) const;
    bool hasWallAhead(const glm::vec3& dir, BlockAccessor getBlock) const;

    MobType   m_type;
    glm::vec3 m_position;
    glm::vec3 m_velocity{0.0f};
    float     m_yaw = 0.0f;
    float     m_headYaw = 0.0f;
    float     m_health;
    bool      m_onGround = false;

    // AI state machine
    AIState   m_aiState = AIState::Idle;
    float     m_aiTimer = 0.0f;
    float     m_stateTimer = 0.0f;
    glm::vec3 m_wanderDir{0.0f};
    glm::vec3 m_fleeFrom{0.0f};

    // Animation
    float     m_walkTime = 0.0f;
    float     m_damageFlash = 0.0f;
    float     m_creeperFuse = 0.0f;
    float     m_headLookTimer = 0.0f;

    // RNG state per entity
    uint32_t  m_rngState;
    uint32_t  nextRng();
    float     randomFloat(); // 0..1
};

} // namespace voxelforge
