#include "OreGenerator.h"
#include "Chunk.h"
#include "Block.h"
#include <cstdint>

namespace voxelforge {

static uint32_t hash(int64_t seed, int a, int b, int c) {
    uint64_t h = static_cast<uint64_t>(seed);
    h = h * 6364136223846793005ULL + 1442695040888963407ULL + static_cast<uint64_t>(a);
    h = h * 6364136223846793005ULL + 1442695040888963407ULL + static_cast<uint64_t>(b);
    h = h * 6364136223846793005ULL + 1442695040888963407ULL + static_cast<uint64_t>(c);
    return static_cast<uint32_t>(h >> 16);
}

struct OreConfig {
    BlockType type;
    int veinsPerChunk;
    int yMin;
    int yMax;
    int size; // radius for sphere placement
};

static constexpr OreConfig s_oreTable[] = {
    { BlockType::CoalOre,     20, 0, 128, 3 },
    { BlockType::IronOre,     20, 0,  64, 2 },
    { BlockType::GoldOre,      2, 0,  32, 2 },
    { BlockType::RedstoneOre,  8, 0,  16, 2 },
    { BlockType::DiamondOre,   1, 0,  16, 2 },
    { BlockType::LapisOre,     1, 0,  32, 2 },
    { BlockType::EmeraldOre,   1, 4,  32, 1 },
};

static constexpr int ORE_COUNT = sizeof(s_oreTable) / sizeof(s_oreTable[0]);

void OreGenerator::generate(Chunk& chunk, int64_t seed) {
    int chunkX = chunk.getChunkX();
    int chunkZ = chunk.getChunkZ();

    for (int oreIdx = 0; oreIdx < ORE_COUNT; ++oreIdx) {
        const OreConfig& ore = s_oreTable[oreIdx];
        int yRange = ore.yMax - ore.yMin;
        if (yRange <= 0) {
            continue;
        }

        for (int vein = 0; vein < ore.veinsPerChunk; ++vein) {
            uint32_t h = hash(seed, chunkX * 1000 + oreIdx, chunkZ * 1000 + vein, oreIdx * 7 + vein);

            int cx = static_cast<int>(h % CHUNK_WIDTH);
            int cy = ore.yMin + static_cast<int>((h >> 4) % static_cast<uint32_t>(yRange));
            int cz = static_cast<int>((h >> 8) % CHUNK_DEPTH);

            int radius = ore.size;
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dy = -radius; dy <= radius; ++dy) {
                    for (int dz = -radius; dz <= radius; ++dz) {
                        if (dx * dx + dy * dy + dz * dz > radius * radius) {
                            continue;
                        }

                        int bx = cx + dx;
                        int by = cy + dy;
                        int bz = cz + dz;

                        if (bx < 0 || bx >= CHUNK_WIDTH ||
                            by < 0 || by >= CHUNK_HEIGHT ||
                            bz < 0 || bz >= CHUNK_DEPTH) {
                            continue;
                        }

                        if (chunk.getBlock(bx, by, bz) == BlockType::Stone) {
                            chunk.setBlock(bx, by, bz, ore.type);
                        }
                    }
                }
            }
        }
    }
}

} // namespace voxelforge
