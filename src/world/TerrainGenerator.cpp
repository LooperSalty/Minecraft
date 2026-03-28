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
    // Low-frequency noise to define river paths
    float river = octavePerlin2D(fx, fz, 4, 0.5f, 1.0f / 512.0f, baseSeed + 8000);
    // River is where noise is close to 0 (narrow band)
    return std::abs(river);
}

int TerrainGenerator::getHeight(int worldX, int worldZ, Biome biome) const {
    float fx = static_cast<float>(worldX);
    float fz = static_cast<float>(worldZ);
    int baseSeed = static_cast<int>(m_seed & 0x7FFFFFFF);

    // Base continental noise
    float baseNoise = octavePerlin2D(fx, fz, 8, 0.5f, 1.0f / 256.0f, baseSeed);

    // Ridge noise layer: creates dramatic cliffs and ridges
    float ridgeRaw = octavePerlin2D(fx, fz, 5, 0.6f, 1.0f / 200.0f, baseSeed + 6000);
    // Transform to ridge shape: 1.0 at ridge peak, 0.0 in valleys
    float ridge = 1.0f - std::abs(ridgeRaw);
    ridge = ridge * ridge; // sharpen the ridge

    // Detail noise for micro-terrain variation
    float detail = octavePerlin2D(fx, fz, 4, 0.5f, 1.0f / 64.0f, baseSeed + 7000);

    const BiomeData& data = getBiomeData(biome);
    float height = static_cast<float>(SEA_LEVEL)
                 + data.baseHeight
                 + baseNoise * data.heightVariation
                 + ridge * data.heightVariation * 0.4f
                 + detail * 2.0f;

    // River valley carving: lower terrain along river paths
    float riverDist = getRiverValue(worldX, worldZ);
    if (riverDist < 0.04f && biome != Biome::Ocean && biome != Biome::Beach) {
        // In the river band, carve down to near sea level
        float riverFactor = 1.0f - (riverDist / 0.04f);
        float carve = riverFactor * 8.0f;
        height -= carve;
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
                    current != BlockType::GrassBlock) {
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

                bool isCave = (noise > threshold) || isCavern;

                if (isCave) {
                    if (y <= 10) {
                        // Lava pools below Y=10
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
