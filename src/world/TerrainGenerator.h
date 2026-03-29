#pragma once
#include "BiomeGenerator.h"
#include <cstdint>

namespace voxelforge {

class Chunk;

class TerrainGenerator {
public:
    explicit TerrainGenerator(int64_t seed);
    int  getHeight(int worldX, int worldZ, Biome biome) const;
    int  getBlendedHeight(int worldX, int worldZ, const BiomeGenerator& biomes) const;
    void carveCaves(Chunk& chunk) const;
    float getRiverValue(int worldX, int worldZ) const;
    float getContinentalValue(int worldX, int worldZ) const;
private:
    float computeRawHeight(int worldX, int worldZ, Biome biome,
                           float baseHeight, float heightVariation) const;
    int64_t m_seed;
};

} // namespace voxelforge
