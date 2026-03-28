#include "TerrainGenerator.h"
#include "Chunk.h"
#include "../util/Noise.h"
#include <algorithm>

namespace voxelforge {

TerrainGenerator::TerrainGenerator(int64_t seed)
    : m_seed(seed) {}

int TerrainGenerator::getHeight(int worldX, int worldZ, Biome biome) const {
    float fx = static_cast<float>(worldX);
    float fz = static_cast<float>(worldZ);
    int baseSeed = static_cast<int>(m_seed & 0x7FFFFFFF);

    float baseNoise = octavePerlin2D(fx, fz, 8, 0.5f, 1.0f / 256.0f, baseSeed);

    const BiomeData& data = getBiomeData(biome);
    float height = static_cast<float>(SEA_LEVEL) + data.baseHeight + baseNoise * data.heightVariation;

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

            for (int y = 5; y <= 55; ++y) {
                if (chunk.getBlock(x, y, z) != BlockType::Stone) {
                    continue;
                }

                float wy = static_cast<float>(y);
                float noise = octavePerlin3D(wx, wy, wz, 3, 0.5f, 1.0f / 32.0f, baseSeed + 5000);

                if (noise > 0.55f) {
                    chunk.setBlock(x, y, z, BlockType::Air);
                }
            }
        }
    }
}

} // namespace voxelforge
