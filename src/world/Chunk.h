#pragma once
#include "Block.h"
#include <array>
#include <glm/glm.hpp>

namespace voxelforge {

constexpr int CHUNK_WIDTH  = 16;
constexpr int CHUNK_HEIGHT = 256;
constexpr int CHUNK_DEPTH  = 16;
constexpr int CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;
constexpr int SEA_LEVEL    = 63;

class Chunk {
public:
    Chunk(int chunkX, int chunkZ);

    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType type);

    int getChunkX() const { return m_chunkX; }
    int getChunkZ() const { return m_chunkZ; }
    glm::ivec3 toWorldPos(int lx, int ly, int lz) const;

private:
    static int index(int x, int y, int z);
    static bool inBounds(int x, int y, int z);

    int m_chunkX;
    int m_chunkZ;
    std::array<BlockType, CHUNK_VOLUME> m_blocks;
};

} // namespace voxelforge
