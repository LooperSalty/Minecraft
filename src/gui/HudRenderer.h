#pragma once
#include "TextRenderer.h"
#include "../player/PlayerController.h"
#include "../player/Inventory.h"
#include "../world/Block.h"
#include <glm/glm.hpp>

namespace voxelforge {

class HudRenderer {
public:
    void init(TextRenderer* textRenderer);

    void drawPlayingHUD(const PlayerController& player, int fps,
                        int loadedChunks, int mobCount, bool creativeMode,
                        int screenW, int screenH);

    void drawInventoryScreen(const Inventory& inventory,
                             int screenW, int screenH,
                             const glm::vec2& mousePos, bool mouseClick,
                             int& clickedSlot);

    // Get approximate block color for HUD display
    static glm::vec4 getBlockColor(BlockType type);

private:
    void drawHealthBar(float health, float maxHealth, int screenW, int screenH);
    void drawHungerBar(float hunger, float maxHunger, int screenW, int screenH);
    void drawHotbar(const Inventory& inventory, int screenW, int screenH);
    void drawDebugInfo(const PlayerController& player, int fps,
                       int loadedChunks, int mobCount, int screenW, int screenH);

    void drawSlot(float x, float y, float size, const ItemStack& item,
                  bool selected, int screenW, int screenH);

    TextRenderer* m_text = nullptr;

    static constexpr float SLOT_SIZE = 40.0f;
    static constexpr float SLOT_GAP  = 2.0f;
    static constexpr float HEART_SIZE = 8.0f;
    static constexpr float HEART_GAP  = 2.0f;
};

} // namespace voxelforge
