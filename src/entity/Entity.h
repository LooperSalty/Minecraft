#pragma once
#include "../world/Block.h"
#include <glm/glm.hpp>
#include <functional>

namespace voxelforge {

using BlockAccessor = std::function<BlockType(int, int, int)>;

enum class MobType : uint8_t {
    Cow, Pig, Sheep, Chicken,
    Zombie, Skeleton, Creeper, Spider,
    COUNT
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

    void update(float dt, const glm::vec3& playerPos, BlockAccessor getBlock);
    void damage(float amount);
    bool isAlive() const { return m_health > 0.0f; }
    bool isHostile() const;

    MobType getType() const { return m_type; }
    const glm::vec3& getPosition() const { return m_position; }
    float getYaw() const { return m_yaw; }
    float getHealth() const { return m_health; }
    const MobInfo& getInfo() const;

    bool overlapsPlayer(const glm::vec3& playerPos, float pw, float ph) const;
    float m_attackTimer = 0.0f;

private:
    void aiWander(float dt);
    void aiChase(float dt, const glm::vec3& target);
    void applyPhysics(float dt, BlockAccessor getBlock);
    bool collidesAt(const glm::vec3& pos, BlockAccessor getBlock) const;

    MobType   m_type;
    glm::vec3 m_position;
    glm::vec3 m_velocity{0.0f};
    float     m_yaw = 0.0f;
    float     m_health;
    float     m_aiTimer = 0.0f;
    glm::vec3 m_wanderDir{0.0f};
    bool      m_onGround = false;
};

} // namespace voxelforge
