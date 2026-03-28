#include "Entity.h"
#include <cmath>
#include <cstdint>

namespace voxelforge {

// ---- MobInfo lookup table ----

static constexpr MobInfo MOB_INFO[] = {
    // name       HP   speed   w     h    bodyColor            headColor            hostile dmg
    {"Cow",      10,  1.0f, 0.9f, 1.3f, {0.44f,0.31f,0.22f}, {0.50f,0.36f,0.26f}, false, 0},
    {"Pig",       8,  1.0f, 0.9f, 0.9f, {0.90f,0.60f,0.55f}, {0.90f,0.65f,0.60f}, false, 0},
    {"Sheep",     8,  1.0f, 0.9f, 1.2f, {0.85f,0.85f,0.85f}, {0.75f,0.75f,0.75f}, false, 0},
    {"Chicken",   4,  1.5f, 0.4f, 0.7f, {0.95f,0.95f,0.95f}, {0.95f,0.70f,0.30f}, false, 0},
    {"Zombie",   20,  0.8f, 0.6f, 1.8f, {0.25f,0.50f,0.25f}, {0.30f,0.55f,0.30f}, true,  3},
    {"Skeleton", 20,  1.0f, 0.6f, 1.8f, {0.80f,0.80f,0.80f}, {0.85f,0.85f,0.80f}, true,  2},
    {"Creeper",  20,  0.7f, 0.6f, 1.7f, {0.30f,0.70f,0.30f}, {0.35f,0.75f,0.35f}, true,  6},
    {"Spider",   16,  1.3f, 1.2f, 0.8f, {0.35f,0.25f,0.20f}, {0.50f,0.10f,0.10f}, true,  2},
};

const MobInfo& getMobInfo(MobType type) {
    return MOB_INFO[static_cast<uint8_t>(type)];
}

// ---- Simple hash-based pseudo-random ----

static uint32_t hashPosition(const glm::vec3& pos) {
    auto ix = static_cast<uint32_t>(static_cast<int>(pos.x * 73856093.0f));
    auto iy = static_cast<uint32_t>(static_cast<int>(pos.y * 19349663.0f));
    auto iz = static_cast<uint32_t>(static_cast<int>(pos.z * 83492791.0f));
    uint32_t h = ix ^ iy ^ iz;
    h ^= h << 13;
    h ^= h >> 17;
    h ^= h << 5;
    return h;
}

// ---- Entity ----

Entity::Entity(MobType type, const glm::vec3& pos)
    : m_type(type)
    , m_position(pos)
    , m_health(getMobInfo(type).maxHealth) {
    // Deterministic initial yaw from position hash
    uint32_t h = hashPosition(pos);
    m_yaw = static_cast<float>(h % 3600) / 10.0f; // 0..360 degrees
}

bool Entity::isHostile() const {
    return getMobInfo(m_type).hostile;
}

const MobInfo& Entity::getInfo() const {
    return getMobInfo(m_type);
}

// ---- AI ----

void Entity::aiWander(float dt) {
    m_aiTimer -= dt;
    if (m_aiTimer <= 0.0f) {
        // Pick a new wander interval (3-5 seconds)
        uint32_t h = hashPosition(m_position);
        float interval = 3.0f + static_cast<float>(h % 200) / 100.0f;
        m_aiTimer = interval;

        // Randomly stop sometimes (~30% chance)
        if ((h >> 8) % 10 < 3) {
            m_wanderDir = glm::vec3(0.0f);
            return;
        }

        // Pick a random normalized XZ direction
        float angle = static_cast<float>(h % 6283) / 1000.0f; // 0..~2*PI
        m_wanderDir = glm::vec3(std::cos(angle), 0.0f, std::sin(angle));
    }

    const auto& info = getMobInfo(m_type);
    m_velocity.x = m_wanderDir.x * info.speed;
    m_velocity.z = m_wanderDir.z * info.speed;

    // Face movement direction
    if (m_wanderDir.x != 0.0f || m_wanderDir.z != 0.0f) {
        m_yaw = std::atan2(m_velocity.x, m_velocity.z) * (180.0f / 3.14159265f);
    }
}

void Entity::aiChase(float dt, const glm::vec3& target) {
    glm::vec3 dir = target - m_position;
    dir.y = 0.0f; // XZ only
    float dist = std::sqrt(dir.x * dir.x + dir.z * dir.z);

    // Give up if too far
    if (dist > 32.0f) {
        aiWander(dt);
        return;
    }

    if (dist > 0.01f) {
        dir.x /= dist;
        dir.z /= dist;
    }

    const auto& info = getMobInfo(m_type);
    float chaseSpeed = info.speed * 1.2f;
    m_velocity.x = dir.x * chaseSpeed;
    m_velocity.z = dir.z * chaseSpeed;

    // Face target
    m_yaw = std::atan2(dir.x, dir.z) * (180.0f / 3.14159265f);
}

// ---- Physics ----

bool Entity::collidesAt(const glm::vec3& pos, BlockAccessor getBlock) const {
    const auto& info = getMobInfo(m_type);
    float halfW = info.width * 0.5f;

    int minX = static_cast<int>(std::floor(pos.x - halfW));
    int maxX = static_cast<int>(std::floor(pos.x + halfW));
    int minY = static_cast<int>(std::floor(pos.y));
    int maxY = static_cast<int>(std::floor(pos.y + info.height));
    int minZ = static_cast<int>(std::floor(pos.z - halfW));
    int maxZ = static_cast<int>(std::floor(pos.z + halfW));

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            for (int z = minZ; z <= maxZ; ++z) {
                BlockType block = getBlock(x, y, z);
                if (isBlockSolid(block)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void Entity::applyPhysics(float dt, BlockAccessor getBlock) {
    // Gravity
    m_velocity.y -= 20.0f * dt;

    // Y collision
    glm::vec3 newPosY = m_position;
    newPosY.y += m_velocity.y * dt;
    if (collidesAt(newPosY, getBlock)) {
        if (m_velocity.y < 0.0f) {
            m_onGround = true;
        }
        m_velocity.y = 0.0f;
    } else {
        m_position.y = newPosY.y;
        m_onGround = false;
    }

    // X collision
    glm::vec3 newPosX = m_position;
    newPosX.x += m_velocity.x * dt;
    if (!collidesAt(newPosX, getBlock)) {
        m_position.x = newPosX.x;
    } else {
        m_velocity.x = 0.0f;
    }

    // Z collision
    glm::vec3 newPosZ = m_position;
    newPosZ.z += m_velocity.z * dt;
    if (!collidesAt(newPosZ, getBlock)) {
        m_position.z = newPosZ.z;
    } else {
        m_velocity.z = 0.0f;
    }

    // Air drag
    m_velocity.x *= 0.8f;
    m_velocity.z *= 0.8f;
}

// ---- Update ----

void Entity::update(float dt, const glm::vec3& playerPos, BlockAccessor getBlock) {
    // Decrement attack timer
    m_attackTimer -= dt;
    if (m_attackTimer < 0.0f) {
        m_attackTimer = 0.0f;
    }

    // AI behavior
    if (isHostile()) {
        glm::vec3 diff = playerPos - m_position;
        diff.y = 0.0f;
        float dist = std::sqrt(diff.x * diff.x + diff.z * diff.z);
        if (dist < 24.0f) {
            aiChase(dt, playerPos);
        } else {
            aiWander(dt);
        }
    } else {
        aiWander(dt);
    }

    applyPhysics(dt, getBlock);

    // Clamp Y
    if (m_position.y < 0.0f) {
        m_position.y = 0.0f;
        m_velocity.y = 0.0f;
    }
}

// ---- Damage ----

void Entity::damage(float amount) {
    m_health -= amount;
    if (m_health < 0.0f) {
        m_health = 0.0f;
    }
}

// ---- Overlap ----

bool Entity::overlapsPlayer(const glm::vec3& playerPos, float pw, float ph) const {
    const auto& info = getMobInfo(m_type);
    float halfW = info.width * 0.5f;
    float halfPW = pw * 0.5f;

    // AABB overlap test
    bool overlapX = (m_position.x - halfW) < (playerPos.x + halfPW) &&
                    (m_position.x + halfW) > (playerPos.x - halfPW);
    bool overlapZ = (m_position.z - halfW) < (playerPos.z + halfPW) &&
                    (m_position.z + halfW) > (playerPos.z - halfPW);
    bool overlapY = m_position.y < (playerPos.y + ph) &&
                    (m_position.y + info.height) > playerPos.y;

    return overlapX && overlapY && overlapZ;
}

} // namespace voxelforge
