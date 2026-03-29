#include "TerrainGenerator.h"
#include "Chunk.h"
#include "Block.h"
#include "../util/Noise.h"
#include <algorithm>
#include <cmath>

namespace voxelforge {

TerrainGenerator::TerrainGenerator(int64_t seed)
    : m_seed(seed) {}

float TerrainGenerator::getRiverValue(int worldX, int worldZ) const {
    float fx = static_cast<float>(worldX);
    float fz = static_cast<float>(worldZ);
    int baseSeed = static_cast<int>(m_seed & 0x7FFFFFFF);

    // Primary river path: low-frequency ridged noise
    float river1 = octavePerlin2D(fx, fz, 4, 0.5f, 1.0f / 512.0f, baseSeed + 8000);
    // Secondary tributary
    float river2 = octavePerlin2D(fx, fz, 3, 0.5f, 1.0f / 300.0f, baseSeed + 8100);

    // River is where noise is close to 0 (narrow band)
    float r1 = std::abs(river1);
    float r2 = std::abs(river2);

    // Return the closer river
    return std::min(r1, r2);
}

float TerrainGenerator::getContinentalValue(int worldX, int worldZ) const {
    float fx = static_cast<float>(worldX);
    float fz = static_cast<float>(worldZ);
    int baseSeed = static_cast<int>(m_seed & 0x7FFFFFFF);

    // Very low frequency continental noise for large-scale land/ocean separation
    return octavePerlin2D(fx, fz, 6, 0.5f, 1.0f / 800.0f, baseSeed + 9000);
}

float TerrainGenerator::computeRawHeight(int worldX, int worldZ, Biome biome,
                                          float bHeight, float bVariation) const {
    float fx = static_cast<float>(worldX);
    float fz = static_cast<float>(worldZ);
    int baseSeed = static_cast<int>(m_seed & 0x7FFFFFFF);

    float continental = getContinentalValue(worldX, worldZ);
    float baseNoise = octavePerlin2D(fx, fz, 8, 0.5f, 1.0f / 256.0f, baseSeed);

    float ridgeRaw = octavePerlin2D(fx, fz, 5, 0.6f, 1.0f / 200.0f, baseSeed + 6000);
    float ridge = 1.0f - std::abs(ridgeRaw);
    ridge = ridge * ridge;

    float detail = octavePerlin2D(fx, fz, 4, 0.5f, 1.0f / 64.0f, baseSeed + 7000);

    float erosion = octavePerlin2D(fx, fz, 3, 0.5f, 1.0f / 350.0f, baseSeed + 7500);
    float erosionFactor = (erosion + 1.0f) * 0.5f;
    erosionFactor = std::clamp(erosionFactor, 0.3f, 1.0f);

    float height = static_cast<float>(SEA_LEVEL)
                 + bHeight
                 + baseNoise * bVariation * erosionFactor
                 + ridge * bVariation * 0.4f * erosionFactor
                 + detail * 2.0f;

    // Continental modifier
    if (continental < -0.2f) {
        height -= (-0.2f - continental) * 15.0f;
    } else if (continental > 0.3f) {
        height += (continental - 0.3f) * 5.0f;
    }

    // Biome-specific modifiers
    switch (biome) {
        case Biome::ExtremeHills: {
            float plateauNoise = octavePerlin2D(fx, fz, 2, 0.5f, 1.0f / 400.0f, baseSeed + 7800);
            // Scale mountain extra by how much variation is present (smooth at biome edges)
            float mountainStrength = std::clamp(bVariation / 18.0f, 0.0f, 1.0f);
            height += ridge * 10.0f * mountainStrength;

            if (plateauNoise > 0.4f && height > static_cast<float>(SEA_LEVEL) + 30.0f) {
                float plateauLevel = static_cast<float>(SEA_LEVEL) + 35.0f +
                    plateauNoise * 20.0f;
                if (height > plateauLevel) {
                    float excess = height - plateauLevel;
                    height = plateauLevel + excess * 0.15f;
                }
            }
            break;
        }
        case Biome::Ocean: {
            float oceanHills = octavePerlin2D(fx, fz, 3, 0.5f, 1.0f / 100.0f, baseSeed + 7200);
            height += oceanHills * 6.0f;
            height = std::min(height, static_cast<float>(SEA_LEVEL - 3));
            break;
        }
        case Biome::Beach: {
            float beachHeight = static_cast<float>(SEA_LEVEL) + detail * 0.5f;
            height = height * 0.2f + beachHeight * 0.8f;
            break;
        }
        case Biome::Swamp: {
            float swampBase = static_cast<float>(SEA_LEVEL) - 1.0f + detail * 1.5f;
            height = height * 0.3f + swampBase * 0.7f;
            break;
        }
        default:
            break;
    }

    // River carving
    float riverDist = getRiverValue(worldX, worldZ);
    if (riverDist < 0.04f && biome != Biome::Ocean && biome != Biome::Beach) {
        float riverFactor = 1.0f - (riverDist / 0.04f);
        height -= riverFactor * 5.0f;
    }

    // Erosion deposit
    if (erosionFactor < 0.5f && height > static_cast<float>(SEA_LEVEL) &&
        height < static_cast<float>(SEA_LEVEL) + 10.0f) {
        height += (0.5f - erosionFactor) * 3.0f;
    }

    return height;
}

int TerrainGenerator::getHeight(int worldX, int worldZ, Biome biome) const {
    const BiomeData& data = getBiomeData(biome);
    float h = computeRawHeight(worldX, worldZ, biome, data.baseHeight, data.heightVariation);
    return std::clamp(static_cast<int>(h), 1, 250);
}

int TerrainGenerator::getBlendedHeight(int worldX, int worldZ,
                                        const BiomeGenerator& biomes) const {
    // Blend biome parameters over a neighborhood to smooth transitions.
    // Without this, adjacent biomes with different baseHeight/heightVariation
    // create sharp cliffs at the boundary.
    constexpr int R = 8;      // blend radius in blocks (was 4 — too small)
    constexpr int STEP = 2;   // sample every 2 blocks for performance
    float totalWeight = 0.0f;
    float blendedBase = 0.0f;
    float blendedVar  = 0.0f;

    for (int dx = -R; dx <= R; dx += STEP) {
        for (int dz = -R; dz <= R; dz += STEP) {
            Biome b = biomes.getBiome(worldX + dx, worldZ + dz);
            const BiomeData& bd = getBiomeData(b);
            float dist2 = static_cast<float>(dx * dx + dz * dz);
            float w = 1.0f / (1.0f + dist2 * 0.15f);
            totalWeight += w;
            blendedBase += bd.baseHeight * w;
            blendedVar  += bd.heightVariation * w;
        }
    }
    blendedBase /= totalWeight;
    blendedVar  /= totalWeight;

    Biome localBiome = biomes.getBiome(worldX, worldZ);
    float h = computeRawHeight(worldX, worldZ, localBiome, blendedBase, blendedVar);
    return std::clamp(static_cast<int>(h), 1, 250);
}

void TerrainGenerator::carveCaves(Chunk& chunk) const {
    int baseSeed = static_cast<int>(m_seed & 0x7FFFFFFF);
    int cx = chunk.getChunkX() * CHUNK_WIDTH;
    int cz = chunk.getChunkZ() * CHUNK_DEPTH;

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            float wx = static_cast<float>(cx + x);
            float wz = static_cast<float>(cz + z);

            for (int y = 5; y <= 62; ++y) {
                BlockType current = chunk.getBlock(x, y, z);
                if (current != BlockType::Stone &&
                    current != BlockType::Dirt &&
                    current != BlockType::GrassBlock &&
                    current != BlockType::Sand &&
                    current != BlockType::Sandstone) {
                    continue;
                }

                float wy = static_cast<float>(y);

                // Standard cave tunnels
                float noise = octavePerlin3D(wx, wy, wz, 3, 0.5f, 1.0f / 32.0f, baseSeed + 5000);

                // Vary threshold with depth: larger caves deeper down
                float depthFactor = 1.0f - (static_cast<float>(y) / 62.0f);
                float threshold = 0.55f - depthFactor * 0.08f;

                // Larger caverns: separate low-frequency noise
                float cavern = octavePerlin3D(wx, wy, wz, 2, 0.5f, 1.0f / 80.0f, baseSeed + 5500);
                bool isCavern = (cavern > 0.6f) && (y < 40);

                // Spaghetti caves: thin winding tunnels
                float spaghetti1 = octavePerlin3D(wx, wy, wz, 2, 0.5f, 1.0f / 50.0f, baseSeed + 5600);
                float spaghetti2 = octavePerlin3D(wx, wy, wz, 2, 0.5f, 1.0f / 50.0f, baseSeed + 5700);
                bool isSpaghetti = (std::abs(spaghetti1) < 0.03f && std::abs(spaghetti2) < 0.03f);

                bool isCave = (noise > threshold) || isCavern || isSpaghetti;

                if (isCave) {
                    if (y <= 10) {
                        chunk.setBlock(x, y, z, BlockType::Lava);
                    } else {
                        chunk.setBlock(x, y, z, BlockType::Air);
                    }
                }
            }
        }
    }
}

} // namespace voxelforge
