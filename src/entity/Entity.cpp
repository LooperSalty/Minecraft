#include "Entity.h"
#include <cmath>
#include <cstdint>
#include <algorithm>

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

// ---- Entity ----

Entity::Entity(MobType type, const glm::vec3& pos)
    : m_type(type)
    , m_position(pos)
    , m_health(getMobInfo(type).maxHealth)
{
    // Deterministic RNG seed from position
    auto ix = static_cast<uint32_t>(static_cast<int>(pos.x * 73856093.0f));
    auto iy = static_cast<uint32_t>(static_cast<int>(pos.y * 19349663.0f));
    auto iz = static_cast<uint32_t>(static_cast<int>(pos.z * 83492791.0f));
    m_rngState = ix ^ iy ^ iz;
    if (m_rngState == 0) m_rngState = 1;

    m_yaw = static_cast<float>(nextRng() % 3600) / 10.0f;
    m_headYaw = m_yaw;
    m_aiTimer = randomFloat() * 3.0f; // stagger initial AI timers
}

uint32_t Entity::nextRng() {
    m_rngState ^= m_rngState << 13;
    m_rngState ^= m_rngState >> 17;
    m_rngState ^= m_rngState << 5;
    return m_rngState;
}

float Entity::randomFloat() {
    return static_cast<float>(nextRng() % 10000) / 10000.0f;
}

bool Entity::isHostile() const {
    return getMobInfo(m_type).hostile;
}

const MobInfo& Entity::getInfo() const {
    return getMobInfo(m_type);
}

// ---- AABB for raycast ----

glm::vec3 Entity::getAABBMin() const {
    const auto& info = getInfo();
    float halfW = info.width * 0.5f;
    return glm::vec3(m_position.x - halfW, m_position.y, m_position.z - halfW);
}

glm::vec3 Entity::getAABBMax() const {
    const auto& info = getInfo();
    float halfW = info.width * 0.5f;
    return glm::vec3(m_position.x + halfW, m_position.y + info.height, m_position.z + halfW);
}

// ---- Environment checks ----

bool Entity::hasGroundAhead(const glm::vec3& dir, BlockAccessor getBlock) const {
    // Check if there's solid ground 1 block ahead and 1 block down
    glm::vec3 ahead = m_position + dir * 1.2f;
    int ax = static_cast<int>(std::floor(ahead.x));
    int ay = static_cast<int>(std::floor(m_position.y)) - 1;
    int az = static_cast<int>(std::floor(ahead.z));
    return isBlockSolid(getBlock(ax, ay, az));
}

bool Entity::hasWallAhead(const glm::vec3& dir, BlockAccessor getBlock) const {
    glm::vec3 ahead = m_position + dir * 0.6f;
    int ax = static_cast<int>(std::floor(ahead.x));
    int ay = static_cast<int>(std::floor(m_position.y));
    int az = static_cast<int>(std::floor(ahead.z));
    // Check both feet level and head level
    return isBlockSolid(getBlock(ax, ay, az)) || isBlockSolid(getBlock(ax, ay + 1, az));
}

// ---- AI States ----

void Entity::aiIdle(float dt) {
    m_velocity.x = 0.0f;
    m_velocity.z = 0.0f;
    m_stateTimer -= dt;

    // Random head look
    m_headLookTimer -= dt;
    if (m_headLookTimer <= 0.0f) {
        m_headYaw = m_yaw + (randomFloat() - 0.5f) * 90.0f;
        m_headLookTimer = 1.0f + randomFloat() * 3.0f;
    }

    if (m_stateTimer <= 0.0f) {
        m_aiState = AIState::Wander;
        m_stateTimer = 2.0f + randomFloat() * 3.0f;

        // Pick a random walk direction
        float angle = randomFloat() * 6.2831853f;
        m_wanderDir = glm::vec3(std::cos(angle), 0.0f, std::sin(angle));
    }
}

void Entity::aiWander(float dt, BlockAccessor getBlock) {
    m_stateTimer -= dt;

    const auto& info = getInfo();
    float speed = info.speed;

    // Check for obstacles
    if (m_wanderDir.x != 0.0f || m_wanderDir.z != 0.0f) {
        if (hasWallAhead(m_wanderDir, getBlock) || !hasGroundAhead(m_wanderDir, getBlock)) {
            // Turn around or pick new direction
            float angle = randomFloat() * 6.2831853f;
            m_wanderDir = glm::vec3(std::cos(angle), 0.0f, std::sin(angle));
        }
    }

    m_velocity.x = m_wanderDir.x * speed;
    m_velocity.z = m_wanderDir.z * speed;

    // Face movement direction
    if (m_wanderDir.x != 0.0f || m_wanderDir.z != 0.0f) {
        m_yaw = std::atan2(m_wanderDir.x, m_wanderDir.z) * (180.0f / 3.14159265f);
        m_headYaw = m_yaw;
    }

    if (m_stateTimer <= 0.0f) {
        m_aiState = AIState::Idle;
        m_stateTimer = 2.0f + randomFloat() * 4.0f;
    }
}

void Entity::aiChase(float dt, const glm::vec3& target, BlockAccessor getBlock) {
    glm::vec3 dir = target - m_position;
    dir.y = 0.0f;
    float dist = std::sqrt(dir.x * dir.x + dir.z * dir.z);

    // Give up if too far
    if (dist > 32.0f) {
        m_aiState = AIState::Wander;
        m_stateTimer = 3.0f;
        float angle = randomFloat() * 6.2831853f;
        m_wanderDir = glm::vec3(std::cos(angle), 0.0f, std::sin(angle));
        return;
    }

    if (dist > 0.01f) {
        dir.x /= dist;
        dir.z /= dist;
    }

    const auto& info = getInfo();
    float chaseSpeed = info.speed * 1.3f;

    // Skeleton behavior: stay 8-12 blocks away
    if (m_type == MobType::Skeleton) {
        if (dist < 8.0f) {
            // Back away
            dir = -dir;
            chaseSpeed = info.speed * 0.8f;
        } else if (dist < 12.0f) {
            // Stay put, just face player
            m_velocity.x = 0.0f;
            m_velocity.z = 0.0f;
            m_yaw = std::atan2(dir.x, dir.z) * (180.0f / 3.14159265f);
            m_headYaw = m_yaw;
            return;
        }
    }

    // Check for cliff ahead - if chasing, still check walls
    glm::vec3 moveDir(dir.x, 0.0f, dir.z);
    if (hasWallAhead(moveDir, getBlock)) {
        // Try to jump over
        if (m_onGround) {
            m_velocity.y = 8.0f;
        }
    }

    m_velocity.x = dir.x * chaseSpeed;
    m_velocity.z = dir.z * chaseSpeed;

    m_yaw = std::atan2(dir.x, dir.z) * (180.0f / 3.14159265f);
    m_headYaw = m_yaw;

    // Switch to attack if close enough
    if (dist < 2.0f && m_type != MobType::Creeper) {
        m_aiState = AIState::Attack;
    }

    // Creeper: switch to fuse if close
    if (m_type == MobType::Creeper && dist < 3.0f) {
        m_aiState = AIState::CreeperFuse;
        m_creeperFuse = 0.0f;
    }
}

void Entity::aiFlee(float dt, const glm::vec3& threat, BlockAccessor getBlock) {
    m_stateTimer -= dt;

    glm::vec3 dir = m_position - threat;
    dir.y = 0.0f;
    float dist = std::sqrt(dir.x * dir.x + dir.z * dir.z);
    if (dist > 0.01f) {
        dir.x /= dist;
        dir.z /= dist;
    }

    const auto& info = getInfo();
    float fleeSpeed = info.speed * 1.5f;

    // Avoid cliffs and walls
    glm::vec3 moveDir(dir.x, 0.0f, dir.z);
    if (hasWallAhead(moveDir, getBlock) || !hasGroundAhead(moveDir, getBlock)) {
        // Deflect 90 degrees
        float temp = dir.x;
        dir.x = -dir.z;
        dir.z = temp;
    }

    m_velocity.x = dir.x * fleeSpeed;
    m_velocity.z = dir.z * fleeSpeed;

    m_yaw = std::atan2(dir.x, dir.z) * (180.0f / 3.14159265f);
    m_headYaw = m_yaw;

    if (m_stateTimer <= 0.0f) {
        m_aiState = AIState::Idle;
        m_stateTimer = 2.0f + randomFloat() * 3.0f;
    }
}

void Entity::aiCreeperFuse(float dt, const glm::vec3& playerPos) {
    // Stop moving
    m_velocity.x = 0.0f;
    m_velocity.z = 0.0f;

    // Face player
    glm::vec3 dir = playerPos - m_position;
    dir.y = 0.0f;
    float dist = std::sqrt(dir.x * dir.x + dir.z * dir.z);
    if (dist > 0.01f) {
        m_yaw = std::atan2(dir.x / dist, dir.z / dist) * (180.0f / 3.14159265f);
        m_headYaw = m_yaw;
    }

    m_creeperFuse += dt;

    // If player moves away, cancel fuse
    if (dist > 4.5f) {
        m_creeperFuse = 0.0f;
        m_aiState = AIState::Chase;
        return;
    }

    // Explode after 1.5 seconds
    if (m_creeperFuse >= 1.5f) {
        // Deal damage is handled externally via checkPlayerDamage
        // Kill self
        m_health = 0.0f;
    }
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

void Entity::update(float dt, const glm::vec3& playerPos, BlockAccessor getBlock,
                    float dayProgress) {
    // Decrement attack timer
    m_attackTimer -= dt;
    if (m_attackTimer < 0.0f) m_attackTimer = 0.0f;

    // Update damage flash
    if (m_damageFlash > 0.0f) {
        m_damageFlash -= dt * 4.0f; // Flash fades over 0.25s
        if (m_damageFlash < 0.0f) m_damageFlash = 0.0f;
    }

    // Update walk animation
    float hSpeed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
    if (hSpeed > 0.1f) {
        m_walkTime += dt * hSpeed * 2.5f;
    } else {
        // Smoothly return to rest
        m_walkTime *= 0.9f;
    }

    // Distance to player
    glm::vec3 diff = playerPos - m_position;
    diff.y = 0.0f;
    float distToPlayer = std::sqrt(diff.x * diff.x + diff.z * diff.z);

    bool isNight = (dayProgress > 0.54f && dayProgress < 0.96f);

    // AI behavior based on mob type
    if (!isHostile()) {
        // -- Passive mob AI --
        switch (m_aiState) {
            case AIState::Idle:
                aiIdle(dt);
                break;
            case AIState::Wander:
                aiWander(dt, getBlock);
                break;
            case AIState::Flee:
                aiFlee(dt, m_fleeFrom, getBlock);
                break;
            default:
                m_aiState = AIState::Idle;
                m_stateTimer = 2.0f;
                break;
        }
    } else {
        // -- Hostile mob AI --
        // Spider is neutral during the day
        bool isAggressive = true;
        if (m_type == MobType::Spider && !isNight) {
            isAggressive = false;
        }

        if (!isAggressive) {
            // Spider during day: acts like passive mob
            switch (m_aiState) {
                case AIState::Idle:
                    aiIdle(dt);
                    break;
                case AIState::Wander:
                    aiWander(dt, getBlock);
                    break;
                case AIState::Flee:
                    aiFlee(dt, m_fleeFrom, getBlock);
                    break;
                default:
                    m_aiState = AIState::Idle;
                    m_stateTimer = 2.0f;
                    break;
            }
        } else {
            // Aggressive hostile mob
            if (m_aiState == AIState::CreeperFuse) {
                aiCreeperFuse(dt, playerPos);
            } else if (distToPlayer < 16.0f) {
                if (m_aiState != AIState::Chase && m_aiState != AIState::Attack
                    && m_aiState != AIState::CreeperFuse) {
                    m_aiState = AIState::Chase;
                }
                if (m_aiState == AIState::Attack) {
                    // Stay near player, face them
                    m_velocity.x = 0.0f;
                    m_velocity.z = 0.0f;
                    if (distToPlayer > 0.01f) {
                        glm::vec3 d = diff / distToPlayer;
                        m_yaw = std::atan2(d.x, d.z) * (180.0f / 3.14159265f);
                        m_headYaw = m_yaw;
                    }
                    // If target moves away, resume chase
                    if (distToPlayer > 2.5f) {
                        m_aiState = AIState::Chase;
                    }
                } else {
                    aiChase(dt, playerPos, getBlock);
                }
            } else if (distToPlayer < 24.0f) {
                // Within detection range but not chase range yet -
                // wander toward player slowly
                switch (m_aiState) {
                    case AIState::Idle:
                        aiIdle(dt);
                        break;
                    case AIState::Wander:
                        aiWander(dt, getBlock);
                        break;
                    default:
                        m_aiState = AIState::Idle;
                        m_stateTimer = 1.0f + randomFloat() * 2.0f;
                        break;
                }
            } else {
                // Far from player, wander
                switch (m_aiState) {
                    case AIState::Idle:
                        aiIdle(dt);
                        break;
                    case AIState::Wander:
                        aiWander(dt, getBlock);
                        break;
                    default:
                        m_aiState = AIState::Idle;
                        m_stateTimer = 2.0f;
                        break;
                }
            }
        }
    }

    applyPhysics(dt, getBlock);

    // Clamp Y
    if (m_position.y < 0.0f) {
        m_position.y = 0.0f;
        m_velocity.y = 0.0f;
    }
}

// ---- Damage ----

void Entity::damage(float amount, const glm::vec3& knockbackDir) {
    m_health -= amount;
    if (m_health < 0.0f) m_health = 0.0f;

    // Damage flash
    m_damageFlash = 1.0f;

    // Knockback
    m_velocity.x += knockbackDir.x * 8.0f;
    m_velocity.y += 4.0f; // Pop up
    m_velocity.z += knockbackDir.z * 8.0f;

    // Passive mobs flee when hit
    if (!isHostile()) {
        m_aiState = AIState::Flee;
        m_stateTimer = 3.0f;
        m_fleeFrom = m_position - knockbackDir; // Flee from the attacker's direction
    }
}

// ---- Overlap ----

bool Entity::overlapsPlayer(const glm::vec3& playerPos, float pw, float ph) const {
    const auto& info = getMobInfo(m_type);
    float halfW = info.width * 0.5f;
    float halfPW = pw * 0.5f;

    bool overlapX = (m_position.x - halfW) < (playerPos.x + halfPW) &&
                    (m_position.x + halfW) > (playerPos.x - halfPW);
    bool overlapZ = (m_position.z - halfW) < (playerPos.z + halfPW) &&
                    (m_position.z + halfW) > (playerPos.z - halfPW);
    bool overlapY = m_position.y < (playerPos.y + ph) &&
                    (m_position.y + info.height) > playerPos.y;

    return overlapX && overlapY && overlapZ;
}

} // namespace voxelforge
