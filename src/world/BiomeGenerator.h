#pragma once
#include <cstdint>

namespace voxelforge {

enum class Biome : uint8_t {
    Plains = 0, Forest, BirchForest, Desert,
    ExtremeHills, Taiga, Swamp, Ocean, Beach,
    COUNT
};

struct BiomeData {
    const char* name;
    float baseHeight;       // added to SEA_LEVEL
    float heightVariation;  // noise amplitude
    int   treeDensity;      // trees per chunk
};

const BiomeData& getBiomeData(Biome biome);

class BiomeGenerator {
public:
    explicit BiomeGenerator(int64_t seed);
    Biome getBiome(int worldX, int worldZ) const;
private:
    int64_t m_seed;
};

} // namespace voxelforge
