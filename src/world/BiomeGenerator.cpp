#include "BiomeGenerator.h"
#include "../util/Noise.h"
#include <array>

namespace voxelforge {

static const std::array<BiomeData, static_cast<size_t>(Biome::COUNT)> s_biomeTable = {{
    { "Plains",       0.0f,   3.0f, 1 },
    { "Forest",       0.0f,   5.0f, 8 },
    { "BirchForest",  0.0f,   4.0f, 8 },
    { "Desert",       0.0f,   2.0f, 0 },
    { "ExtremeHills", 10.0f, 30.0f, 1 },
    { "Taiga",        2.0f,   5.0f, 7 },
    { "Swamp",       -3.0f,   2.0f, 3 },
    { "Ocean",       -25.0f,  5.0f, 0 },
    { "Beach",        0.0f,   1.0f, 0 },
}};

const BiomeData& getBiomeData(Biome biome) {
    auto idx = static_cast<size_t>(biome);
    if (idx >= s_biomeTable.size()) {
        return s_biomeTable[0];
    }
    return s_biomeTable[idx];
}

BiomeGenerator::BiomeGenerator(int64_t seed)
    : m_seed(seed) {}

Biome BiomeGenerator::getBiome(int worldX, int worldZ) const {
    float fx = static_cast<float>(worldX);
    float fz = static_cast<float>(worldZ);
    int baseSeed = static_cast<int>(m_seed & 0x7FFFFFFF);

    float temperature    = octavePerlin2D(fx, fz, 4, 0.5f, 1.0f / 256.0f, baseSeed + 1000);
    float humidity       = octavePerlin2D(fx, fz, 4, 0.5f, 1.0f / 256.0f, baseSeed + 2000);
    float continentalness = octavePerlin2D(fx, fz, 6, 0.5f, 1.0f / 400.0f, baseSeed + 3000);

    if (continentalness < -0.3f) {
        return Biome::Ocean;
    }
    if (continentalness < -0.15f) {
        return Biome::Beach;
    }

    bool hot  = temperature > 0.3f;
    bool cold = temperature < -0.3f;
    bool dry  = humidity < -0.2f;
    bool wet  = humidity > 0.2f;

    if (hot && dry) {
        return Biome::Desert;
    }
    if (hot && wet) {
        return Biome::Swamp;
    }
    if (cold) {
        return (humidity > 0.0f) ? Biome::Taiga : Biome::ExtremeHills;
    }
    if (wet) {
        return (temperature < 0.0f) ? Biome::BirchForest : Biome::Forest;
    }

    return Biome::Plains;
}

} // namespace voxelforge
