#pragma once
#include "BiomeGenerator.h"
#include <cstdint>

namespace voxelforge {

class Chunk;

class TerrainGenerator {
public:
    explicit TerrainGenerator(int64_t seed);
    int  getHeight(int worldX, int worldZ, Biome biome) const;
    void carveCaves(Chunk& chunk) const;
private:
    int64_t m_seed;
};

} // namespace voxelforge
