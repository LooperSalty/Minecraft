#include "Chunk.h"

namespace voxelforge {

Chunk::Chunk(int chunkX, int chunkZ)
    : m_chunkX(chunkX), m_chunkZ(chunkZ)
{
    m_blocks.fill(BlockType::Air);
}

int Chunk::index(int x, int y, int z) {
    return y * CHUNK_WIDTH * CHUNK_DEPTH + z * CHUNK_WIDTH + x;
}

bool Chunk::inBounds(int x, int y, int z) {
    return x >= 0 && x < CHUNK_WIDTH
        && y >= 0 && y < CHUNK_HEIGHT
        && z >= 0 && z < CHUNK_DEPTH;
}

BlockType Chunk::getBlock(int x, int y, int z) const {
    if (!inBounds(x, y, z)) return BlockType::Air;
    return m_blocks[index(x, y, z)];
}

void Chunk::setBlock(int x, int y, int z, BlockType type) {
    if (!inBounds(x, y, z)) return;
    m_blocks[index(x, y, z)] = type;
}

glm::ivec3 Chunk::toWorldPos(int lx, int ly, int lz) const {
    return {m_chunkX * CHUNK_WIDTH + lx, ly, m_chunkZ * CHUNK_DEPTH + lz};
}

} // namespace voxelforge
