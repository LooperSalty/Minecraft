#pragma once
#include "Chunk.h"
#include "WorldGenerator.h"
#include "../render/ChunkMesher.h"
#include "../render/ChunkRenderer.h"
#include "../render/Frustum.h"
#include "../render/Shader.h"
#include <unordered_map>
#include <memory>

namespace voxelforge {

class ChunkManager {
public:
    ChunkManager(int64_t seed, int renderDistance);

    void update(const glm::vec3& playerPos);
    void preloadSpawnArea(const glm::vec3& spawnPos);
    void renderAll(const Shader& shader, const Frustum& frustum) const;
    void renderTransparent(const Shader& shader, const Frustum& frustum) const;

    Chunk* getChunk(int cx, int cz) const;
    int getLoadedCount() const { return static_cast<int>(m_chunks.size()); }

    // World-coordinate block access
    BlockType getBlock(int wx, int wy, int wz) const;
    void setBlock(int wx, int wy, int wz, BlockType type);

private:
    struct LoadedChunk {
        std::unique_ptr<Chunk> chunk;
        ChunkRenderer renderer;
        bool meshDirty = true;
    };

    static uint64_t key(int cx, int cz);
    void ensureChunk(int cx, int cz);
    void unloadDistant(int centerCX, int centerCZ);
    void rebuildMesh(LoadedChunk& lc);
    ChunkNeighbors getNeighbors(int cx, int cz) const;

    std::unordered_map<uint64_t, LoadedChunk> m_chunks;
    WorldGenerator m_generator;
    int m_renderDist;
};

} // namespace voxelforge
