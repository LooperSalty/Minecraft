#include "StructureGenerator.h"
#include "Chunk.h"
#include "Block.h"
#include "../util/Noise.h"
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace voxelforge {

// ============================================================
// Hash utility
// ============================================================

static uint32_t hash(int64_t seed, int a, int b, int c) {
    uint64_t h = static_cast<uint64_t>(seed);
    h = h * 6364136223846793005ULL + 1442695040888963407ULL + static_cast<uint64_t>(a);
    h = h * 6364136223846793005ULL + 1442695040888963407ULL + static_cast<uint64_t>(b);
    h = h * 6364136223846793005ULL + 1442695040888963407ULL + static_cast<uint64_t>(c);
    return static_cast<uint32_t>(h >> 16);
}

// ============================================================
// Tree helpers
// ============================================================

enum class TreeType { Oak, BigOak, Birch, Spruce, None };

static TreeType getTreeType(Biome biome, uint32_t rngVal) {
    switch (biome) {
        case Biome::Forest:
        case Biome::Plains:
            // 5% chance of big oak in forests/plains
            if ((rngVal % 100) < 5) return TreeType::BigOak;
            return TreeType::Oak;
        case Biome::Swamp:
            return TreeType::Oak;
        case Biome::BirchForest:
            return TreeType::Birch;
        case Biome::Taiga:
            return TreeType::Spruce;
        case Biome::ExtremeHills:
            // Mix of spruce and oak at lower elevations
            return ((rngVal % 3) == 0) ? TreeType::Spruce : TreeType::Oak;
        default:
            return TreeType::None;
    }
}

static bool isLeafOrLog(BlockType t) {
    return t == BlockType::OakLog || t == BlockType::BirchLog ||
           t == BlockType::SpruceLog || t == BlockType::OakLeaves ||
           t == BlockType::BirchLeaves || t == BlockType::SpruceLeaves;
}

static void placeOakTree(Chunk& chunk, int bx, int by, int bz,
                         BlockType logType, BlockType leafType,
                         int trunkHeight, uint32_t cornerSeed) {
    // Place trunk
    for (int ty = 1; ty <= trunkHeight; ++ty) {
        int py = by + ty;
        if (py >= CHUNK_HEIGHT) break;
        if (bx >= 0 && bx < CHUNK_WIDTH && bz >= 0 && bz < CHUNK_DEPTH) {
            chunk.setBlock(bx, py, bz, logType);
        }
    }

    // Leaves: sphere-ish 5x5x3 blob centered at trunk top
    int topY = by + trunkHeight;
    for (int ly = -1; ly <= 1; ++ly) {
        int py = topY + ly;
        if (py < 0 || py >= CHUNK_HEIGHT) continue;

        int radius = (ly == 0) ? 2 : 2; // full radius on all layers
        for (int lx = -radius; lx <= radius; ++lx) {
            for (int lz = -radius; lz <= radius; ++lz) {
                int px = bx + lx;
                int pz = bz + lz;

                if (px < 0 || px >= CHUNK_WIDTH || pz < 0 || pz >= CHUNK_DEPTH) continue;

                // Remove corners randomly for organic shape
                bool isCorner = (std::abs(lx) == 2 && std::abs(lz) == 2);
                if (isCorner) {
                    uint32_t ch = hash(cornerSeed, lx + px, ly + py, lz + pz);
                    if ((ch & 1) != 0) continue;
                }

                BlockType existing = chunk.getBlock(px, py, pz);
                if (existing == BlockType::Air || existing == leafType) {
                    chunk.setBlock(px, py, pz, leafType);
                }
            }
        }
    }

    // Top leaf layer (smaller)
    int topLeafY = topY + 2;
    if (topLeafY < CHUNK_HEIGHT) {
        for (int lx = -1; lx <= 1; ++lx) {
            for (int lz = -1; lz <= 1; ++lz) {
                int px = bx + lx;
                int pz = bz + lz;
                if (px < 0 || px >= CHUNK_WIDTH || pz < 0 || pz >= CHUNK_DEPTH) continue;
                bool isCorner = (std::abs(lx) == 1 && std::abs(lz) == 1);
                if (isCorner) {
                    uint32_t ch = hash(cornerSeed, lx + px, topLeafY, lz + pz);
                    if ((ch & 1) != 0) continue;
                }
                BlockType existing = chunk.getBlock(px, topLeafY, pz);
                if (existing == BlockType::Air) {
                    chunk.setBlock(px, topLeafY, pz, leafType);
                }
            }
        }
    }
}

static void placeBigOakTree(Chunk& chunk, int bx, int by, int bz,
                            int trunkHeight, uint32_t rng) {
    BlockType logType = BlockType::OakLog;
    BlockType leafType = BlockType::OakLeaves;

    // Thick trunk (2x2 if space allows)
    for (int ty = 1; ty <= trunkHeight; ++ty) {
        int py = by + ty;
        if (py >= CHUNK_HEIGHT) break;
        for (int dx = 0; dx <= 1; ++dx) {
            for (int dz = 0; dz <= 1; ++dz) {
                int px = bx + dx;
                int pz = bz + dz;
                if (px >= 0 && px < CHUNK_WIDTH && pz >= 0 && pz < CHUNK_DEPTH) {
                    chunk.setBlock(px, py, pz, logType);
                }
            }
        }
    }

    // Large crown: sphere of radius 4-5 at top
    int topY = by + trunkHeight;
    int crownRadius = 4;
    for (int ly = -2; ly <= 3; ++ly) {
        int py = topY + ly;
        if (py < 0 || py >= CHUNK_HEIGHT) continue;
        int layerR = crownRadius - std::abs(ly);
        if (layerR < 1) layerR = 1;
        for (int lx = -layerR; lx <= layerR; ++lx) {
            for (int lz = -layerR; lz <= layerR; ++lz) {
                if (lx * lx + lz * lz > layerR * layerR + 1) continue;
                int px = bx + lx;
                int pz = bz + lz;
                if (px < 0 || px >= CHUNK_WIDTH || pz < 0 || pz >= CHUNK_DEPTH) continue;
                BlockType existing = chunk.getBlock(px, py, pz);
                if (existing == BlockType::Air || existing == leafType) {
                    chunk.setBlock(px, py, pz, leafType);
                }
            }
        }
    }

    // Branches: 2-3 extending outward from upper trunk
    int branchCount = 2 + static_cast<int>(rng % 2);
    for (int b = 0; b < branchCount; ++b) {
        uint32_t branchRng = hash(rng, b, trunkHeight, bx);
        int branchY = by + trunkHeight - 2 - static_cast<int>(branchRng % 3);
        int dirX = (static_cast<int>(branchRng >> 4) % 3) - 1;
        int dirZ = (static_cast<int>(branchRng >> 8) % 3) - 1;
        if (dirX == 0 && dirZ == 0) dirX = 1;

        for (int step = 1; step <= 3; ++step) {
            int px = bx + dirX * step;
            int pz = bz + dirZ * step;
            int py = branchY + step / 2;
            if (px >= 0 && px < CHUNK_WIDTH && pz >= 0 && pz < CHUNK_DEPTH &&
                py >= 0 && py < CHUNK_HEIGHT) {
                chunk.setBlock(px, py, pz, logType);
            }
        }
    }
}

static void placeSpruceTree(Chunk& chunk, int bx, int by, int bz,
                            int trunkHeight, uint32_t cornerSeed) {
    BlockType logType = BlockType::SpruceLog;
    BlockType leafType = BlockType::SpruceLeaves;

    // Place trunk
    for (int ty = 1; ty <= trunkHeight; ++ty) {
        int py = by + ty;
        if (py >= CHUNK_HEIGHT) break;
        if (bx >= 0 && bx < CHUNK_WIDTH && bz >= 0 && bz < CHUNK_DEPTH) {
            chunk.setBlock(bx, py, bz, logType);
        }
    }

    // Triangular leaf shape: wide at bottom, narrow at top
    int leafStart = by + 2; // leaves start a couple blocks up
    int leafEnd = by + trunkHeight + 1;
    int totalLeafHeight = leafEnd - leafStart;

    for (int ly = 0; ly <= totalLeafHeight; ++ly) {
        int py = leafStart + ly;
        if (py >= CHUNK_HEIGHT) break;

        // Radius decreases from bottom to top
        float progress = static_cast<float>(ly) / static_cast<float>(totalLeafHeight);
        int radius = static_cast<int>((1.0f - progress) * 3.0f) + 1;
        // Alternate: every other layer is slightly narrower for spruce look
        if (ly % 2 == 1) radius = std::max(1, radius - 1);

        for (int lx = -radius; lx <= radius; ++lx) {
            for (int lz = -radius; lz <= radius; ++lz) {
                // Diamond shape
                if (std::abs(lx) + std::abs(lz) > radius) continue;
                int px = bx + lx;
                int pz = bz + lz;
                if (px < 0 || px >= CHUNK_WIDTH || pz < 0 || pz >= CHUNK_DEPTH) continue;

                BlockType existing = chunk.getBlock(px, py, pz);
                if (existing == BlockType::Air || existing == leafType) {
                    chunk.setBlock(px, py, pz, leafType);
                }
            }
        }
    }

    // Top spike
    int spikeY = leafEnd + 1;
    if (spikeY < CHUNK_HEIGHT && bx >= 0 && bx < CHUNK_WIDTH &&
        bz >= 0 && bz < CHUNK_DEPTH) {
        chunk.setBlock(bx, spikeY, bz, leafType);
    }
}

static void placeHugeMushroom(Chunk& chunk, int bx, int by, int bz,
                              int height, bool isBrown) {
    // Stem: white-ish (use Sandstone as placeholder for stem)
    for (int ty = 1; ty <= height; ++ty) {
        int py = by + ty;
        if (py >= CHUNK_HEIGHT) break;
        if (bx >= 0 && bx < CHUNK_WIDTH && bz >= 0 && bz < CHUNK_DEPTH) {
            chunk.setBlock(bx, py, bz, BlockType::OakPlanks); // stem placeholder
        }
    }

    // Cap
    int capY = by + height + 1;
    int capR = isBrown ? 3 : 2;
    BlockType capBlock = isBrown ? BlockType::MushroomBrown : BlockType::MushroomRed;

    if (isBrown) {
        // Flat cap for brown mushroom
        for (int dx = -capR; dx <= capR; ++dx) {
            for (int dz = -capR; dz <= capR; ++dz) {
                if (std::abs(dx) == capR && std::abs(dz) == capR) continue;
                int px = bx + dx;
                int pz = bz + dz;
                if (px >= 0 && px < CHUNK_WIDTH && pz >= 0 && pz < CHUNK_DEPTH &&
                    capY < CHUNK_HEIGHT) {
                    // Use dirt block with a mushroom-brown color to represent cap
                    // Since we don't have a huge mushroom cap block, use a solid block
                    chunk.setBlock(px, capY, pz, BlockType::Dirt);
                }
            }
        }
    } else {
        // Dome cap for red mushroom
        for (int dy = 0; dy <= 2; ++dy) {
            int py = capY + dy;
            if (py >= CHUNK_HEIGHT) continue;
            int layerR = capR - dy;
            if (layerR < 0) layerR = 0;
            for (int dx = -layerR; dx <= layerR; ++dx) {
                for (int dz = -layerR; dz <= layerR; ++dz) {
                    int px = bx + dx;
                    int pz = bz + dz;
                    if (px >= 0 && px < CHUNK_WIDTH && pz >= 0 && pz < CHUNK_DEPTH) {
                        chunk.setBlock(px, py, pz, BlockType::Dirt);
                    }
                }
            }
        }
    }
}

// ============================================================
// Tree generation
// ============================================================

void StructureGenerator::generateTrees(Chunk& chunk, int64_t seed,
                                       const BiomeGenerator& biomes,
                                       const TerrainGenerator& terrain) {
    int chunkX = chunk.getChunkX();
    int chunkZ = chunk.getChunkZ();

    // Sample biome at chunk center for tree density
    int centerWorldX = chunkX * CHUNK_WIDTH + CHUNK_WIDTH / 2;
    int centerWorldZ = chunkZ * CHUNK_DEPTH + CHUNK_DEPTH / 2;
    Biome centerBiome = biomes.getBiome(centerWorldX, centerWorldZ);
    const BiomeData& biomeData = getBiomeData(centerBiome);

    // Mushroom Island: generate huge mushrooms instead of trees
    if (centerBiome == Biome::MushroomIsland) {
        int maxMushrooms = 3;
        for (int i = 0; i < maxMushrooms; ++i) {
            uint32_t h = hash(seed + 9500, chunkX * 100 + i, chunkZ * 100 + i, i);
            int lx = static_cast<int>(h % CHUNK_WIDTH);
            int lz = static_cast<int>((h >> 8) % CHUNK_DEPTH);
            int worldX = chunkX * CHUNK_WIDTH + lx;
            int worldZ = chunkZ * CHUNK_DEPTH + lz;
            Biome localBiome = biomes.getBiome(worldX, worldZ);
            if (localBiome != Biome::MushroomIsland) continue;

            int surfaceY = terrain.getBlendedHeight(worldX, worldZ, biomes);
            if (surfaceY <= SEA_LEVEL || surfaceY >= CHUNK_HEIGHT - 12) continue;

            int mushroomHeight = 5 + static_cast<int>((h >> 16) % 4);
            bool isBrown = ((h >> 12) & 1) != 0;
            placeHugeMushroom(chunk, lx, surfaceY, lz, mushroomHeight, isBrown);
        }
        return;
    }

    int maxTrees = biomeData.treeDensity;
    if (maxTrees <= 0) return;

    for (int i = 0; i < maxTrees; ++i) {
        uint32_t h = hash(seed + 9000, chunkX * 100 + i, chunkZ * 100 + i, i);

        int lx = static_cast<int>(h % CHUNK_WIDTH);
        int lz = static_cast<int>((h >> 8) % CHUNK_DEPTH);

        int worldX = chunkX * CHUNK_WIDTH + lx;
        int worldZ = chunkZ * CHUNK_DEPTH + lz;

        Biome localBiome = biomes.getBiome(worldX, worldZ);
        TreeType treeType = getTreeType(localBiome, h);
        if (treeType == TreeType::None) continue;

        int surfaceY = terrain.getBlendedHeight(worldX, worldZ, biomes);
        if (surfaceY <= 0 || surfaceY >= CHUNK_HEIGHT - 15) continue;

        // Verify surface is suitable
        BlockType surfaceBlock = chunk.getBlock(lx, surfaceY, lz);
        if (surfaceBlock != BlockType::GrassBlock && surfaceBlock != BlockType::Dirt &&
            surfaceBlock != BlockType::Mycelium) {
            continue;
        }
        if (surfaceY + 1 >= CHUNK_HEIGHT) continue;
        BlockType aboveBlock = chunk.getBlock(lx, surfaceY + 1, lz);
        if (aboveBlock != BlockType::Air && !isLeafOrLog(aboveBlock)) continue;

        // Check for existing trees nearby (avoid overlap)
        bool tooClose = false;
        for (int dx = -2; dx <= 2 && !tooClose; ++dx) {
            for (int dz = -2; dz <= 2 && !tooClose; ++dz) {
                if (dx == 0 && dz == 0) continue;
                int cx = lx + dx;
                int cz = lz + dz;
                if (cx >= 0 && cx < CHUNK_WIDTH && cz >= 0 && cz < CHUNK_DEPTH) {
                    BlockType check = chunk.getBlock(cx, surfaceY + 1, cz);
                    if (check == BlockType::OakLog || check == BlockType::BirchLog ||
                        check == BlockType::SpruceLog) {
                        tooClose = true;
                    }
                }
            }
        }
        if (tooClose) continue;

        switch (treeType) {
            case TreeType::Oak: {
                int trunkH = 4 + static_cast<int>((h >> 16) % 3); // 4-6
                placeOakTree(chunk, lx, surfaceY, lz,
                            BlockType::OakLog, BlockType::OakLeaves, trunkH, h);
                break;
            }
            case TreeType::BigOak: {
                int trunkH = 8 + static_cast<int>((h >> 16) % 5); // 8-12
                placeBigOakTree(chunk, lx, surfaceY, lz, trunkH, h);
                break;
            }
            case TreeType::Birch: {
                int trunkH = 5 + static_cast<int>((h >> 16) % 3); // 5-7
                placeOakTree(chunk, lx, surfaceY, lz,
                            BlockType::BirchLog, BlockType::BirchLeaves, trunkH, h);
                break;
            }
            case TreeType::Spruce: {
                int trunkH = 6 + static_cast<int>((h >> 16) % 5); // 6-10
                placeSpruceTree(chunk, lx, surfaceY, lz, trunkH, h);
                break;
            }
            default:
                break;
        }
    }
}

// ============================================================
// Vegetation generation
// ============================================================

void StructureGenerator::generateVegetation(Chunk& chunk, int64_t seed,
                                             const BiomeGenerator& biomes,
                                             const TerrainGenerator& terrain) {
    int chunkX = chunk.getChunkX();
    int chunkZ = chunk.getChunkZ();
    int baseSeed = static_cast<int>(seed & 0x7FFFFFFF);

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            int worldX = chunkX * CHUNK_WIDTH + x;
            int worldZ = chunkZ * CHUNK_DEPTH + z;

            Biome biome = biomes.getBiome(worldX, worldZ);
            int surfaceY = terrain.getBlendedHeight(worldX, worldZ, biomes);

            if (surfaceY >= CHUNK_HEIGHT - 2) continue;

            BlockType surface = chunk.getBlock(x, surfaceY, z);
            BlockType above = chunk.getBlock(x, surfaceY + 1, z);

            float fx = static_cast<float>(worldX);
            float fz = static_cast<float>(worldZ);

            float vegNoise = octavePerlin2D(fx, fz, 3, 0.5f, 1.0f / 32.0f, baseSeed + 10000);
            float flowerNoise = octavePerlin2D(fx, fz, 3, 0.5f, 1.0f / 48.0f, baseSeed + 11000);

            uint32_t h = hash(seed + 12000, worldX, worldZ, surfaceY);
            float rng = static_cast<float>(h % 1000) / 1000.0f;

            // Snow layer on top of blocks in cold biomes
            if (biome == Biome::Taiga || biome == Biome::ExtremeHills) {
                bool shouldSnow = (biome == Biome::Taiga) ||
                                  (biome == Biome::ExtremeHills && surfaceY > 95);
                if (shouldSnow && above == BlockType::Air &&
                    surface != BlockType::Water && surface != BlockType::Air &&
                    surface != BlockType::SnowLayer) {
                    chunk.setBlock(x, surfaceY + 1, z, BlockType::SnowLayer);
                    continue;
                }
            }

            // Frozen water in Taiga
            if (biome == Biome::Taiga && surface == BlockType::Water) {
                chunk.setBlock(x, surfaceY, z, BlockType::Ice);
                continue;
            }

            // Skip if above is not air (tree trunk, snow, etc.)
            if (above != BlockType::Air) continue;

            // Skip underwater areas for vegetation (but handle lily pads separately)
            if (surfaceY < SEA_LEVEL && biome != Biome::Swamp) continue;

            switch (biome) {
                case Biome::Plains: {
                    if (surface != BlockType::GrassBlock) break;
                    if (vegNoise > 0.0f && rng < 0.30f) {
                        chunk.setBlock(x, surfaceY + 1, z, BlockType::TallGrass);
                    } else if (flowerNoise > 0.3f && rng < 0.05f) {
                        BlockType flower = ((h >> 10) & 1) ? BlockType::Poppy : BlockType::Dandelion;
                        chunk.setBlock(x, surfaceY + 1, z, flower);
                    }
                    break;
                }
                case Biome::Forest: {
                    if (surface != BlockType::GrassBlock) break;
                    if (vegNoise > 0.1f && rng < 0.20f) {
                        chunk.setBlock(x, surfaceY + 1, z, BlockType::TallGrass);
                    } else if (flowerNoise > 0.4f && rng < 0.03f) {
                        BlockType flower = ((h >> 10) & 1) ? BlockType::Poppy : BlockType::Dandelion;
                        chunk.setBlock(x, surfaceY + 1, z, flower);
                    } else if (vegNoise < -0.3f && rng < 0.02f) {
                        // Mushrooms on forest floor
                        BlockType mush = ((h >> 11) & 1) ? BlockType::MushroomRed : BlockType::MushroomBrown;
                        chunk.setBlock(x, surfaceY + 1, z, mush);
                    }
                    break;
                }
                case Biome::BirchForest: {
                    if (surface != BlockType::GrassBlock) break;
                    if (vegNoise > 0.1f && rng < 0.15f) {
                        chunk.setBlock(x, surfaceY + 1, z, BlockType::TallGrass);
                    } else if (flowerNoise > 0.4f && rng < 0.02f) {
                        BlockType flower = ((h >> 10) & 1) ? BlockType::Poppy : BlockType::Dandelion;
                        chunk.setBlock(x, surfaceY + 1, z, flower);
                    }
                    break;
                }
                case Biome::Desert: {
                    if (surface != BlockType::Sand) break;
                    if (vegNoise > 0.4f && rng < 0.03f) {
                        // Cactus: 1-3 blocks tall
                        int cactusH = 1 + static_cast<int>((h >> 4) % 3);
                        for (int cy = 1; cy <= cactusH; ++cy) {
                            if (surfaceY + cy < CHUNK_HEIGHT) {
                                chunk.setBlock(x, surfaceY + cy, z, BlockType::Cactus);
                            }
                        }
                    } else if (vegNoise < -0.3f && rng < 0.02f) {
                        // Dead bushes in desert
                        chunk.setBlock(x, surfaceY + 1, z, BlockType::DeadBush);
                    }
                    break;
                }
                case Biome::Swamp: {
                    if (surface == BlockType::Water && surfaceY == SEA_LEVEL) {
                        // Lily pads on water surface
                        if (vegNoise > 0.2f && rng < 0.08f) {
                            chunk.setBlock(x, surfaceY + 1, z, BlockType::LilyPad);
                        }
                    } else if (surface == BlockType::GrassBlock) {
                        if (vegNoise > -0.1f && rng < 0.20f) {
                            chunk.setBlock(x, surfaceY + 1, z, BlockType::TallGrass);
                        } else if (vegNoise < -0.4f && rng < 0.03f) {
                            BlockType mush = ((h >> 11) & 1) ? BlockType::MushroomRed : BlockType::MushroomBrown;
                            chunk.setBlock(x, surfaceY + 1, z, mush);
                        }
                        // Clay patches near sea level
                        if (surfaceY <= SEA_LEVEL + 2 && flowerNoise < -0.3f) {
                            chunk.setBlock(x, surfaceY, z, BlockType::Clay);
                        }
                    }
                    // Vines hanging from trees (check if there's a leaf/log above)
                    for (int vy = surfaceY + 3; vy < std::min(surfaceY + 10, CHUNK_HEIGHT); ++vy) {
                        BlockType blockAbove = chunk.getBlock(x, vy, z);
                        if (blockAbove == BlockType::OakLeaves || blockAbove == BlockType::OakLog) {
                            // Place vines hanging down
                            if (rng < 0.15f) {
                                for (int vd = 1; vd <= 3; ++vd) {
                                    int vineY = vy - vd;
                                    if (vineY > surfaceY + 1 && vineY < CHUNK_HEIGHT) {
                                        BlockType vineCheck = chunk.getBlock(x, vineY, z);
                                        if (vineCheck == BlockType::Air) {
                                            chunk.setBlock(x, vineY, z, BlockType::Vine);
                                        } else {
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
                case Biome::Ocean: {
                    // Ocean floor: clay and gravel patches
                    if (surface == BlockType::Dirt || surface == BlockType::Sand) {
                        if (flowerNoise > 0.3f) {
                            chunk.setBlock(x, surfaceY, z, BlockType::Clay);
                        } else if (flowerNoise < -0.3f) {
                            chunk.setBlock(x, surfaceY, z, BlockType::Gravel);
                        }
                    }
                    break;
                }
                case Biome::MushroomIsland: {
                    // Surface is mycelium (set in WorldGenerator)
                    if (surface == BlockType::Mycelium || surface == BlockType::GrassBlock) {
                        if (rng < 0.05f) {
                            BlockType mush = ((h >> 11) & 1) ? BlockType::MushroomRed : BlockType::MushroomBrown;
                            chunk.setBlock(x, surfaceY + 1, z, mush);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

// ============================================================
// Structure generation (villages, temples, dungeons, mineshafts)
// ============================================================

static void placeVillageHouse(Chunk& chunk, int baseX, int baseY, int baseZ,
                              int sizeX, int sizeZ, int height, uint32_t rng) {
    // Floor: oak planks
    for (int dx = 0; dx < sizeX; ++dx) {
        for (int dz = 0; dz < sizeZ; ++dz) {
            int px = baseX + dx;
            int pz = baseZ + dz;
            if (px >= 0 && px < CHUNK_WIDTH && pz >= 0 && pz < CHUNK_DEPTH) {
                chunk.setBlock(px, baseY, pz, BlockType::OakPlanks);
            }
        }
    }

    // Walls: cobblestone
    for (int dy = 1; dy <= height; ++dy) {
        int py = baseY + dy;
        if (py >= CHUNK_HEIGHT) break;
        for (int dx = 0; dx < sizeX; ++dx) {
            for (int dz = 0; dz < sizeZ; ++dz) {
                bool isWall = (dx == 0 || dx == sizeX - 1 || dz == 0 || dz == sizeZ - 1);
                if (!isWall) continue;
                int px = baseX + dx;
                int pz = baseZ + dz;
                if (px >= 0 && px < CHUNK_WIDTH && pz >= 0 && pz < CHUNK_DEPTH) {
                    chunk.setBlock(px, py, pz, BlockType::Cobblestone);
                }
            }
        }
    }

    // Door opening: 2 blocks high on one side
    int doorSide = static_cast<int>(rng % 4);
    int doorX = baseX, doorZ = baseZ;
    if (doorSide == 0) { doorX = baseX; doorZ = baseZ + sizeZ / 2; }
    else if (doorSide == 1) { doorX = baseX + sizeX - 1; doorZ = baseZ + sizeZ / 2; }
    else if (doorSide == 2) { doorX = baseX + sizeX / 2; doorZ = baseZ; }
    else { doorX = baseX + sizeX / 2; doorZ = baseZ + sizeZ - 1; }

    if (doorX >= 0 && doorX < CHUNK_WIDTH && doorZ >= 0 && doorZ < CHUNK_DEPTH) {
        for (int dy = 1; dy <= 2; ++dy) {
            int py = baseY + dy;
            if (py < CHUNK_HEIGHT) {
                chunk.setBlock(doorX, py, doorZ, BlockType::Air);
            }
        }
    }

    // Roof: oak planks (flat for simplicity, slightly raised)
    int roofY = baseY + height + 1;
    if (roofY < CHUNK_HEIGHT) {
        for (int dx = -1; dx <= sizeX; ++dx) {
            for (int dz = -1; dz <= sizeZ; ++dz) {
                int px = baseX + dx;
                int pz = baseZ + dz;
                if (px >= 0 && px < CHUNK_WIDTH && pz >= 0 && pz < CHUNK_DEPTH) {
                    chunk.setBlock(px, roofY, pz, BlockType::OakPlanks);
                }
            }
        }
    }

    // Interior clear
    for (int dy = 1; dy <= height; ++dy) {
        int py = baseY + dy;
        if (py >= CHUNK_HEIGHT) break;
        for (int dx = 1; dx < sizeX - 1; ++dx) {
            for (int dz = 1; dz < sizeZ - 1; ++dz) {
                int px = baseX + dx;
                int pz = baseZ + dz;
                if (px >= 0 && px < CHUNK_WIDTH && pz >= 0 && pz < CHUNK_DEPTH) {
                    chunk.setBlock(px, py, pz, BlockType::Air);
                }
            }
        }
    }
}

static void placeDesertTemple(Chunk& chunk, int baseX, int baseY, int baseZ) {
    // 9x9 base pyramid made of sandstone, stepping up
    int size = 9;
    int layers = 4;
    for (int layer = 0; layer < layers; ++layer) {
        int py = baseY + layer;
        if (py >= CHUNK_HEIGHT) break;
        int offset = layer;
        for (int dx = offset; dx < size - offset; ++dx) {
            for (int dz = offset; dz < size - offset; ++dz) {
                int px = baseX + dx;
                int pz = baseZ + dz;
                if (px >= 0 && px < CHUNK_WIDTH && pz >= 0 && pz < CHUNK_DEPTH) {
                    chunk.setBlock(px, py, pz, BlockType::Sandstone);
                }
            }
        }
    }

    // Hollow interior: clear inside at base level + 1
    for (int dx = 2; dx < size - 2; ++dx) {
        for (int dz = 2; dz < size - 2; ++dz) {
            int px = baseX + dx;
            int pz = baseZ + dz;
            if (px >= 0 && px < CHUNK_WIDTH && pz >= 0 && pz < CHUNK_DEPTH) {
                for (int dy = 1; dy < layers - 1; ++dy) {
                    int py = baseY + dy;
                    if (py < CHUNK_HEIGHT) {
                        chunk.setBlock(px, py, pz, BlockType::Air);
                    }
                }
            }
        }
    }

    // Entrance
    int entranceX = baseX + size / 2;
    int entranceZ = baseZ;
    if (entranceX >= 0 && entranceX < CHUNK_WIDTH &&
        entranceZ >= 0 && entranceZ < CHUNK_DEPTH) {
        for (int dy = 0; dy < 2; ++dy) {
            int py = baseY + dy;
            if (py < CHUNK_HEIGHT) {
                chunk.setBlock(entranceX, py, entranceZ, BlockType::Air);
            }
        }
    }
}

static void placeDungeon(Chunk& chunk, int baseX, int baseY, int baseZ) {
    int sizeX = 5, sizeZ = 5, height = 4;

    for (int dx = 0; dx < sizeX; ++dx) {
        for (int dz = 0; dz < sizeZ; ++dz) {
            for (int dy = 0; dy < height; ++dy) {
                int px = baseX + dx;
                int pz = baseZ + dz;
                int py = baseY + dy;
                if (px < 0 || px >= CHUNK_WIDTH || pz < 0 || pz >= CHUNK_DEPTH ||
                    py < 0 || py >= CHUNK_HEIGHT) continue;

                bool isWall = (dx == 0 || dx == sizeX - 1 ||
                               dz == 0 || dz == sizeZ - 1 ||
                               dy == 0 || dy == height - 1);
                if (isWall) {
                    // Mix cobblestone and mossy cobblestone
                    uint32_t wallHash = hash(0, px, py, pz);
                    BlockType wallBlock = (wallHash % 3 == 0) ?
                        BlockType::MossyCobblestone : BlockType::Cobblestone;
                    chunk.setBlock(px, py, pz, wallBlock);
                } else {
                    chunk.setBlock(px, py, pz, BlockType::Air);
                }
            }
        }
    }

    // Place a chest in the center
    int chestX = baseX + sizeX / 2;
    int chestZ = baseZ + sizeZ / 2;
    int chestY = baseY + 1;
    if (chestX >= 0 && chestX < CHUNK_WIDTH && chestZ >= 0 && chestZ < CHUNK_DEPTH &&
        chestY >= 0 && chestY < CHUNK_HEIGHT) {
        chunk.setBlock(chestX, chestY, chestZ, BlockType::ChestBlock);
    }
}

static void placeMineshaftSegment(Chunk& chunk, int startX, int startY, int startZ,
                                   int dirX, int dirZ, int length) {
    for (int i = 0; i < length; ++i) {
        int cx = startX + dirX * i;
        int cz = startZ + dirZ * i;

        // 3x3 tunnel
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = 0; dy <= 2; ++dy) {
                int px = cx + dx;
                int py = startY + dy;
                int pz = cz;
                if (dirX == 0) {
                    px = cx + dx;
                    pz = cz;
                } else {
                    px = cx;
                    pz = cz + dx;
                }

                if (px < 0 || px >= CHUNK_WIDTH || pz < 0 || pz >= CHUNK_DEPTH ||
                    py < 0 || py >= CHUNK_HEIGHT) continue;

                chunk.setBlock(px, py, pz, BlockType::Air);
            }
        }

        // Support beams every 4 blocks
        if (i % 4 == 0) {
            for (int dy = 0; dy <= 2; ++dy) {
                int py = startY + dy;
                // Left pillar
                int lx = cx + (dirX == 0 ? -2 : 0);
                int lz = cz + (dirX == 0 ? 0 : -2);
                if (lx >= 0 && lx < CHUNK_WIDTH && lz >= 0 && lz < CHUNK_DEPTH &&
                    py >= 0 && py < CHUNK_HEIGHT) {
                    chunk.setBlock(lx, py, lz, BlockType::OakPlanks);
                }
                // Right pillar
                int rx = cx + (dirX == 0 ? 2 : 0);
                int rz = cz + (dirX == 0 ? 0 : 2);
                if (rx >= 0 && rx < CHUNK_WIDTH && rz >= 0 && rz < CHUNK_DEPTH &&
                    py >= 0 && py < CHUNK_HEIGHT) {
                    chunk.setBlock(rx, py, rz, BlockType::OakPlanks);
                }
            }
            // Top beam
            int topY = startY + 3;
            if (topY < CHUNK_HEIGHT) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int px = cx + (dirX == 0 ? dx : 0);
                    int pz = cz + (dirX == 0 ? 0 : dx);
                    if (px >= 0 && px < CHUNK_WIDTH && pz >= 0 && pz < CHUNK_DEPTH) {
                        chunk.setBlock(px, topY, pz, BlockType::OakPlanks);
                    }
                }
            }
        }
    }
}

void StructureGenerator::generateStructures(Chunk& chunk, int64_t seed,
                                            const BiomeGenerator& biomes,
                                            const TerrainGenerator& terrain) {
    int chunkX = chunk.getChunkX();
    int chunkZ = chunk.getChunkZ();

    // Use chunk-level hash to decide what structures spawn
    uint32_t structHash = hash(seed + 20000, chunkX, chunkZ, 0);

    // Sample biome at chunk center
    int centerWorldX = chunkX * CHUNK_WIDTH + CHUNK_WIDTH / 2;
    int centerWorldZ = chunkZ * CHUNK_DEPTH + CHUNK_DEPTH / 2;
    Biome centerBiome = biomes.getBiome(centerWorldX, centerWorldZ);

    // ---- Villages (Plains biome, ~1 in 50 chunks) ----
    if (centerBiome == Biome::Plains && (structHash % 50) == 0) {
        int numHouses = 3 + static_cast<int>((structHash >> 8) % 3); // 3-5 houses
        for (int i = 0; i < numHouses; ++i) {
            uint32_t houseHash = hash(seed + 21000, chunkX * 10 + i, chunkZ * 10 + i, i);
            int lx = 2 + static_cast<int>(houseHash % (CHUNK_WIDTH - 8));
            int lz = 2 + static_cast<int>((houseHash >> 8) % (CHUNK_DEPTH - 8));
            int worldX = chunkX * CHUNK_WIDTH + lx;
            int worldZ = chunkZ * CHUNK_DEPTH + lz;
            int surfaceY = terrain.getBlendedHeight(worldX, worldZ, biomes);

            if (surfaceY <= SEA_LEVEL || surfaceY >= CHUNK_HEIGHT - 10) continue;

            int sizeX = 5;
            int sizeZ = 5;
            int wallH = 3 + static_cast<int>((houseHash >> 16) % 2); // 3-4

            placeVillageHouse(chunk, lx, surfaceY, lz, sizeX, sizeZ, wallH, houseHash);

            // One house gets a chest
            if (i == 0) {
                int chestX = lx + 1;
                int chestZ = lz + 1;
                int chestY = surfaceY + 1;
                if (chestX < CHUNK_WIDTH && chestZ < CHUNK_DEPTH && chestY < CHUNK_HEIGHT) {
                    chunk.setBlock(chestX, chestY, chestZ, BlockType::ChestBlock);
                }
            }
        }
    }

    // ---- Desert Temples (Desert biome, ~1 in 80 chunks) ----
    if (centerBiome == Biome::Desert && (structHash % 80) == 0) {
        int lx = 3 + static_cast<int>((structHash >> 4) % (CHUNK_WIDTH - 12));
        int lz = 3 + static_cast<int>((structHash >> 12) % (CHUNK_DEPTH - 12));
        int worldX = chunkX * CHUNK_WIDTH + lx;
        int worldZ = chunkZ * CHUNK_DEPTH + lz;
        int surfaceY = terrain.getBlendedHeight(worldX, worldZ, biomes);

        if (surfaceY > SEA_LEVEL && surfaceY < CHUNK_HEIGHT - 10) {
            placeDesertTemple(chunk, lx, surfaceY, lz);
        }
    }

    // ---- Underground Dungeons (~1 in 100 chunks, below Y=40) ----
    if ((structHash % 100) == 1) {
        uint32_t dungHash = hash(seed + 22000, chunkX, chunkZ, 1);
        int dx = 2 + static_cast<int>(dungHash % (CHUNK_WIDTH - 7));
        int dz = 2 + static_cast<int>((dungHash >> 8) % (CHUNK_DEPTH - 7));
        int dy = 10 + static_cast<int>((dungHash >> 16) % 25); // Y 10-35

        // Only place if surrounded by stone (in a cave area)
        BlockType checkBlock = chunk.getBlock(dx, dy, dz);
        if (checkBlock == BlockType::Air || checkBlock == BlockType::Stone) {
            placeDungeon(chunk, dx, dy, dz);
        }
    }

    // ---- Mineshafts (~1 in 60 chunks, below Y=50) ----
    if ((structHash % 60) == 2) {
        uint32_t mineHash = hash(seed + 23000, chunkX, chunkZ, 2);
        int mx = static_cast<int>(mineHash % CHUNK_WIDTH);
        int mz = static_cast<int>((mineHash >> 8) % CHUNK_DEPTH);
        int my = 15 + static_cast<int>((mineHash >> 16) % 30); // Y 15-45

        // Main corridor in X direction
        int corridorLen = 8 + static_cast<int>((mineHash >> 4) % 8); // 8-15 blocks
        placeMineshaftSegment(chunk, mx, my, mz, 1, 0,
                              std::min(corridorLen, CHUNK_WIDTH - mx));

        // Branch corridor in Z direction
        uint32_t branchHash = hash(seed + 23100, chunkX, chunkZ, 3);
        int branchStart = static_cast<int>(branchHash % static_cast<uint32_t>(corridorLen));
        int branchX = mx + branchStart;
        if (branchX < CHUNK_WIDTH) {
            int branchLen = 6 + static_cast<int>((branchHash >> 8) % 6);
            placeMineshaftSegment(chunk, branchX, my, mz, 0, 1,
                                  std::min(branchLen, CHUNK_DEPTH - mz));
        }
    }
}

} // namespace voxelforge
