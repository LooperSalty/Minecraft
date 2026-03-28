#include "WorldGenerator.h"
#include "Chunk.h"
#include "Block.h"
#include "OreGenerator.h"
#include "StructureGenerator.h"

namespace voxelforge {

WorldGenerator::WorldGenerator(int64_t seed)
    : m_seed(seed)
    , m_biomes(seed)
    , m_terrain(seed) {}

void WorldGenerator::generate(Chunk& chunk) const {
    int chunkX = chunk.getChunkX();
    int chunkZ = chunk.getChunkZ();

    // Pass 1: fill terrain columns
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            int worldX = chunkX * CHUNK_WIDTH + x;
            int worldZ = chunkZ * CHUNK_DEPTH + z;

            Biome biome = m_biomes.getBiome(worldX, worldZ);
            int height = m_terrain.getHeight(worldX, worldZ, biome);

            bool isDesert = (biome == Biome::Desert || biome == Biome::Beach);

            // Bedrock layer: Y 0-4
            for (int y = 0; y <= 4; ++y) {
                chunk.setBlock(x, y, z, BlockType::Bedrock);
            }

            // Stone layer: Y 5 to (height - 4)
            int stoneTop = height - 4;
            for (int y = 5; y <= stoneTop; ++y) {
                chunk.setBlock(x, y, z, BlockType::Stone);
            }

            // Desert sandstone underlay: Y (height-7) to (height-4)
            if (biome == Biome::Desert) {
                int sandstoneBottom = height - 7;
                int sandstoneTop = height - 4;
                for (int y = sandstoneBottom; y <= sandstoneTop; ++y) {
                    if (y >= 5) {
                        chunk.setBlock(x, y, z, BlockType::Sandstone);
                    }
                }
            }

            // Filler layer: Y (height-3) to (height-1)
            BlockType fillerBlock = isDesert ? BlockType::Sand : BlockType::Dirt;
            for (int y = height - 3; y <= height - 1; ++y) {
                if (y >= 5) {
                    chunk.setBlock(x, y, z, fillerBlock);
                }
            }

            // Surface block: Y = height
            BlockType surfaceBlock = BlockType::GrassBlock;
            if (isDesert) {
                surfaceBlock = BlockType::Sand;
            } else if (biome == Biome::ExtremeHills && height > 90) {
                surfaceBlock = BlockType::SnowBlock;
            } else if (biome == Biome::Taiga) {
                surfaceBlock = BlockType::Dirt;
            }

            if (height >= 1) {
                chunk.setBlock(x, height, z, surfaceBlock);
            }

            // Water fill: if surface is below sea level
            if (height < SEA_LEVEL) {
                for (int y = height + 1; y <= SEA_LEVEL; ++y) {
                    chunk.setBlock(x, y, z, BlockType::Water);
                }
            }
        }
    }

    // Pass 2: carve caves
    m_terrain.carveCaves(chunk);

    // Pass 3: generate ores
    OreGenerator::generate(chunk, m_seed);

    // Pass 4: generate trees
    StructureGenerator::generateTrees(chunk, m_seed, m_biomes, m_terrain);
}

} // namespace voxelforge
