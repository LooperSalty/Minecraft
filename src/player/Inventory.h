#pragma once
#include "../world/Block.h"
#include <algorithm>

namespace voxelforge {

struct ItemStack {
    BlockType type = BlockType::Air;
    int count = 0;

    bool isEmpty() const { return type == BlockType::Air || count <= 0; }
};

class Inventory {
public:
    static constexpr int HOTBAR_SIZE = 9;
    static constexpr int TOTAL_SLOTS = 36; // 9 hotbar + 27 main
    static constexpr int MAX_STACK = 64;

    Inventory();

    const ItemStack& getSlot(int index) const;
    ItemStack& getSlot(int index);
    const ItemStack& getHotbarSlot(int index) const;
    ItemStack& getHotbarSlot(int index);

    int getSelectedSlot() const { return m_selectedSlot; }
    void setSelectedSlot(int slot);
    void scrollSelected(float delta);

    bool addItem(BlockType type, int count = 1);
    bool removeItem(int slot, int count = 1);
    void swapSlots(int a, int b);

    BlockType getSelectedBlockType() const;
    int getSelectedCount() const;

private:
    ItemStack m_slots[TOTAL_SLOTS] = {};
    int m_selectedSlot = 0;
};

} // namespace voxelforge
