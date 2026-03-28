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

    // Random distance 24-48 blocks from player
    float dist = 24.0f + static_cast<float>(rng(state) % 2400) / 100.0f;
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
        BlockType above = getBlock(ix, y + 1, iz);
        if (isBlockSolid(below) && at == BlockType::Air && above == BlockType::Air) {
            // Don't spawn in water
            if (below != BlockType::Water) {
                surfaceY = y;
                break;
            }
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
            uint32_t hostileIdx = 4 + (rng(state) % 4);
            mobType = static_cast<MobType>(hostileIdx);
        } else {
            uint32_t passiveIdx = rng(state) % 4;
            mobType = static_cast<MobType>(passiveIdx);
        }
    } else {
        // Day: passive only
        uint32_t passiveIdx = rng(state) % 4;
        mobType = static_cast<MobType>(passiveIdx);
    }

    glm::vec3 spawnPos(spawnX, static_cast<float>(surfaceY), spawnZ);

    // Spawn in groups of 2-4 for passive mobs
    int groupSize = 1;
    if (!getMobInfo(mobType).hostile) {
        groupSize = 2 + static_cast<int>(rng(state) % 3); // 2-4
    }

    for (int i = 0; i < groupSize; ++i) {
        if (static_cast<int>(m_entities.size()) >= MAX_ENTITIES) break;

        float offsetX = static_cast<float>(static_cast<int>(rng(state) % 600) - 300) / 100.0f;
        float offsetZ = static_cast<float>(static_cast<int>(rng(state) % 600) - 300) / 100.0f;

        glm::vec3 groupPos = spawnPos + glm::vec3(offsetX, 0.0f, offsetZ);

        // Verify this group position has ground
        int gx = static_cast<int>(std::floor(groupPos.x));
        int gz = static_cast<int>(std::floor(groupPos.z));
        int gy = surfaceY;
        BlockType gBelow = getBlock(gx, gy - 1, gz);
        BlockType gAt    = getBlock(gx, gy, gz);
        if (isBlockSolid(gBelow) && gAt == BlockType::Air) {
            m_entities.emplace_back(mobType, groupPos);
        }
    }
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
        entity.update(dt, playerPos, getBlock, dayProgress);
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

        // Creeper explosion damage
        if (entity.getType() == MobType::Creeper && entity.getAIState() == AIState::CreeperFuse) {
            if (entity.getCreeperFuse() >= 1.5f) {
                // Creeper about to die = explode. Damage based on distance.
                glm::vec3 diff = playerPos - entity.getPosition();
                float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
                if (dist < 5.0f) {
                    float dmgFactor = 1.0f - (dist / 5.0f);
                    totalDamage += entity.getInfo().attackDamage * dmgFactor * 2.0f;
                }
            }
            continue;
        }

        if (entity.overlapsPlayer(playerPos, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            totalDamage += entity.getInfo().attackDamage;
            entity.m_attackTimer = 1.5f;
        }
    }

    return totalDamage;
}

// ---- Combat: Raycast against entities ----

EntityHit EntityManager::raycastEntity(const glm::vec3& origin, const glm::vec3& dir,
                                        float maxDist) const {
    EntityHit closest;
    closest.distance = maxDist + 1.0f;

    for (int i = 0; i < static_cast<int>(m_entities.size()); ++i) {
        const Entity& e = m_entities[i];
        if (!e.isAlive()) continue;

        glm::vec3 bmin = e.getAABBMin();
        glm::vec3 bmax = e.getAABBMax();

        // Ray-AABB intersection (slab method)
        float tmin = 0.0f;
        float tmax = maxDist;

        for (int axis = 0; axis < 3; ++axis) {
            float o = origin[axis];
            float d = dir[axis];
            float lo = bmin[axis];
            float hi = bmax[axis];

            if (std::abs(d) < 1e-8f) {
                // Ray parallel to slab
                if (o < lo || o > hi) {
                    tmin = maxDist + 1.0f; // No hit
                    break;
                }
            } else {
                float t1 = (lo - o) / d;
                float t2 = (hi - o) / d;
                if (t1 > t2) std::swap(t1, t2);
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);
                if (tmin > tmax) {
                    tmin = maxDist + 1.0f;
                    break;
                }
            }
        }

        if (tmin <= maxDist && tmin < closest.distance && tmin >= 0.0f) {
            closest.hit = true;
            closest.entityIndex = i;
            closest.distance = tmin;
        }
    }

    return closest;
}

// ---- Combat: Damage entity ----

void EntityManager::damageEntity(int index, float amount, const glm::vec3& knockbackDir) {
    if (index < 0 || index >= static_cast<int>(m_entities.size())) return;
    m_entities[index].damage(amount, knockbackDir);
}

// ---- Drawing helper ----

void drawCubeHelper(Shader& shader, const glm::mat4& vp, const glm::mat4& model,
                    const glm::vec3& color, float damageFlash) {
    glm::vec3 finalColor = color;
    if (damageFlash > 0.0f) {
        // Mix with red for damage flash
        finalColor = glm::mix(color, glm::vec3(1.0f, 0.2f, 0.2f), damageFlash);
    }

    glm::mat4 mvp = vp * model;
    shader.setMat4("uMVP", mvp);
    shader.setVec4("uColor", glm::vec4(finalColor, 1.0f));
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

// ---- Mob-specific renderers ----

// Helper: build a model matrix for a body part centered at an offset from the entity's base
static glm::mat4 partModel(const glm::vec3& entityPos, float entityYaw,
                            const glm::vec3& offset, const glm::vec3& size,
                            const glm::vec3& pivot = glm::vec3(0.0f),
                            float rotAngle = 0.0f,
                            const glm::vec3& rotAxis = glm::vec3(1.0f, 0.0f, 0.0f)) {
    float yawRad = glm::radians(entityYaw);
    glm::mat4 m(1.0f);
    m = glm::translate(m, entityPos);
    m = glm::rotate(m, yawRad, glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::translate(m, offset + pivot);
    if (std::abs(rotAngle) > 0.001f) {
        m = glm::rotate(m, rotAngle, rotAxis);
    }
    m = glm::translate(m, -pivot);
    m = glm::scale(m, size);
    return m;
}

void EntityManager::renderCow(const Entity& e, const glm::mat4& vp) {
    float flash = e.getDamageFlash();
    float walk = e.getWalkTime();
    float legSwing = std::sin(walk) * 0.6f;

    glm::vec3 pos = e.getPosition();
    float yaw = e.getYaw();

    // Colors
    glm::vec3 brown(0.44f, 0.31f, 0.22f);
    glm::vec3 lightBrown(0.55f, 0.42f, 0.30f);
    glm::vec3 darkBrown(0.30f, 0.20f, 0.12f);
    glm::vec3 white(0.90f, 0.88f, 0.85f);

    // Body: wide rectangle
    float bW = 0.5f, bH = 0.5f, bD = 0.9f;
    float legH = 0.5f;
    glm::mat4 body = partModel(pos, yaw, glm::vec3(-bW*0.5f, legH, -bD*0.5f),
                               glm::vec3(bW, bH, bD));
    drawCubeHelper(m_shader, vp, body, brown, flash);

    // White spots (slightly larger overlay on top of body)
    glm::mat4 spots = partModel(pos, yaw, glm::vec3(-bW*0.3f, legH+0.05f, -bD*0.2f),
                                glm::vec3(bW*0.4f, bH*0.6f, bD*0.3f));
    drawCubeHelper(m_shader, vp, spots, white, flash);

    // Head
    float headS = 0.35f;
    glm::mat4 head = partModel(pos, yaw,
        glm::vec3(-headS*0.5f, legH + bH*0.3f, -bD*0.5f - headS*0.8f),
        glm::vec3(headS, headS, headS*0.9f));
    drawCubeHelper(m_shader, vp, head, darkBrown, flash);

    // Legs (4)
    float lW = 0.15f, lD = 0.15f;
    // Front-left
    glm::mat4 fl = partModel(pos, yaw, glm::vec3(-bW*0.35f, 0.0f, -bD*0.35f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), legSwing);
    drawCubeHelper(m_shader, vp, fl, lightBrown, flash);
    // Front-right
    glm::mat4 fr = partModel(pos, yaw, glm::vec3(bW*0.2f, 0.0f, -bD*0.35f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), -legSwing);
    drawCubeHelper(m_shader, vp, fr, lightBrown, flash);
    // Back-left
    glm::mat4 bl = partModel(pos, yaw, glm::vec3(-bW*0.35f, 0.0f, bD*0.2f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), -legSwing);
    drawCubeHelper(m_shader, vp, bl, lightBrown, flash);
    // Back-right
    glm::mat4 br = partModel(pos, yaw, glm::vec3(bW*0.2f, 0.0f, bD*0.2f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), legSwing);
    drawCubeHelper(m_shader, vp, br, lightBrown, flash);
}

void EntityManager::renderPig(const Entity& e, const glm::mat4& vp) {
    float flash = e.getDamageFlash();
    float walk = e.getWalkTime();
    float legSwing = std::sin(walk) * 0.5f;

    glm::vec3 pos = e.getPosition();
    float yaw = e.getYaw();

    glm::vec3 pink(0.90f, 0.60f, 0.55f);
    glm::vec3 darkPink(0.80f, 0.45f, 0.40f);
    glm::vec3 snout(0.85f, 0.50f, 0.45f);

    // Body
    float bW = 0.45f, bH = 0.4f, bD = 0.7f;
    float legH = 0.3f;
    glm::mat4 body = partModel(pos, yaw, glm::vec3(-bW*0.5f, legH, -bD*0.5f),
                               glm::vec3(bW, bH, bD));
    drawCubeHelper(m_shader, vp, body, pink, flash);

    // Head
    float headS = 0.3f;
    glm::mat4 head = partModel(pos, yaw,
        glm::vec3(-headS*0.5f, legH + bH*0.1f, -bD*0.5f - headS*0.7f),
        glm::vec3(headS, headS, headS*0.8f));
    drawCubeHelper(m_shader, vp, head, pink, flash);

    // Snout
    glm::mat4 sn = partModel(pos, yaw,
        glm::vec3(-0.08f, legH + bH*0.1f, -bD*0.5f - headS*0.7f - 0.08f),
        glm::vec3(0.16f, 0.12f, 0.08f));
    drawCubeHelper(m_shader, vp, sn, snout, flash);

    // Legs (4)
    float lW = 0.12f, lD = 0.12f;
    glm::mat4 fl = partModel(pos, yaw, glm::vec3(-bW*0.3f, 0.0f, -bD*0.3f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), legSwing);
    drawCubeHelper(m_shader, vp, fl, darkPink, flash);

    glm::mat4 fr = partModel(pos, yaw, glm::vec3(bW*0.15f, 0.0f, -bD*0.3f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), -legSwing);
    drawCubeHelper(m_shader, vp, fr, darkPink, flash);

    glm::mat4 bl = partModel(pos, yaw, glm::vec3(-bW*0.3f, 0.0f, bD*0.15f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), -legSwing);
    drawCubeHelper(m_shader, vp, bl, darkPink, flash);

    glm::mat4 br = partModel(pos, yaw, glm::vec3(bW*0.15f, 0.0f, bD*0.15f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), legSwing);
    drawCubeHelper(m_shader, vp, br, darkPink, flash);
}

void EntityManager::renderSheep(const Entity& e, const glm::mat4& vp) {
    float flash = e.getDamageFlash();
    float walk = e.getWalkTime();
    float legSwing = std::sin(walk) * 0.5f;

    glm::vec3 pos = e.getPosition();
    float yaw = e.getYaw();

    glm::vec3 wool(0.90f, 0.90f, 0.88f);
    glm::vec3 face(0.60f, 0.55f, 0.50f);
    glm::vec3 leg(0.55f, 0.50f, 0.45f);

    // Body (fluffy - slightly larger)
    float bW = 0.55f, bH = 0.5f, bD = 0.8f;
    float legH = 0.45f;
    glm::mat4 body = partModel(pos, yaw, glm::vec3(-bW*0.5f, legH, -bD*0.5f),
                               glm::vec3(bW, bH, bD));
    drawCubeHelper(m_shader, vp, body, wool, flash);

    // Fluffy wool top layer
    glm::mat4 woolTop = partModel(pos, yaw,
        glm::vec3(-bW*0.55f, legH - 0.02f, -bD*0.55f),
        glm::vec3(bW*1.1f, bH*1.05f, bD*1.1f));
    drawCubeHelper(m_shader, vp, woolTop, glm::vec3(0.95f, 0.95f, 0.93f), flash);

    // Head
    float headS = 0.28f;
    glm::mat4 head = partModel(pos, yaw,
        glm::vec3(-headS*0.5f, legH + bH*0.2f, -bD*0.5f - headS*0.6f),
        glm::vec3(headS, headS, headS*0.8f));
    drawCubeHelper(m_shader, vp, head, face, flash);

    // Legs (4)
    float lW = 0.1f, lD = 0.1f;
    glm::mat4 fl = partModel(pos, yaw, glm::vec3(-bW*0.3f, 0.0f, -bD*0.3f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), legSwing);
    drawCubeHelper(m_shader, vp, fl, leg, flash);

    glm::mat4 fr = partModel(pos, yaw, glm::vec3(bW*0.15f, 0.0f, -bD*0.3f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), -legSwing);
    drawCubeHelper(m_shader, vp, fr, leg, flash);

    glm::mat4 bl = partModel(pos, yaw, glm::vec3(-bW*0.3f, 0.0f, bD*0.15f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), -legSwing);
    drawCubeHelper(m_shader, vp, bl, leg, flash);

    glm::mat4 br = partModel(pos, yaw, glm::vec3(bW*0.15f, 0.0f, bD*0.15f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), legSwing);
    drawCubeHelper(m_shader, vp, br, leg, flash);
}

void EntityManager::renderChicken(const Entity& e, const glm::mat4& vp) {
    float flash = e.getDamageFlash();
    float walk = e.getWalkTime();
    float legSwing = std::sin(walk) * 0.6f;

    glm::vec3 pos = e.getPosition();
    float yaw = e.getYaw();

    glm::vec3 white(0.95f, 0.95f, 0.93f);
    glm::vec3 yellow(0.95f, 0.85f, 0.25f);
    glm::vec3 red(0.85f, 0.15f, 0.10f);
    glm::vec3 beak(0.95f, 0.70f, 0.20f);

    // Body (small, round)
    float bW = 0.25f, bH = 0.25f, bD = 0.35f;
    float legH = 0.2f;
    glm::mat4 body = partModel(pos, yaw, glm::vec3(-bW*0.5f, legH, -bD*0.5f),
                               glm::vec3(bW, bH, bD));
    drawCubeHelper(m_shader, vp, body, white, flash);

    // Head
    float headS = 0.15f;
    glm::mat4 head = partModel(pos, yaw,
        glm::vec3(-headS*0.5f, legH + bH*0.6f, -bD*0.5f - headS*0.4f),
        glm::vec3(headS, headS, headS));
    drawCubeHelper(m_shader, vp, head, white, flash);

    // Comb (red on top of head)
    glm::mat4 comb = partModel(pos, yaw,
        glm::vec3(-0.03f, legH + bH*0.6f + headS, -bD*0.5f - headS*0.2f),
        glm::vec3(0.06f, 0.06f, 0.04f));
    drawCubeHelper(m_shader, vp, comb, red, flash);

    // Beak
    glm::mat4 beakM = partModel(pos, yaw,
        glm::vec3(-0.03f, legH + bH*0.6f + headS*0.2f, -bD*0.5f - headS*0.4f - 0.06f),
        glm::vec3(0.06f, 0.04f, 0.06f));
    drawCubeHelper(m_shader, vp, beakM, beak, flash);

    // Legs (2, thin yellow)
    float lW = 0.04f, lD = 0.04f;
    glm::mat4 ll = partModel(pos, yaw, glm::vec3(-bW*0.2f, 0.0f, 0.0f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), legSwing);
    drawCubeHelper(m_shader, vp, ll, yellow, flash);

    glm::mat4 rl = partModel(pos, yaw, glm::vec3(bW*0.1f, 0.0f, 0.0f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), -legSwing);
    drawCubeHelper(m_shader, vp, rl, yellow, flash);
}

void EntityManager::renderZombie(const Entity& e, const glm::mat4& vp) {
    float flash = e.getDamageFlash();
    float walk = e.getWalkTime();
    float legSwing = std::sin(walk) * 0.7f;
    float armSwing = std::sin(walk) * 0.5f;

    glm::vec3 pos = e.getPosition();
    float yaw = e.getYaw();

    glm::vec3 skin(0.30f, 0.55f, 0.30f);
    glm::vec3 shirt(0.18f, 0.35f, 0.50f);
    glm::vec3 pants(0.20f, 0.15f, 0.30f);

    float legH = 0.7f;
    float bodyH = 0.6f;
    float bodyW = 0.35f, bodyD = 0.2f;
    float headS = 0.3f;

    // Body (upper = shirt color)
    glm::mat4 body = partModel(pos, yaw,
        glm::vec3(-bodyW*0.5f, legH, -bodyD*0.5f),
        glm::vec3(bodyW, bodyH, bodyD));
    drawCubeHelper(m_shader, vp, body, shirt, flash);

    // Head
    glm::mat4 head = partModel(pos, yaw,
        glm::vec3(-headS*0.5f, legH + bodyH, -headS*0.5f),
        glm::vec3(headS, headS, headS));
    drawCubeHelper(m_shader, vp, head, skin, flash);

    // Arms (stretched forward, zombie style)
    float armW = 0.12f, armH = 0.6f, armD = 0.12f;
    // Left arm - stretched forward
    glm::mat4 la = partModel(pos, yaw,
        glm::vec3(-bodyW*0.5f - armW, legH + bodyH*0.3f, -bodyD*0.5f - armH*0.5f),
        glm::vec3(armW, armW, armH),
        glm::vec3(armW*0.5f, armW*0.5f, 0.0f), -0.3f + armSwing);
    drawCubeHelper(m_shader, vp, la, skin, flash);

    // Right arm - stretched forward
    glm::mat4 ra = partModel(pos, yaw,
        glm::vec3(bodyW*0.5f, legH + bodyH*0.3f, -bodyD*0.5f - armH*0.5f),
        glm::vec3(armW, armW, armH),
        glm::vec3(armW*0.5f, armW*0.5f, 0.0f), -0.3f - armSwing);
    drawCubeHelper(m_shader, vp, ra, skin, flash);

    // Legs
    float lW = 0.15f, lD = 0.15f;
    glm::mat4 ll = partModel(pos, yaw, glm::vec3(-bodyW*0.35f, 0.0f, -lD*0.5f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), legSwing);
    drawCubeHelper(m_shader, vp, ll, pants, flash);

    glm::mat4 rl = partModel(pos, yaw, glm::vec3(bodyW*0.2f, 0.0f, -lD*0.5f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), -legSwing);
    drawCubeHelper(m_shader, vp, rl, pants, flash);
}

void EntityManager::renderSkeleton(const Entity& e, const glm::mat4& vp) {
    float flash = e.getDamageFlash();
    float walk = e.getWalkTime();
    float legSwing = std::sin(walk) * 0.6f;
    float armSwing = std::sin(walk) * 0.4f;

    glm::vec3 pos = e.getPosition();
    float yaw = e.getYaw();

    glm::vec3 bone(0.85f, 0.83f, 0.78f);
    glm::vec3 darkBone(0.70f, 0.68f, 0.63f);

    float legH = 0.7f;
    float bodyH = 0.6f;
    float bodyW = 0.3f, bodyD = 0.12f; // Very thin
    float headS = 0.3f;

    // Body (thin ribcage)
    glm::mat4 body = partModel(pos, yaw,
        glm::vec3(-bodyW*0.5f, legH, -bodyD*0.5f),
        glm::vec3(bodyW, bodyH, bodyD));
    drawCubeHelper(m_shader, vp, body, bone, flash);

    // Head (skull)
    glm::mat4 head = partModel(pos, yaw,
        glm::vec3(-headS*0.5f, legH + bodyH, -headS*0.5f),
        glm::vec3(headS, headS, headS));
    drawCubeHelper(m_shader, vp, head, bone, flash);

    // Arms (thin, hanging with swing)
    float armW = 0.08f, armH = 0.55f, armD = 0.08f;
    glm::mat4 la = partModel(pos, yaw,
        glm::vec3(-bodyW*0.5f - armW, legH + bodyH - armH*0.1f, -armD*0.5f),
        glm::vec3(armW, armH, armD),
        glm::vec3(armW*0.5f, armH, armD*0.5f), armSwing);
    drawCubeHelper(m_shader, vp, la, darkBone, flash);

    glm::mat4 ra = partModel(pos, yaw,
        glm::vec3(bodyW*0.5f, legH + bodyH - armH*0.1f, -armD*0.5f),
        glm::vec3(armW, armH, armD),
        glm::vec3(armW*0.5f, armH, armD*0.5f), -armSwing);
    drawCubeHelper(m_shader, vp, ra, darkBone, flash);

    // Legs (thin)
    float lW = 0.08f, lD = 0.08f;
    glm::mat4 ll = partModel(pos, yaw, glm::vec3(-bodyW*0.3f, 0.0f, -lD*0.5f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), legSwing);
    drawCubeHelper(m_shader, vp, ll, darkBone, flash);

    glm::mat4 rl = partModel(pos, yaw, glm::vec3(bodyW*0.15f, 0.0f, -lD*0.5f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), -legSwing);
    drawCubeHelper(m_shader, vp, rl, darkBone, flash);
}

void EntityManager::renderCreeper(const Entity& e, const glm::mat4& vp) {
    float flash = e.getDamageFlash();
    float walk = e.getWalkTime();
    float legSwing = std::sin(walk) * 0.4f;
    float fuse = e.getCreeperFuse();

    glm::vec3 pos = e.getPosition();
    float yaw = e.getYaw();

    // Creeper flashes white when fusing
    glm::vec3 green(0.30f, 0.70f, 0.30f);
    glm::vec3 darkGreen(0.20f, 0.55f, 0.20f);
    if (fuse > 0.0f) {
        // Flash between green and white
        float flashIntensity = std::sin(fuse * 12.0f) * 0.5f + 0.5f;
        green = glm::mix(green, glm::vec3(1.0f), flashIntensity * 0.7f);
        darkGreen = glm::mix(darkGreen, glm::vec3(1.0f), flashIntensity * 0.7f);
    }

    float legH = 0.4f;
    float bodyH = 0.7f;
    float bodyW = 0.35f, bodyD = 0.2f;
    float headS = 0.3f;

    // Body (tall, no arms)
    glm::mat4 body = partModel(pos, yaw,
        glm::vec3(-bodyW*0.5f, legH, -bodyD*0.5f),
        glm::vec3(bodyW, bodyH, bodyD));
    drawCubeHelper(m_shader, vp, body, green, flash);

    // Dark spots on body
    glm::mat4 spot1 = partModel(pos, yaw,
        glm::vec3(-bodyW*0.2f, legH + 0.15f, -bodyD*0.5f - 0.01f),
        glm::vec3(bodyW*0.3f, 0.2f, 0.02f));
    drawCubeHelper(m_shader, vp, spot1, darkGreen, flash);

    // Head
    glm::mat4 head = partModel(pos, yaw,
        glm::vec3(-headS*0.5f, legH + bodyH, -headS*0.5f),
        glm::vec3(headS, headS, headS));
    drawCubeHelper(m_shader, vp, head, green, flash);

    // 4 short legs, no arms
    float lW = 0.12f, lD = 0.12f;
    glm::mat4 fl = partModel(pos, yaw, glm::vec3(-bodyW*0.35f, 0.0f, -bodyD*0.3f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), legSwing);
    drawCubeHelper(m_shader, vp, fl, darkGreen, flash);

    glm::mat4 fr = partModel(pos, yaw, glm::vec3(bodyW*0.2f, 0.0f, -bodyD*0.3f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), -legSwing);
    drawCubeHelper(m_shader, vp, fr, darkGreen, flash);

    glm::mat4 bl = partModel(pos, yaw, glm::vec3(-bodyW*0.35f, 0.0f, bodyD*0.15f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), -legSwing);
    drawCubeHelper(m_shader, vp, bl, darkGreen, flash);

    glm::mat4 br = partModel(pos, yaw, glm::vec3(bodyW*0.2f, 0.0f, bodyD*0.15f),
                             glm::vec3(lW, legH, lD),
                             glm::vec3(lW*0.5f, legH, lD*0.5f), legSwing);
    drawCubeHelper(m_shader, vp, br, darkGreen, flash);
}

void EntityManager::renderSpider(const Entity& e, const glm::mat4& vp) {
    float flash = e.getDamageFlash();
    float walk = e.getWalkTime();

    glm::vec3 pos = e.getPosition();
    float yaw = e.getYaw();

    glm::vec3 dark(0.25f, 0.18f, 0.15f);
    glm::vec3 eye(0.80f, 0.10f, 0.10f);
    glm::vec3 legCol(0.20f, 0.14f, 0.10f);

    // Body (wide, flat)
    float bW = 0.6f, bH = 0.3f, bD = 0.8f;
    float baseY = 0.15f;
    glm::mat4 body = partModel(pos, yaw, glm::vec3(-bW*0.5f, baseY, -bD*0.3f),
                               glm::vec3(bW, bH, bD*0.6f));
    drawCubeHelper(m_shader, vp, body, dark, flash);

    // Abdomen (rear, larger)
    glm::mat4 abd = partModel(pos, yaw, glm::vec3(-bW*0.4f, baseY - 0.02f, bD*0.1f),
                              glm::vec3(bW*0.8f, bH*1.1f, bD*0.45f));
    drawCubeHelper(m_shader, vp, abd, dark, flash);

    // Head (front, with red eyes)
    float headS = 0.25f;
    glm::mat4 head = partModel(pos, yaw,
        glm::vec3(-headS*0.5f, baseY, -bD*0.3f - headS*0.7f),
        glm::vec3(headS, headS*0.9f, headS*0.8f));
    drawCubeHelper(m_shader, vp, head, dark, flash);

    // Eyes (two small red cubes)
    glm::mat4 eyeL = partModel(pos, yaw,
        glm::vec3(-headS*0.35f, baseY + headS*0.4f, -bD*0.3f - headS*0.7f - 0.02f),
        glm::vec3(0.06f, 0.06f, 0.03f));
    drawCubeHelper(m_shader, vp, eyeL, eye, flash);

    glm::mat4 eyeR = partModel(pos, yaw,
        glm::vec3(headS*0.15f, baseY + headS*0.4f, -bD*0.3f - headS*0.7f - 0.02f),
        glm::vec3(0.06f, 0.06f, 0.03f));
    drawCubeHelper(m_shader, vp, eyeR, eye, flash);

    // 8 legs (4 per side), thin, angled outward
    float legW = 0.05f, legH = 0.35f;
    for (int side = 0; side < 2; ++side) {
        float xSign = (side == 0) ? -1.0f : 1.0f;
        for (int i = 0; i < 4; ++i) {
            float zOff = -bD*0.2f + static_cast<float>(i) * bD * 0.2f;
            float phase = walk + static_cast<float>(i) * 1.57f + static_cast<float>(side) * 0.78f;
            float swing = std::sin(phase) * 0.3f;

            // Leg extends outward and down
            glm::mat4 leg = partModel(pos, yaw,
                glm::vec3(xSign * bW * 0.5f, baseY*0.5f, zOff),
                glm::vec3(legH, legW, legW),
                glm::vec3(0.0f, legW*0.5f, legW*0.5f),
                swing + xSign * 0.8f,  // Angle outward
                glm::vec3(0.0f, 0.0f, 1.0f));
            drawCubeHelper(m_shader, vp, leg, legCol, flash);
        }
    }
}

// ---- Main render dispatch ----

void EntityManager::renderEntity(const Entity& e, const glm::mat4& vp) {
    switch (e.getType()) {
        case MobType::Cow:      renderCow(e, vp);      break;
        case MobType::Pig:      renderPig(e, vp);      break;
        case MobType::Sheep:    renderSheep(e, vp);    break;
        case MobType::Chicken:  renderChicken(e, vp);  break;
        case MobType::Zombie:   renderZombie(e, vp);   break;
        case MobType::Skeleton: renderSkeleton(e, vp); break;
        case MobType::Creeper:  renderCreeper(e, vp);  break;
        case MobType::Spider:   renderSpider(e, vp);   break;
        default: break;
    }
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
