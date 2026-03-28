#pragma once
#include "BiomeGenerator.h"
#include "TerrainGenerator.h"
#include <cstdint>

namespace voxelforge {

class Chunk;

class WorldGenerator {
public:
    explicit WorldGenerator(int64_t seed);
    void generate(Chunk& chunk) const;
    const BiomeGenerator&  getBiomes()  const { return m_biomes; }
    const TerrainGenerator& getTerrain() const { return m_terrain; }
    int64_t getSeed() const { return m_seed; }
private:
    int64_t m_seed;
    BiomeGenerator  m_biomes;
    TerrainGenerator m_terrain;
};

} // namespace voxelforge
