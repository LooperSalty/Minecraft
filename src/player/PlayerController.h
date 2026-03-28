#pragma once
#include "../world/Block.h"
#include "Inventory.h"
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

    // Inventory access
    Inventory&       getInventory()       { return m_inventory; }
    const Inventory& getInventory() const { return m_inventory; }

    // Hotbar (delegated to inventory)
    void scrollHotbar(float delta);
    void setSelectedSlot(int slot) { m_inventory.setSelectedSlot(slot); }
    int  getSelectedSlot() const { return m_inventory.getSelectedSlot(); }
    BlockType getSelectedBlock() const;

    // Accessors
    glm::vec3 getEyePosition() const;
    glm::vec3 getPosition() const { return m_position; }
    float getYaw() const   { return m_yaw; }
    float getPitch() const { return m_pitch; }
    bool  isOnGround() const { return m_onGround; }
    bool  isSprinting() const { return m_sprinting; }
    float getHealth() const  { return m_health; }
    float getMaxHealth() const { return 20.0f; }
    float getHunger() const  { return m_hunger; }
    float getMaxHunger() const { return 20.0f; }
    glm::vec3 getFront() const;

    // External damage / respawn
    void takeDamage(float amount) { m_health = std::max(0.0f, m_health - amount); }
    void resetHealth()            { m_health = 20.0f; m_hunger = 20.0f; }
    void setPosition(const glm::vec3& p) { m_position = p; m_velocity = glm::vec3(0.0f); }

private:
    void handleMouseLook(const glm::vec2& delta);
    void handleMovement(float dt, const InputState& in);
    void applyPhysics(float dt, BlockAccessor getBlock);
    bool collidesAt(const glm::vec3& pos, BlockAccessor getBlock) const;
    void applyFallDamage(float fallDist);
    void updateHunger(float dt, const InputState& in);

    glm::vec3 m_position;
    glm::vec3 m_velocity{0.0f};
    float m_yaw   = -90.0f;
    float m_pitch = 0.0f;
    bool  m_onGround = false;
    float m_fallStart = 0.0f;
    bool  m_wasFalling = false;
    float m_health = 20.0f;
    float m_hunger = 20.0f;
    bool  m_creative = false;
    bool  m_sprinting = false;

    Inventory m_inventory;

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
    static constexpr float GRAVITY     = 32.0f;   // blocks/s^2
    static constexpr float JUMP_VEL    = 8.2f;    // blocks/s
    static constexpr float TERMINAL_VEL= 78.4f;
    static constexpr float REACH       = 5.0f;

    // Hunger constants
    static constexpr float HUNGER_SPRINT_DRAIN = 0.1f;
    static constexpr float HUNGER_REGEN_THRESHOLD = 18.0f;
    static constexpr float HEALTH_REGEN_RATE = 0.5f;
    static constexpr float STARVATION_RATE = 0.5f;
};

} // namespace voxelforge
