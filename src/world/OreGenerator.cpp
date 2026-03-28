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
    int minSize;  // min vein blocks
    int maxSize;  // max vein blocks (radius derived)
    bool extremeHillsOnly;
};

static constexpr OreConfig s_oreTable[] = {
    // Coal: Y=1-127, veins of 8-16, very common
    { BlockType::CoalOre,     20, 1, 127, 8,  16, false },
    // Iron: Y=1-63, veins of 4-8, common
    { BlockType::IronOre,     20, 1,  63, 4,   8, false },
    // Gold: Y=1-31, veins of 4-8, uncommon
    { BlockType::GoldOre,      2, 1,  31, 4,   8, false },
    // Redstone: Y=1-15, veins of 4-8
    { BlockType::RedstoneOre,  8, 1,  15, 4,   8, false },
    // Diamond: Y=1-15, veins of 1-4, rare
    { BlockType::DiamondOre,   1, 1,  15, 1,   4, false },
    // Lapis: Y=1-30, veins of 4-8
    { BlockType::LapisOre,     1, 1,  30, 4,   8, false },
    // Emerald: Y=1-31, single blocks, ExtremeHills only
    { BlockType::EmeraldOre,   1, 1,  31, 1,   1, true  },
};

static constexpr int ORE_COUNT = sizeof(s_oreTable) / sizeof(s_oreTable[0]);

void OreGenerator::generate(Chunk& chunk, int64_t seed) {
    generate(chunk, seed, false);
}

void OreGenerator::generate(Chunk& chunk, int64_t seed, bool isExtremeHills) {
    int chunkX = chunk.getChunkX();
    int chunkZ = chunk.getChunkZ();

    for (int oreIdx = 0; oreIdx < ORE_COUNT; ++oreIdx) {
        const OreConfig& ore = s_oreTable[oreIdx];

        // Skip ExtremeHills-only ores if not in ExtremeHills
        if (ore.extremeHillsOnly && !isExtremeHills) continue;

        int yRange = ore.yMax - ore.yMin;
        if (yRange <= 0) continue;

        for (int vein = 0; vein < ore.veinsPerChunk; ++vein) {
            uint32_t h = hash(seed, chunkX * 1000 + oreIdx, chunkZ * 1000 + vein, oreIdx * 7 + vein);

            int cx = static_cast<int>(h % CHUNK_WIDTH);
            int cy = ore.yMin + static_cast<int>((h >> 4) % static_cast<uint32_t>(yRange));
            int cz = static_cast<int>((h >> 8) % CHUNK_DEPTH);

            // Determine vein size for this particular vein
            int sizeRange = ore.maxSize - ore.minSize + 1;
            int veinSize = ore.minSize + static_cast<int>((h >> 12) % static_cast<uint32_t>(sizeRange));

            // Derive radius from vein size: small veins are tighter
            int radius = 1;
            if (veinSize > 4) radius = 2;
            if (veinSize > 10) radius = 3;

            int placed = 0;
            for (int dx = -radius; dx <= radius && placed < veinSize; ++dx) {
                for (int dy = -radius; dy <= radius && placed < veinSize; ++dy) {
                    for (int dz = -radius; dz <= radius && placed < veinSize; ++dz) {
                        if (dx * dx + dy * dy + dz * dz > radius * radius) continue;

                        int bx = cx + dx;
                        int by = cy + dy;
                        int bz = cz + dz;

                        if (bx < 0 || bx >= CHUNK_WIDTH ||
                            by < 0 || by >= CHUNK_HEIGHT ||
                            bz < 0 || bz >= CHUNK_DEPTH) continue;

                        if (chunk.getBlock(bx, by, bz) == BlockType::Stone) {
                            // Use hash to skip some blocks for irregular shape
                            uint32_t placeHash = hash(seed + 100, bx + chunkX * 16, by, bz + chunkZ * 16);
                            if ((placeHash % 3) != 0 || placed < ore.minSize) {
                                chunk.setBlock(bx, by, bz, ore.type);
                                ++placed;
                            }
                        }
                    }
                }
            }
        }
    }
}

} // namespace voxelforge
