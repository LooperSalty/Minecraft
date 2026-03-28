#include "EntityManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace voxelforge {

// ---- Deterministic RNG ----

static uint32_t rng(uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

// ---- Unit cube mesh (36 vertices for 12 triangles) ----

static constexpr float CUBE_VERTS[] = {
    // Back face (z=0)
    0,0,0,  1,1,0,  1,0,0,
    0,0,0,  0,1,0,  1,1,0,
    // Front face (z=1)
    0,0,1,  1,0,1,  1,1,1,
    0,0,1,  1,1,1,  0,1,1,
    // Left face (x=0)
    0,0,0,  0,0,1,  0,1,1,
    0,0,0,  0,1,1,  0,1,0,
    // Right face (x=1)
    1,0,0,  1,1,1,  1,0,1,
    1,0,0,  1,1,0,  1,1,1,
    // Bottom face (y=0)
    0,0,0,  1,0,0,  1,0,1,
    0,0,0,  1,0,1,  0,0,1,
    // Top face (y=1)
    0,1,0,  1,1,1,  1,1,0,
    0,1,0,  0,1,1,  1,1,1,
};

// ---- Init / Cleanup ----

void EntityManager::init() {
    m_shader.loadFromFiles("assets/shaders/line.vert", "assets/shaders/line.frag");

    glGenVertexArrays(1, &m_cubeVAO);
    glGenBuffers(1, &m_cubeVBO);

    glBindVertexArray(m_cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTS), CUBE_VERTS, GL_STATIC_DRAW);

    // location 0 = aPos (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    glBindVertexArray(0);
}

void EntityManager::cleanup() {
    if (m_cubeVBO) {
        glDeleteBuffers(1, &m_cubeVBO);
        m_cubeVBO = 0;
    }
    if (m_cubeVAO) {
        glDeleteVertexArrays(1, &m_cubeVAO);
        m_cubeVAO = 0;
    }
}

// ---- Spawning ----

void EntityManager::trySpawn(const glm::vec3& playerPos, BlockAccessor getBlock,
                             float dayProgress) {
    if (static_cast<int>(m_entities.size()) >= MAX_ENTITIES) {
        return;
    }

    // Seed RNG from player position and a rolling counter
    static uint32_t frameCounter = 0;
    ++frameCounter;
    uint32_t state = static_cast<uint32_t>(
        static_cast<int>(playerPos.x * 73856093.0f) ^
        static_cast<int>(playerPos.z * 19349663.0f) ^
        frameCounter);
    if (state == 0) state = 1;

    // Random distance 20-40 blocks from player
    float dist = 20.0f + static_cast<float>(rng(state) % 2000) / 100.0f;
    float angle = static_cast<float>(rng(state) % 6283) / 1000.0f;

    float spawnX = playerPos.x + std::cos(angle) * dist;
    float spawnZ = playerPos.z + std::sin(angle) * dist;

    // Find surface Y by scanning down from Y=100
    int ix = static_cast<int>(std::floor(spawnX));
    int iz = static_cast<int>(std::floor(spawnZ));
    int surfaceY = -1;

    for (int y = 100; y >= 1; --y) {
        BlockType below = getBlock(ix, y - 1, iz);
        BlockType at    = getBlock(ix, y, iz);
        if (isBlockSolid(below) && at == BlockType::Air) {
            surfaceY = y;
            break;
        }
    }

    if (surfaceY < 0) {
        return; // No valid surface found
    }

    // Determine mob category based on time of day
    bool isNight = (dayProgress > 0.54f && dayProgress < 0.96f);

    MobType mobType;
    if (isNight) {
        // 75% hostile, 25% passive
        uint32_t roll = rng(state) % 100;
        if (roll < 75) {
            // Hostile: Zombie(4), Skeleton(5), Creeper(6), Spider(7)
            uint32_t hostileIdx = 4 + (rng(state) % 4);
            mobType = static_cast<MobType>(hostileIdx);
        } else {
            // Passive: Cow(0), Pig(1), Sheep(2), Chicken(3)
            uint32_t passiveIdx = rng(state) % 4;
            mobType = static_cast<MobType>(passiveIdx);
        }
    } else {
        // Day: passive only
        uint32_t passiveIdx = rng(state) % 4;
        mobType = static_cast<MobType>(passiveIdx);
    }

    glm::vec3 spawnPos(spawnX, static_cast<float>(surfaceY), spawnZ);
    m_entities.emplace_back(mobType, spawnPos);
}

// ---- Update ----

void EntityManager::update(float dt, const glm::vec3& playerPos,
                           BlockAccessor getBlock, float dayProgress) {
    // Spawn timer
    m_spawnTimer += dt;
    if (m_spawnTimer >= SPAWN_INTERVAL) {
        trySpawn(playerPos, getBlock, dayProgress);
        m_spawnTimer = 0.0f;
    }

    // Update all entities
    for (auto& entity : m_entities) {
        entity.update(dt, playerPos, getBlock);
    }

    // Determine if it's day
    bool isDay = (dayProgress <= 0.54f || dayProgress >= 0.96f);

    // Remove dead entities, despawn far entities, and despawn hostile mobs during day
    m_entities.erase(
        std::remove_if(m_entities.begin(), m_entities.end(),
            [&](const Entity& e) {
                if (!e.isAlive()) {
                    return true;
                }

                glm::vec3 diff = e.getPosition() - playerPos;
                float dist = std::sqrt(diff.x * diff.x + diff.z * diff.z);

                // Despawn if too far from player
                if (dist > DESPAWN_DIST) {
                    return true;
                }

                // Despawn hostile mobs during day if > 32 blocks
                if (isDay && e.isHostile() && dist > 32.0f) {
                    return true;
                }

                return false;
            }),
        m_entities.end());
}

// ---- Player damage ----

float EntityManager::checkPlayerDamage(const glm::vec3& playerPos) {
    float totalDamage = 0.0f;
    constexpr float PLAYER_WIDTH  = 0.6f;
    constexpr float PLAYER_HEIGHT = 1.8f;

    for (auto& entity : m_entities) {
        if (!entity.isHostile() || !entity.isAlive()) {
            continue;
        }
        if (entity.m_attackTimer > 0.0f) {
            continue;
        }
        if (entity.overlapsPlayer(playerPos, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            totalDamage += entity.getInfo().attackDamage;
            entity.m_attackTimer = 1.5f;
        }
    }

    return totalDamage;
}

// ---- Rendering ----

void EntityManager::renderEntity(const Entity& e, const glm::mat4& vp) {
    const auto& info = e.getInfo();
    float yawRad = glm::radians(e.getYaw());

    // --- Body ---
    glm::mat4 bodyModel(1.0f);
    bodyModel = glm::translate(bodyModel, e.getPosition());
    bodyModel = glm::rotate(bodyModel, yawRad, glm::vec3(0.0f, 1.0f, 0.0f));
    // Center the body horizontally
    float bodyW = info.width;
    float bodyH = info.height * 0.65f;
    float bodyD = info.width * 0.6f;
    bodyModel = glm::translate(bodyModel, glm::vec3(-bodyW * 0.5f, 0.0f, -bodyD * 0.5f));
    bodyModel = glm::scale(bodyModel, glm::vec3(bodyW, bodyH, bodyD));

    glm::mat4 bodyMVP = vp * bodyModel;
    m_shader.setMat4("uMVP", bodyMVP);
    m_shader.setVec4("uColor", glm::vec4(info.bodyColor, 1.0f));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // --- Head ---
    glm::mat4 headModel(1.0f);
    headModel = glm::translate(headModel, e.getPosition());
    headModel = glm::rotate(headModel, yawRad, glm::vec3(0.0f, 1.0f, 0.0f));
    float headW = info.width * 0.5f;
    float headH = info.height * 0.3f;
    float headD = info.width * 0.5f;
    // Position head on top of body, centered
    headModel = glm::translate(headModel, glm::vec3(-headW * 0.5f, info.height * 0.65f, -headD * 0.5f));
    headModel = glm::scale(headModel, glm::vec3(headW, headH, headD));

    glm::mat4 headMVP = vp * headModel;
    m_shader.setMat4("uMVP", headMVP);
    m_shader.setVec4("uColor", glm::vec4(info.headColor, 1.0f));
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void EntityManager::render(const glm::mat4& viewProj) {
    m_shader.use();
    glBindVertexArray(m_cubeVAO);

    for (const auto& entity : m_entities) {
        if (entity.isAlive()) {
            renderEntity(entity, viewProj);
        }
    }

    glBindVertexArray(0);
}

} // namespace voxelforge
