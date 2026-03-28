#pragma once
#include "../world/Block.h"
#include <glm/glm.hpp>
#include <functional>

namespace voxelforge {

using BlockAccessor = std::function<BlockType(int, int, int)>;
using BlockSetter   = std::function<void(int, int, int, BlockType)>;

class PlayerController {
public:
    explicit PlayerController(const glm::vec3& spawnPos);

    struct InputState {
        bool forward = false, backward = false, left = false, right = false;
        bool jump = false, sprint = false, sneak = false;
        bool breakBlock = false, placeBlock = false;
        bool creative = false;           // fly mode
        float scrollDelta = 0.0f;
        glm::vec2 mouseDelta{0.0f};
    };

    void update(float dt, const InputState& input, BlockAccessor getBlock);

    // Block interaction
    struct BlockHit {
        bool hit = false;
        glm::ivec3 blockPos{0};
        glm::ivec3 placePos{0};
    };
    BlockHit raycast(BlockAccessor getBlock) const;
    void breakBlock(BlockSetter setBlock, BlockAccessor getBlock);
    void placeBlock(BlockSetter setBlock, BlockAccessor getBlock);

    // Hotbar
    void scrollHotbar(float delta);
    int  getSelectedSlot() const { return m_selectedSlot; }
    BlockType getSelectedBlock() const;

    // Accessors
    glm::vec3 getEyePosition() const;
    glm::vec3 getPosition() const { return m_position; }
    float getYaw() const   { return m_yaw; }
    float getPitch() const { return m_pitch; }
    bool  isOnGround() const { return m_onGround; }
    float getHealth() const  { return m_health; }
    float getMaxHealth() const { return 20.0f; }
    glm::vec3 getFront() const;

    // External damage / respawn
    void takeDamage(float amount) { m_health = std::max(0.0f, m_health - amount); }
    void resetHealth()            { m_health = 20.0f; }
    void setPosition(const glm::vec3& p) { m_position = p; m_velocity = glm::vec3(0.0f); }

private:
    void handleMouseLook(const glm::vec2& delta);
    void handleMovement(float dt, const InputState& in);
    void applyPhysics(float dt, BlockAccessor getBlock);
    bool collidesAt(const glm::vec3& pos, BlockAccessor getBlock) const;
    void applyFallDamage(float fallDist);

    glm::vec3 m_position;
    glm::vec3 m_velocity{0.0f};
    float m_yaw   = -90.0f;
    float m_pitch = 0.0f;
    bool  m_onGround = false;
    float m_fallStart = 0.0f;
    bool  m_wasFalling = false;
    float m_health = 20.0f;
    int   m_selectedSlot = 0;
    bool  m_creative = false;

    // Breaking state
    bool m_wasBreaking = false;
    bool m_wasPlacing  = false;

    static constexpr float WIDTH       = 0.6f;
    static constexpr float HEIGHT      = 1.8f;
    static constexpr float EYE_HEIGHT  = 1.62f;
    static constexpr float WALK_SPEED  = 4.317f;
    static constexpr float SPRINT_SPEED= 5.612f;
    static constexpr float SNEAK_SPEED = 1.295f;
    static constexpr float FLY_SPEED   = 10.89f;
    static constexpr float GRAVITY     = 32.0f;   // blocks/s²
    static constexpr float JUMP_VEL    = 8.2f;    // blocks/s
    static constexpr float TERMINAL_VEL= 78.4f;
    static constexpr float REACH       = 5.0f;
};

} // namespace voxelforge
