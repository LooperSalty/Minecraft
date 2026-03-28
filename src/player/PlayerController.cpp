#include "PlayerController.h"
#include "../world/Block.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

namespace voxelforge {

// ── Hotbar layout ──────────────────────────────────────────────────

static constexpr BlockType HOTBAR[] = {
    BlockType::Stone, BlockType::Dirt, BlockType::GrassBlock,
    BlockType::Cobblestone, BlockType::OakLog, BlockType::OakPlanks,
    BlockType::Sand, BlockType::OakLeaves, BlockType::Bedrock
};

static constexpr int HOTBAR_SIZE = 9;

// ── Construction ───────────────────────────────────────────────────

PlayerController::PlayerController(const glm::vec3& spawnPos)
    : m_position(spawnPos)
{}

// ── Hotbar ─────────────────────────────────────────────────────────

BlockType PlayerController::getSelectedBlock() const {
    return HOTBAR[m_selectedSlot];
}

void PlayerController::scrollHotbar(float delta) {
    if (delta == 0.0f) return;
    int d = (delta > 0.0f) ? 1 : -1;
    m_selectedSlot = ((m_selectedSlot + d) % HOTBAR_SIZE + HOTBAR_SIZE) % HOTBAR_SIZE;
}

// ── Helpers ────────────────────────────────────────────────────────

glm::vec3 PlayerController::getEyePosition() const {
    return m_position + glm::vec3(0.0f, EYE_HEIGHT, 0.0f);
}

glm::vec3 PlayerController::getFront() const {
    float yawRad   = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);
    return glm::normalize(glm::vec3(
        std::cos(yawRad) * std::cos(pitchRad),
        std::sin(pitchRad),
        std::sin(yawRad) * std::cos(pitchRad)));
}

// ── Mouse look ─────────────────────────────────────────────────────

void PlayerController::handleMouseLook(const glm::vec2& delta) {
    constexpr float SENSITIVITY = 0.1f;
    m_yaw   += delta.x * SENSITIVITY;
    m_pitch += delta.y * SENSITIVITY;
    m_pitch  = std::clamp(m_pitch, -89.9f, 89.9f);
}

// ── Movement ───────────────────────────────────────────────────────

void PlayerController::handleMovement(float dt, const InputState& in) {
    float yawRad = glm::radians(m_yaw);
    glm::vec3 flatFront = glm::normalize(glm::vec3(std::cos(yawRad), 0.0f, std::sin(yawRad)));
    glm::vec3 flatRight = glm::normalize(glm::cross(flatFront, glm::vec3(0.0f, 1.0f, 0.0f)));

    float speed = WALK_SPEED;
    if (in.sprint) speed = SPRINT_SPEED;
    if (in.sneak)  speed = SNEAK_SPEED;

    glm::vec3 dir(0.0f);
    if (in.forward)  dir += flatFront;
    if (in.backward) dir -= flatFront;
    if (in.right)    dir += flatRight;
    if (in.left)     dir -= flatRight;

    if (m_creative) {
        speed = FLY_SPEED;
        if (in.jump)  dir.y += 1.0f;
        if (in.sneak) dir.y -= 1.0f;

        if (glm::length(dir) > 0.0f) {
            dir = glm::normalize(dir);
        }
        m_velocity = dir * speed;
    } else {
        if (glm::length(glm::vec2(dir.x, dir.z)) > 0.0f) {
            glm::vec3 hDir = glm::normalize(glm::vec3(dir.x, 0.0f, dir.z));
            m_velocity.x = hDir.x * speed;
            m_velocity.z = hDir.z * speed;
        } else {
            m_velocity.x = 0.0f;
            m_velocity.z = 0.0f;
        }
    }
}

// ── Collision ──────────────────────────────────────────────────────

bool PlayerController::collidesAt(const glm::vec3& pos, BlockAccessor getBlock) const {
    constexpr float HALF_W = WIDTH / 2.0f;

    int minX = static_cast<int>(std::floor(pos.x - HALF_W));
    int maxX = static_cast<int>(std::floor(pos.x + HALF_W));
    int minY = static_cast<int>(std::floor(pos.y));
    int maxY = static_cast<int>(std::floor(pos.y + HEIGHT));
    int minZ = static_cast<int>(std::floor(pos.z - HALF_W));
    int maxZ = static_cast<int>(std::floor(pos.z + HALF_W));

    for (int bx = minX; bx <= maxX; ++bx) {
        for (int by = minY; by <= maxY; ++by) {
            for (int bz = minZ; bz <= maxZ; ++bz) {
                if (isBlockSolid(getBlock(bx, by, bz))) {
                    return true;
                }
            }
        }
    }
    return false;
}

// ── Fall damage ────────────────────────────────────────────────────

void PlayerController::applyFallDamage(float fallDist) {
    if (fallDist > 3.0f) {
        m_health -= (fallDist - 3.0f);
        m_health  = std::max(m_health, 0.0f);
    }
}

// ── Physics (survival only) ────────────────────────────────────────

void PlayerController::applyPhysics(float dt, BlockAccessor getBlock) {
    // Gravity
    m_velocity.y -= GRAVITY * dt;
    m_velocity.y  = std::max(m_velocity.y, -TERMINAL_VEL);

    glm::vec3 newPos = m_position;

    // Y axis first
    newPos.y = m_position.y + m_velocity.y * dt;
    if (collidesAt(newPos, getBlock)) {
        if (m_velocity.y < 0.0f) {
            m_onGround = true;
            // Landing: apply fall damage
            if (m_wasFalling) {
                float fallDist = m_fallStart - m_position.y;
                applyFallDamage(fallDist);
                m_wasFalling = false;
            }
        }
        newPos.y = m_position.y;
        m_velocity.y = 0.0f;
    } else {
        if (!m_onGround && !m_wasFalling) {
            m_wasFalling = true;
            m_fallStart  = m_position.y;
        }
        m_onGround = false;
    }
    m_position.y = newPos.y;

    // X axis
    newPos.x = m_position.x + m_velocity.x * dt;
    glm::vec3 testX(newPos.x, m_position.y, m_position.z);
    if (collidesAt(testX, getBlock)) {
        newPos.x = m_position.x;
        m_velocity.x = 0.0f;
    }
    m_position.x = newPos.x;

    // Z axis
    newPos.z = m_position.z + m_velocity.z * dt;
    glm::vec3 testZ(m_position.x, m_position.y, newPos.z);
    if (collidesAt(testZ, getBlock)) {
        newPos.z = m_position.z;
        m_velocity.z = 0.0f;
    }
    m_position.z = newPos.z;
}

// ── Raycasting (DDA) ───────────────────────────────────────────────

PlayerController::BlockHit PlayerController::raycast(BlockAccessor getBlock) const {
    glm::vec3 origin = getEyePosition();
    glm::vec3 dir    = getFront();

    glm::ivec3 blockPos(
        static_cast<int>(std::floor(origin.x)),
        static_cast<int>(std::floor(origin.y)),
        static_cast<int>(std::floor(origin.z)));

    glm::vec3 deltaDist(
        (dir.x != 0.0f) ? std::abs(1.0f / dir.x) : 1e30f,
        (dir.y != 0.0f) ? std::abs(1.0f / dir.y) : 1e30f,
        (dir.z != 0.0f) ? std::abs(1.0f / dir.z) : 1e30f);

    glm::ivec3 step;
    glm::vec3  tMax;

    if (dir.x >= 0.0f) {
        step.x = 1;
        tMax.x = (static_cast<float>(blockPos.x + 1) - origin.x) * deltaDist.x;
    } else {
        step.x = -1;
        tMax.x = (origin.x - static_cast<float>(blockPos.x)) * deltaDist.x;
    }
    if (dir.y >= 0.0f) {
        step.y = 1;
        tMax.y = (static_cast<float>(blockPos.y + 1) - origin.y) * deltaDist.y;
    } else {
        step.y = -1;
        tMax.y = (origin.y - static_cast<float>(blockPos.y)) * deltaDist.y;
    }
    if (dir.z >= 0.0f) {
        step.z = 1;
        tMax.z = (static_cast<float>(blockPos.z + 1) - origin.z) * deltaDist.z;
    } else {
        step.z = -1;
        tMax.z = (origin.z - static_cast<float>(blockPos.z)) * deltaDist.z;
    }

    glm::ivec3 prevPos = blockPos;

    for (float dist = 0.0f; dist < REACH;) {
        prevPos = blockPos;

        if (tMax.x < tMax.y) {
            if (tMax.x < tMax.z) {
                dist = tMax.x;
                blockPos.x += step.x;
                tMax.x += deltaDist.x;
            } else {
                dist = tMax.z;
                blockPos.z += step.z;
                tMax.z += deltaDist.z;
            }
        } else {
            if (tMax.y < tMax.z) {
                dist = tMax.y;
                blockPos.y += step.y;
                tMax.y += deltaDist.y;
            } else {
                dist = tMax.z;
                blockPos.z += step.z;
                tMax.z += deltaDist.z;
            }
        }

        if (dist > REACH) break;

        BlockType bt = getBlock(blockPos.x, blockPos.y, blockPos.z);
        if (bt != BlockType::Air && bt != BlockType::Water) {
            BlockHit result;
            result.hit      = true;
            result.blockPos = blockPos;
            result.placePos = prevPos;
            return result;
        }
    }

    return {};
}

// ── Block interaction ──────────────────────────────────────────────

void PlayerController::breakBlock(BlockSetter setBlock, BlockAccessor getBlock) {
    BlockHit hit = raycast(getBlock);
    if (hit.hit) {
        setBlock(hit.blockPos.x, hit.blockPos.y, hit.blockPos.z, BlockType::Air);
    }
}

void PlayerController::placeBlock(BlockSetter setBlock, BlockAccessor getBlock) {
    BlockHit hit = raycast(getBlock);
    if (!hit.hit) return;

    // Don't place if block would overlap the player
    if (collidesAt(m_position, [&](int x, int y, int z) -> BlockType {
        if (x == hit.placePos.x && y == hit.placePos.y && z == hit.placePos.z) {
            return getSelectedBlock();
        }
        return getBlock(x, y, z);
    })) {
        return;
    }

    setBlock(hit.placePos.x, hit.placePos.y, hit.placePos.z, getSelectedBlock());
}

// ── Per-frame update ───────────────────────────────────────────────

void PlayerController::update(float dt, const InputState& input, BlockAccessor getBlock) {
    handleMouseLook(input.mouseDelta);
    m_creative = input.creative;
    handleMovement(dt, input);

    if (m_creative) {
        m_position += m_velocity * dt;
        m_onGround = false;
        m_wasFalling = false;
    } else {
        // Jump
        if (input.jump && m_onGround) {
            m_velocity.y = JUMP_VEL;
            m_onGround   = false;
            m_wasFalling = true;
            m_fallStart  = m_position.y;
        }
        applyPhysics(dt, getBlock);
    }

    scrollHotbar(input.scrollDelta);

    // Rising-edge tracking for block interaction
    // Actual break/place is driven by the caller via breakBlock()/placeBlock()
    m_wasBreaking = input.breakBlock;
    m_wasPlacing  = input.placeBlock;
}

} // namespace voxelforge
