#include "HudRenderer.h"
#include <cstdio>
#include <cmath>
#include <string>

namespace voxelforge {

void HudRenderer::init(TextRenderer* textRenderer) {
    m_text = textRenderer;
}

// ---------------------------------------------------------------------------
// Block color mapping for inventory display
// ---------------------------------------------------------------------------
glm::vec4 HudRenderer::getBlockColor(BlockType type) {
    switch (type) {
        case BlockType::Stone:        return {0.50f, 0.50f, 0.50f, 1.0f};
        case BlockType::Dirt:         return {0.55f, 0.35f, 0.20f, 1.0f};
        case BlockType::GrassBlock:   return {0.30f, 0.65f, 0.20f, 1.0f};
        case BlockType::Cobblestone:  return {0.45f, 0.45f, 0.45f, 1.0f};
        case BlockType::OakLog:       return {0.55f, 0.40f, 0.25f, 1.0f};
        case BlockType::OakPlanks:    return {0.72f, 0.56f, 0.35f, 1.0f};
        case BlockType::Bedrock:      return {0.20f, 0.20f, 0.20f, 1.0f};
        case BlockType::Sand:         return {0.85f, 0.80f, 0.55f, 1.0f};
        case BlockType::Gravel:       return {0.55f, 0.52f, 0.50f, 1.0f};
        case BlockType::Water:        return {0.20f, 0.40f, 0.80f, 0.7f};
        case BlockType::Lava:         return {0.90f, 0.40f, 0.10f, 1.0f};
        case BlockType::OakLeaves:    return {0.20f, 0.55f, 0.15f, 1.0f};
        case BlockType::BirchLog:     return {0.80f, 0.78f, 0.75f, 1.0f};
        case BlockType::BirchLeaves:  return {0.40f, 0.65f, 0.30f, 1.0f};
        case BlockType::SpruceLog:    return {0.35f, 0.25f, 0.15f, 1.0f};
        case BlockType::SpruceLeaves: return {0.15f, 0.35f, 0.15f, 1.0f};
        case BlockType::CoalOre:      return {0.30f, 0.30f, 0.30f, 1.0f};
        case BlockType::IronOre:      return {0.60f, 0.55f, 0.50f, 1.0f};
        case BlockType::GoldOre:      return {0.75f, 0.65f, 0.30f, 1.0f};
        case BlockType::DiamondOre:   return {0.40f, 0.75f, 0.80f, 1.0f};
        case BlockType::RedstoneOre:  return {0.70f, 0.20f, 0.20f, 1.0f};
        case BlockType::LapisOre:     return {0.20f, 0.30f, 0.70f, 1.0f};
        case BlockType::EmeraldOre:   return {0.20f, 0.70f, 0.30f, 1.0f};
        case BlockType::SnowBlock:    return {0.90f, 0.92f, 0.95f, 1.0f};
        case BlockType::Sandstone:    return {0.80f, 0.75f, 0.55f, 1.0f};
        case BlockType::TallGrass:    return {0.25f, 0.60f, 0.15f, 1.0f};
        case BlockType::Poppy:        return {0.80f, 0.15f, 0.15f, 1.0f};
        case BlockType::Dandelion:    return {0.90f, 0.85f, 0.15f, 1.0f};
        case BlockType::Cactus:       return {0.20f, 0.50f, 0.15f, 1.0f};
        case BlockType::Clay:         return {0.60f, 0.62f, 0.65f, 1.0f};
        default:                      return {0.30f, 0.30f, 0.30f, 0.5f};
    }
}

// ---------------------------------------------------------------------------
// Draw a single inventory slot
// ---------------------------------------------------------------------------
void HudRenderer::drawSlot(float x, float y, float size, const ItemStack& item,
                            bool selected, int screenW, int screenH) {
    // Slot background
    glm::vec4 bgColor = selected
        ? glm::vec4(0.60f, 0.60f, 0.60f, 0.85f)
        : glm::vec4(0.20f, 0.20f, 0.20f, 0.75f);
    m_text->drawRect(x, y, size, size, bgColor, screenW, screenH);

    // Slot border
    float bw = 1.5f;
    glm::vec4 borderColor = selected
        ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
        : glm::vec4(0.40f, 0.40f, 0.40f, 0.9f);
    m_text->drawRect(x, y, size, bw, borderColor, screenW, screenH);           // top
    m_text->drawRect(x, y + size - bw, size, bw, borderColor, screenW, screenH); // bottom
    m_text->drawRect(x, y, bw, size, borderColor, screenW, screenH);           // left
    m_text->drawRect(x + size - bw, y, bw, size, borderColor, screenW, screenH); // right

    // Block color indicator
    if (!item.isEmpty()) {
        float pad = 6.0f;
        float itemSize = size - pad * 2.0f;
        glm::vec4 col = getBlockColor(item.type);
        m_text->drawRect(x + pad, y + pad, itemSize, itemSize, col, screenW, screenH);

        // Count text (bottom-right)
        if (item.count > 1) {
            char countStr[8];
            std::snprintf(countStr, sizeof(countStr), "%d", item.count);
            float textScale = 1.5f;
            float tw = m_text->textWidth(countStr, textScale);
            m_text->drawText(countStr,
                             x + size - tw - 3.0f,
                             y + size - m_text->textHeight(textScale) - 2.0f,
                             textScale, {1.0f, 1.0f, 1.0f, 1.0f}, screenW, screenH);
        }
    }
}

// ---------------------------------------------------------------------------
// Health bar
// ---------------------------------------------------------------------------
void HudRenderer::drawHealthBar(float health, float maxHealth, int screenW, int screenH) {
    float fw = static_cast<float>(screenW);
    float fh = static_cast<float>(screenH);

    float hotbarWidth = Inventory::HOTBAR_SIZE * (SLOT_SIZE + SLOT_GAP) - SLOT_GAP;
    float hotbarX = (fw - hotbarWidth) / 2.0f;
    float hotbarY = fh - SLOT_SIZE - 8.0f;

    // Hearts drawn above hotbar, left side
    float heartsY = hotbarY - HEART_SIZE - 6.0f;
    float heartsX = hotbarX;

    int totalHearts = static_cast<int>(maxHealth / 2.0f);
    int fullHearts  = static_cast<int>(health / 2.0f);
    bool halfHeart  = (static_cast<int>(health) % 2) == 1;

    for (int i = 0; i < totalHearts; ++i) {
        float hx = heartsX + i * (HEART_SIZE + HEART_GAP);

        // Background (dark red outline)
        m_text->drawRect(hx, heartsY, HEART_SIZE, HEART_SIZE,
                         {0.25f, 0.05f, 0.05f, 0.8f}, screenW, screenH);

        if (i < fullHearts) {
            // Full heart
            m_text->drawRect(hx + 1.0f, heartsY + 1.0f,
                             HEART_SIZE - 2.0f, HEART_SIZE - 2.0f,
                             {0.85f, 0.10f, 0.10f, 1.0f}, screenW, screenH);
        } else if (i == fullHearts && halfHeart) {
            // Half heart: left half red, right half dark
            float halfW = (HEART_SIZE - 2.0f) / 2.0f;
            m_text->drawRect(hx + 1.0f, heartsY + 1.0f,
                             halfW, HEART_SIZE - 2.0f,
                             {0.85f, 0.10f, 0.10f, 1.0f}, screenW, screenH);
            m_text->drawRect(hx + 1.0f + halfW, heartsY + 1.0f,
                             halfW, HEART_SIZE - 2.0f,
                             {0.35f, 0.08f, 0.08f, 0.8f}, screenW, screenH);
        }
    }
}

// ---------------------------------------------------------------------------
// Hunger bar
// ---------------------------------------------------------------------------
void HudRenderer::drawHungerBar(float hunger, float maxHunger, int screenW, int screenH) {
    float fw = static_cast<float>(screenW);
    float fh = static_cast<float>(screenH);

    float hotbarWidth = Inventory::HOTBAR_SIZE * (SLOT_SIZE + SLOT_GAP) - SLOT_GAP;
    float hotbarX = (fw - hotbarWidth) / 2.0f;
    float hotbarY = fh - SLOT_SIZE - 8.0f;

    // Hunger icons drawn above hotbar, right side (mirrored from health)
    float heartsY = hotbarY - HEART_SIZE - 6.0f;
    float hungerStartX = hotbarX + hotbarWidth; // right-aligned

    int totalDrumsticks = static_cast<int>(maxHunger / 2.0f);
    int fullDrumsticks  = static_cast<int>(hunger / 2.0f);
    bool halfDrumstick  = (static_cast<int>(hunger) % 2) == 1;

    for (int i = 0; i < totalDrumsticks; ++i) {
        // Draw from right to left (Minecraft style)
        float hx = hungerStartX - (i + 1) * (HEART_SIZE + HEART_GAP);

        // Background
        m_text->drawRect(hx, heartsY, HEART_SIZE, HEART_SIZE,
                         {0.15f, 0.10f, 0.05f, 0.8f}, screenW, screenH);

        if (i < fullDrumsticks) {
            // Full drumstick (orange/brown)
            m_text->drawRect(hx + 1.0f, heartsY + 1.0f,
                             HEART_SIZE - 2.0f, HEART_SIZE - 2.0f,
                             {0.75f, 0.50f, 0.15f, 1.0f}, screenW, screenH);
        } else if (i == fullDrumsticks && halfDrumstick) {
            float halfW = (HEART_SIZE - 2.0f) / 2.0f;
            m_text->drawRect(hx + 1.0f, heartsY + 1.0f,
                             halfW, HEART_SIZE - 2.0f,
                             {0.75f, 0.50f, 0.15f, 1.0f}, screenW, screenH);
            m_text->drawRect(hx + 1.0f + halfW, heartsY + 1.0f,
                             halfW, HEART_SIZE - 2.0f,
                             {0.30f, 0.20f, 0.08f, 0.8f}, screenW, screenH);
        }
    }
}

// ---------------------------------------------------------------------------
// Hotbar
// ---------------------------------------------------------------------------
void HudRenderer::drawHotbar(const Inventory& inventory, int screenW, int screenH) {
    float fw = static_cast<float>(screenW);
    float fh = static_cast<float>(screenH);

    float hotbarWidth = Inventory::HOTBAR_SIZE * (SLOT_SIZE + SLOT_GAP) - SLOT_GAP;
    float hotbarX = (fw - hotbarWidth) / 2.0f;
    float hotbarY = fh - SLOT_SIZE - 8.0f;

    int selected = inventory.getSelectedSlot();

    for (int i = 0; i < Inventory::HOTBAR_SIZE; ++i) {
        float sx = hotbarX + i * (SLOT_SIZE + SLOT_GAP);
        drawSlot(sx, hotbarY, SLOT_SIZE, inventory.getHotbarSlot(i),
                 i == selected, screenW, screenH);
    }

    // Selected slot name below hotbar
    const ItemStack& sel = inventory.getHotbarSlot(selected);
    if (!sel.isEmpty()) {
        const char* name = getBlockData(sel.type).name;
        m_text->drawTextCentered(name, fw / 2.0f, fh - 2.0f,
                                 1.5f, {1.0f, 1.0f, 1.0f, 0.8f}, screenW, screenH);
    }
}

// ---------------------------------------------------------------------------
// Debug info
// ---------------------------------------------------------------------------
void HudRenderer::drawDebugInfo(const PlayerController& player, int fps,
                                 int loadedChunks, int mobCount,
                                 int screenW, int screenH) {
    char buf[128];
    float y = 5.0f;
    float scale = 1.5f;
    float lineH = m_text->textHeight(scale) + 3.0f;
    glm::vec4 col = {1.0f, 1.0f, 1.0f, 0.8f};

    // FPS
    std::snprintf(buf, sizeof(buf), "FPS: %d", fps);
    m_text->drawText(buf, 5.0f, y, scale, col, screenW, screenH);
    y += lineH;

    // Position
    const auto& pos = player.getPosition();
    std::snprintf(buf, sizeof(buf), "XYZ: %.1f / %.1f / %.1f", pos.x, pos.y, pos.z);
    m_text->drawText(buf, 5.0f, y, scale, col, screenW, screenH);
    y += lineH;

    // Chunks
    std::snprintf(buf, sizeof(buf), "Chunks: %d", loadedChunks);
    m_text->drawText(buf, 5.0f, y, scale, col, screenW, screenH);
    y += lineH;

    // Mobs
    std::snprintf(buf, sizeof(buf), "Mobs: %d", mobCount);
    m_text->drawText(buf, 5.0f, y, scale, col, screenW, screenH);
}

// ---------------------------------------------------------------------------
// Main playing HUD
// ---------------------------------------------------------------------------
void HudRenderer::drawPlayingHUD(const PlayerController& player, int fps,
                                  int loadedChunks, int mobCount, bool creativeMode,
                                  int screenW, int screenH) {
    if (!m_text) return;

    float fw = static_cast<float>(screenW);

    // Debug info (top-left)
    drawDebugInfo(player, fps, loadedChunks, mobCount, screenW, screenH);

    // Mode indicator (top-right)
    const char* modeStr = creativeMode ? "Creative" : "Survival";
    float modeW = m_text->textWidth(modeStr, 1.5f);
    m_text->drawText(modeStr, fw - modeW - 5.0f, 5.0f, 1.5f,
                     {0.8f, 0.8f, 0.8f, 0.8f}, screenW, screenH);

    // Hotbar (bottom-center)
    drawHotbar(player.getInventory(), screenW, screenH);

    // Health bar (above hotbar, left side)
    if (!creativeMode) {
        drawHealthBar(player.getHealth(), player.getMaxHealth(), screenW, screenH);
        drawHungerBar(player.getHunger(), player.getMaxHunger(), screenW, screenH);
    }
}

// ---------------------------------------------------------------------------
// Inventory screen overlay
// ---------------------------------------------------------------------------
void HudRenderer::drawInventoryScreen(const Inventory& inventory,
                                       int screenW, int screenH,
                                       const glm::vec2& mousePos, bool mouseClick,
                                       int& clickedSlot) {
    if (!m_text) return;

    float fw = static_cast<float>(screenW);
    float fh = static_cast<float>(screenH);

    clickedSlot = -1;

    // Dark background overlay
    m_text->drawRect(0.0f, 0.0f, fw, fh, {0.0f, 0.0f, 0.0f, 0.65f}, screenW, screenH);

    // Inventory grid: 4 rows x 9 cols
    // Top 3 rows = main inventory (slots 9-35), bottom row = hotbar (slots 0-8)
    float gridW = 9 * (SLOT_SIZE + SLOT_GAP) - SLOT_GAP;
    float gridH = 4 * (SLOT_SIZE + SLOT_GAP) - SLOT_GAP + 10.0f; // extra gap before hotbar
    float gridX = (fw - gridW) / 2.0f;
    float gridY = (fh - gridH) / 2.0f;

    // Title
    m_text->drawTextCentered("Inventory", fw / 2.0f, gridY - 25.0f, 2.5f,
                             {1.0f, 1.0f, 1.0f, 1.0f}, screenW, screenH);

    // Main inventory (rows 0-2 = slots 9-35)
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 9; ++col) {
            int slotIdx = 9 + row * 9 + col;
            float sx = gridX + col * (SLOT_SIZE + SLOT_GAP);
            float sy = gridY + row * (SLOT_SIZE + SLOT_GAP);

            drawSlot(sx, sy, SLOT_SIZE, inventory.getSlot(slotIdx),
                     false, screenW, screenH);

            // Check click
            if (mouseClick &&
                mousePos.x >= sx && mousePos.x <= sx + SLOT_SIZE &&
                mousePos.y >= sy && mousePos.y <= sy + SLOT_SIZE) {
                clickedSlot = slotIdx;
            }
        }
    }

    // Separator gap
    float hotbarRowY = gridY + 3 * (SLOT_SIZE + SLOT_GAP) + 6.0f;

    // Hotbar row (slots 0-8)
    for (int col = 0; col < 9; ++col) {
        int slotIdx = col;
        float sx = gridX + col * (SLOT_SIZE + SLOT_GAP);

        bool isSelected = (col == inventory.getSelectedSlot());
        drawSlot(sx, hotbarRowY, SLOT_SIZE, inventory.getSlot(slotIdx),
                 isSelected, screenW, screenH);

        // Check click
        if (mouseClick &&
            mousePos.x >= sx && mousePos.x <= sx + SLOT_SIZE &&
            mousePos.y >= hotbarRowY && mousePos.y <= hotbarRowY + SLOT_SIZE) {
            clickedSlot = slotIdx;
        }
    }
}

} // namespace voxelforge
