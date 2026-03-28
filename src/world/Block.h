#pragma once
#include <cstdint>
#include <array>

namespace voxelforge {

enum class BlockType : uint16_t {
    Air = 0,
    Stone, Dirt, GrassBlock, Cobblestone,
    OakLog, OakPlanks, Bedrock, Sand, Gravel, Water, Lava,
    // Phase 1
    OakLeaves, BirchLog, BirchLeaves,
    SpruceLog, SpruceLeaves,
    CoalOre, IronOre, GoldOre, DiamondOre,
    RedstoneOre, LapisOre, EmeraldOre,
    SnowBlock, Sandstone,
    // Phase 2 – vegetation & terrain variety
    TallGrass, Poppy, Dandelion, Cactus, Clay,
    // Phase 2b – decorative blocks
    Glass, Bricks, Bookshelf, TNT, Pumpkin, Melon,
    CraftingTable, Furnace,
    // Phase 2c – world generation blocks
    MossyCobblestone, Ice, SnowLayer, Mycelium, Vine,
    DeadBush, LilyPad, MushroomRed, MushroomBrown,
    ChestBlock,
    COUNT
};

enum class BlockFace : uint8_t {
    Top = 0,
    Bottom,
    North,
    South,
    East,
    West
};

struct BlockData {
    const char* name;
    bool isOpaque;
    bool isSolid;
    uint8_t lightEmission;
    std::array<uint8_t, 6> textureIndices; // top, bottom, north, south, east, west
};

const BlockData& getBlockData(BlockType type);
bool isBlockOpaque(BlockType type);
bool isBlockSolid(BlockType type);
bool isBlockCross(BlockType type);
bool isBlockTransparent(BlockType type);

} // namespace voxelforge
