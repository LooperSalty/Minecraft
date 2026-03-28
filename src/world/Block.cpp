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
    // Phase 2 – vegetation & terrain variety
    {"tall_grass",     false, false, 0,  {28, 28, 28, 28, 28, 28}},
    {"poppy",          false, false, 0,  {29, 29, 29, 29, 29, 29}},
    {"dandelion",      false, false, 0,  {30, 30, 30, 30, 30, 30}},
    {"cactus",         true,  true,  0,  {31, 31, 31, 31, 31, 31}},
    {"clay",           true,  true,  0,  {32, 32, 32, 32, 32, 32}},
    // Phase 2b – decorative blocks
    {"glass",          false, true,  0,  {33, 33, 33, 33, 33, 33}},
    {"bricks",         true,  true,  0,  {34, 34, 34, 34, 34, 34}},
    {"bookshelf",      true,  true,  0,  {7,  7,  35, 35, 35, 35}},  // top/bottom=planks, sides=books
    {"tnt",            true,  true,  0,  {37, 37, 36, 36, 36, 36}},  // top/bottom=gray cap, sides=red
    {"pumpkin",        true,  true,  0,  {39, 39, 38, 38, 38, 38}},  // top=stem, sides=orange
    {"melon",          true,  true,  0,  {41, 41, 40, 40, 40, 40}},  // top=tan, sides=green stripes
    {"crafting_table", true,  true,  0,  {42, 7,  43, 43, 43, 43}},  // top=tool marks, bottom=planks, sides=regular
    {"furnace",        true,  true,  0,  {0,  0,  44, 0,  0,  0 }},  // top/bottom=stone, north=furnace face, other sides=stone
    // Phase 2c – world generation blocks
    {"mossy_cobble",   true,  true,  0,  {45, 45, 45, 45, 45, 45}},
    {"ice",            false, true,  0,  {46, 46, 46, 46, 46, 46}},
    {"snow_layer",     false, false, 0,  {26, 26, 26, 26, 26, 26}},
    {"mycelium",       true,  true,  0,  {47, 1,  48, 48, 48, 48}},
    {"vine",           false, false, 0,  {49, 49, 49, 49, 49, 49}},
    {"dead_bush",      false, false, 0,  {50, 50, 50, 50, 50, 50}},
    {"lily_pad",       false, false, 0,  {51, 51, 51, 51, 51, 51}},
    {"mushroom_red",   false, false, 0,  {52, 52, 52, 52, 52, 52}},
    {"mushroom_brown", false, false, 0,  {53, 53, 53, 53, 53, 53}},
    {"chest_block",    true,  true,  0,  {54, 54, 55, 55, 55, 55}},
};

const BlockData& getBlockData(BlockType type) {
    auto i = static_cast<uint16_t>(type);
    if (i >= static_cast<uint16_t>(BlockType::COUNT)) return BLOCK_DATA[0];
    return BLOCK_DATA[i];
}

bool isBlockOpaque(BlockType type) { return getBlockData(type).isOpaque; }
bool isBlockSolid(BlockType type)  { return getBlockData(type).isSolid;  }
bool isBlockCross(BlockType type) {
    return type == BlockType::TallGrass ||
           type == BlockType::Poppy ||
           type == BlockType::Dandelion ||
           type == BlockType::DeadBush ||
           type == BlockType::MushroomRed ||
           type == BlockType::MushroomBrown ||
           type == BlockType::Vine;
}

bool isBlockTransparent(BlockType type) {
    return type == BlockType::Water ||
           type == BlockType::Glass ||
           type == BlockType::OakLeaves ||
           type == BlockType::BirchLeaves ||
           type == BlockType::SpruceLeaves ||
           type == BlockType::Ice;
}

} // namespace voxelforge
