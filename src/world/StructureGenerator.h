#pragma once
#include "BiomeGenerator.h"
#include "TerrainGenerator.h"
#include <cstdint>

namespace voxelforge {

class Chunk;

class StructureGenerator {
public:
    static void generateTrees(Chunk& chunk, int64_t seed,
                              const BiomeGenerator& biomes,
                              const TerrainGenerator& terrain);
    static void generateVegetation(Chunk& chunk, int64_t seed,
                                   const BiomeGenerator& biomes,
                                   const TerrainGenerator& terrain);
    static void generateStructures(Chunk& chunk, int64_t seed,
                                   const BiomeGenerator& biomes,
                                   const TerrainGenerator& terrain);
};

} // namespace voxelforge
