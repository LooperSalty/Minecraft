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

} // namespace voxelforge
