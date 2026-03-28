#include "Block.h"

namespace voxelforge {

// Texture index map (position in 16x16 atlas grid):
//  0=Stone  1=Dirt  2=GrassTop  3=GrassSide  4=Cobblestone
//  5=LogTop  6=LogSide  7=Planks  8=Bedrock  9=Sand
// 10=Gravel 11=Water 12=Lava

static constexpr BlockData BLOCK_DATA[] = {
    // name            opaque solid  light  textures {top,bot,N,S,E,W}
    {"air",            false, false, 0,  {0,  0,  0,  0,  0,  0 }},
    {"stone",          true,  true,  0,  {0,  0,  0,  0,  0,  0 }},
    {"dirt",           true,  true,  0,  {1,  1,  1,  1,  1,  1 }},
    {"grass_block",    true,  true,  0,  {2,  1,  3,  3,  3,  3 }},
    {"cobblestone",    true,  true,  0,  {4,  4,  4,  4,  4,  4 }},
    {"oak_log",        true,  true,  0,  {5,  5,  6,  6,  6,  6 }},
    {"oak_planks",     true,  true,  0,  {7,  7,  7,  7,  7,  7 }},
    {"bedrock",        true,  true,  0,  {8,  8,  8,  8,  8,  8 }},
    {"sand",           true,  true,  0,  {9,  9,  9,  9,  9,  9 }},
    {"gravel",         true,  true,  0,  {10, 10, 10, 10, 10, 10}},
    {"water",          false, false, 0,  {11, 11, 11, 11, 11, 11}},
    {"lava",           false, false, 15, {12, 12, 12, 12, 12, 12}},
    // Phase 1
    {"oak_leaves",     true,  true,  0,  {13, 13, 13, 13, 13, 13}},
    {"birch_log",      true,  true,  0,  {15, 15, 14, 14, 14, 14}},
    {"birch_leaves",   true,  true,  0,  {16, 16, 16, 16, 16, 16}},
    {"spruce_log",     true,  true,  0,  {5,  5,  17, 17, 17, 17}},
    {"spruce_leaves",  true,  true,  0,  {18, 18, 18, 18, 18, 18}},
    {"coal_ore",       true,  true,  0,  {19, 19, 19, 19, 19, 19}},
    {"iron_ore",       true,  true,  0,  {20, 20, 20, 20, 20, 20}},
    {"gold_ore",       true,  true,  0,  {21, 21, 21, 21, 21, 21}},
    {"diamond_ore",    true,  true,  0,  {22, 22, 22, 22, 22, 22}},
    {"redstone_ore",   true,  true,  0,  {23, 23, 23, 23, 23, 23}},
    {"lapis_ore",      true,  true,  0,  {24, 24, 24, 24, 24, 24}},
    {"emerald_ore",    true,  true,  0,  {25, 25, 25, 25, 25, 25}},
    {"snow_block",     true,  true,  0,  {26, 26, 26, 26, 26, 26}},
    {"sandstone",      true,  true,  0,  {27, 27, 27, 27, 27, 27}},
};

const BlockData& getBlockData(BlockType type) {
    auto i = static_cast<uint16_t>(type);
    if (i >= static_cast<uint16_t>(BlockType::COUNT)) return BLOCK_DATA[0];
    return BLOCK_DATA[i];
}

bool isBlockOpaque(BlockType type) { return getBlockData(type).isOpaque; }
bool isBlockSolid(BlockType type)  { return getBlockData(type).isSolid;  }

} // namespace voxelforge
