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

int TerrainGenerator::getHeight(int worldX, int worldZ, Biome biome) const {
    float fx = static_cast<float>(worldX);
    float fz = static_cast<float>(worldZ);
    int baseSeed = static_cast<int>(m_seed & 0x7FFFFFFF);

    // Continental noise: large-scale land/ocean separation
    float continental = getContinentalValue(worldX, worldZ);

    // Base terrain noise
    float baseNoise = octavePerlin2D(fx, fz, 8, 0.5f, 1.0f / 256.0f, baseSeed);

    // Ridge noise layer: creates dramatic cliffs and ridges
    float ridgeRaw = octavePerlin2D(fx, fz, 5, 0.6f, 1.0f / 200.0f, baseSeed + 6000);
    float ridge = 1.0f - std::abs(ridgeRaw);
    ridge = ridge * ridge; // sharpen the ridge

    // Detail noise for micro-terrain variation
    float detail = octavePerlin2D(fx, fz, 4, 0.5f, 1.0f / 64.0f, baseSeed + 7000);

    // Erosion simulation: smooth steep slopes
    float erosion = octavePerlin2D(fx, fz, 3, 0.5f, 1.0f / 350.0f, baseSeed + 7500);
    // Erosion factor: 0.0 = heavily eroded (smooth), 1.0 = not eroded (rough)
    float erosionFactor = (erosion + 1.0f) * 0.5f;
    erosionFactor = std::clamp(erosionFactor, 0.3f, 1.0f);

    // Plateau noise: flat-topped mountains
    float plateauNoise = octavePerlin2D(fx, fz, 2, 0.5f, 1.0f / 400.0f, baseSeed + 7800);

    const BiomeData& data = getBiomeData(biome);
    float height = static_cast<float>(SEA_LEVEL)
                 + data.baseHeight
                 + baseNoise * data.heightVariation * erosionFactor
                 + ridge * data.heightVariation * 0.4f * erosionFactor
                 + detail * 2.0f;

    // Continental modifier: push oceans deeper, raise continents
    if (continental < -0.2f) {
        // Ocean areas: deepen based on continental value
        float oceanDepth = (-0.2f - continental) * 15.0f;
        height -= oceanDepth;
    } else if (continental > 0.3f) {
        // Deep inland: slightly raise terrain
        float raise = (continental - 0.3f) * 5.0f;
        height += raise;
    }

    // Biome-specific terrain modifications
    switch (biome) {
        case Biome::ExtremeHills: {
            // More dramatic mountains with optional plateaus
            float mountainExtra = ridge * 20.0f;
            height += mountainExtra;

            // Plateaus: clamp height at certain levels
            if (plateauNoise > 0.4f && height > static_cast<float>(SEA_LEVEL) + 30.0f) {
                float plateauLevel = static_cast<float>(SEA_LEVEL) + 35.0f +
                    plateauNoise * 20.0f;
                if (height > plateauLevel) {
                    // Smooth transition to plateau top
                    float excess = height - plateauLevel;
                    height = plateauLevel + excess * 0.15f;
                }
            }
            break;
        }
        case Biome::Ocean: {
            // Underwater hills: add variation to ocean floor
            float oceanHills = octavePerlin2D(fx, fz, 3, 0.5f, 1.0f / 100.0f, baseSeed + 7200);
            height += oceanHills * 6.0f;
            // Ensure ocean floor stays below sea level
            height = std::min(height, static_cast<float>(SEA_LEVEL - 3));
            break;
        }
        case Biome::Beach: {
            // Very flat near sea level
            float beachHeight = static_cast<float>(SEA_LEVEL) + detail * 0.5f;
            // Blend toward beach height
            height = height * 0.2f + beachHeight * 0.8f;
            break;
        }
        case Biome::Swamp: {
            // Very flat, near sea level
            float swampBase = static_cast<float>(SEA_LEVEL) - 1.0f + detail * 1.5f;
            height = height * 0.3f + swampBase * 0.7f;
            break;
        }
        default:
            break;
    }

    // River valley carving: lower terrain along river paths
    float riverDist = getRiverValue(worldX, worldZ);
    if (riverDist < 0.04f && biome != Biome::Ocean && biome != Biome::Beach) {
        float riverFactor = 1.0f - (riverDist / 0.04f);
        float carve = riverFactor * 8.0f;
        height -= carve;
    }

    // Erosion: sediment deposit at the bottom of hills
    // Where erosion is strong and terrain is near sea level, add slight build-up
    if (erosionFactor < 0.5f && height > static_cast<float>(SEA_LEVEL) &&
        height < static_cast<float>(SEA_LEVEL) + 10.0f) {
        float deposit = (0.5f - erosionFactor) * 3.0f;
        height += deposit;
    }

    return std::clamp(static_cast<int>(height), 1, 250);
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
