#include "Inventory.h"
#include <algorithm>

namespace voxelforge {

Inventory::Inventory() {
    // Survival mode: start with empty inventory
}

const ItemStack& Inventory::getSlot(int index) const {
    static const ItemStack empty{};
    if (index < 0 || index >= TOTAL_SLOTS) return empty;
    return m_slots[index];
}

ItemStack& Inventory::getSlot(int index) {
    static ItemStack dummy{};
    if (index < 0 || index >= TOTAL_SLOTS) return dummy;
    return m_slots[index];
}

const ItemStack& Inventory::getHotbarSlot(int index) const {
    return getSlot(index);
}

ItemStack& Inventory::getHotbarSlot(int index) {
    return getSlot(index);
}

void Inventory::setSelectedSlot(int slot) {
    m_selectedSlot = std::clamp(slot, 0, HOTBAR_SIZE - 1);
}

void Inventory::scrollSelected(float delta) {
    if (delta == 0.0f) return;
    int d = (delta > 0.0f) ? 1 : -1;
    m_selectedSlot = ((m_selectedSlot + d) % HOTBAR_SIZE + HOTBAR_SIZE) % HOTBAR_SIZE;
}

bool Inventory::addItem(BlockType type, int count) {
    if (type == BlockType::Air || count <= 0) return false;

    // First pass: try to stack with existing matching slots
    for (int i = 0; i < TOTAL_SLOTS; ++i) {
        if (m_slots[i].type == type && m_slots[i].count < MAX_STACK) {
            int space = MAX_STACK - m_slots[i].count;
            int toAdd = std::min(count, space);
            m_slots[i].count += toAdd;
            count -= toAdd;
            if (count <= 0) return true;
        }
    }

    // Second pass: find empty slots
    for (int i = 0; i < TOTAL_SLOTS; ++i) {
        if (m_slots[i].isEmpty()) {
            int toAdd = std::min(count, MAX_STACK);
            m_slots[i].type = type;
            m_slots[i].count = toAdd;
            count -= toAdd;
            if (count <= 0) return true;
        }
    }

    return count <= 0;
}

bool Inventory::removeItem(int slot, int count) {
    if (slot < 0 || slot >= TOTAL_SLOTS) return false;
    if (m_slots[slot].isEmpty()) return false;
    if (m_slots[slot].count < count) return false;

    m_slots[slot].count -= count;
    if (m_slots[slot].count <= 0) {
        m_slots[slot].type = BlockType::Air;
        m_slots[slot].count = 0;
    }
    return true;
}

void Inventory::swapSlots(int a, int b) {
    if (a < 0 || a >= TOTAL_SLOTS || b < 0 || b >= TOTAL_SLOTS) return;
    if (a == b) return;
    ItemStack temp = m_slots[a];
    m_slots[a] = m_slots[b];
    m_slots[b] = temp;
}

BlockType Inventory::getSelectedBlockType() const {
    return m_slots[m_selectedSlot].type;
}

int Inventory::getSelectedCount() const {
    return m_slots[m_selectedSlot].count;
}

} // namespace voxelforge
