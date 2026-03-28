#include "ChunkManager.h"
#include "Chunk.h"
#include "../render/ChunkMesher.h"
#include <cmath>
#include <algorithm>

namespace voxelforge {

// ---------- helpers ----------

uint64_t ChunkManager::key(int cx, int cz) {
    return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32)
         | static_cast<uint64_t>(static_cast<uint32_t>(cz));
}

// ---------- construction ----------

ChunkManager::ChunkManager(int64_t seed, int renderDistance)
    : m_generator(seed)
    , m_renderDist(renderDistance)
{}

// ---------- queries ----------

Chunk* ChunkManager::getChunk(int cx, int cz) const {
    auto it = m_chunks.find(key(cx, cz));
    if (it == m_chunks.end()) {
        return nullptr;
    }
    return it->second.chunk.get();
}

ChunkNeighbors ChunkManager::getNeighbors(int cx, int cz) const {
    return {
        getChunk(cx - 1, cz),  // negX (west)
        getChunk(cx + 1, cz),  // posX (east)
        getChunk(cx, cz - 1),  // negZ (north)
        getChunk(cx, cz + 1),  // posZ (south)
    };
}

// ---------- chunk loading ----------

void ChunkManager::ensureChunk(int cx, int cz) {
    uint64_t k = key(cx, cz);
    if (m_chunks.count(k) != 0) {
        return;
    }

    LoadedChunk lc;
    lc.chunk = std::make_unique<Chunk>(cx, cz);
    m_generator.generate(*lc.chunk);
    lc.meshDirty = true;

    m_chunks.emplace(k, std::move(lc));

    // Mark neighbor meshes as dirty so seams are fixed
    auto markDirty = [&](int nx, int nz) {
        auto it = m_chunks.find(key(nx, nz));
        if (it != m_chunks.end()) {
            it->second.meshDirty = true;
        }
    };
    markDirty(cx - 1, cz);
    markDirty(cx + 1, cz);
    markDirty(cx, cz - 1);
    markDirty(cx, cz + 1);
}

void ChunkManager::unloadDistant(int centerCX, int centerCZ) {
    auto it = m_chunks.begin();
    while (it != m_chunks.end()) {
        uint64_t k = it->first;
        int cx = static_cast<int>(static_cast<uint32_t>(k >> 32));
        int cz = static_cast<int>(static_cast<uint32_t>(k & 0xFFFFFFFF));

        int dx = std::abs(cx - centerCX);
        int dz = std::abs(cz - centerCZ);

        if (dx > m_renderDist + 2 || dz > m_renderDist + 2) {
            it = m_chunks.erase(it);
        } else {
            ++it;
        }
    }
}

// ---------- mesh building ----------

void ChunkManager::rebuildMesh(LoadedChunk& lc) {
    int cx = lc.chunk->getChunkX();
    int cz = lc.chunk->getChunkZ();
    ChunkNeighbors nb = getNeighbors(cx, cz);

    ChunkMeshPair meshPair = ChunkMesher::generateMesh(*lc.chunk, nb);
    lc.renderer.upload(meshPair);
    lc.meshDirty = false;
}

// ---------- per-frame update ----------

void ChunkManager::update(const glm::vec3& playerPos) {
    int centerCX = static_cast<int>(std::floor(playerPos.x / CHUNK_WIDTH));
    int centerCZ = static_cast<int>(std::floor(playerPos.z / CHUNK_DEPTH));

    // Load chunks in a square around the player
    for (int dx = -m_renderDist; dx <= m_renderDist; ++dx) {
        for (int dz = -m_renderDist; dz <= m_renderDist; ++dz) {
            ensureChunk(centerCX + dx, centerCZ + dz);
        }
    }

    // Rebuild dirty meshes (limit to 4 per frame to avoid stuttering)
    int rebuilds = 0;
    constexpr int MAX_REBUILDS_PER_FRAME = 4;

    for (auto& [k, lc] : m_chunks) {
        if (!lc.meshDirty) {
            continue;
        }
        if (rebuilds >= MAX_REBUILDS_PER_FRAME) {
            break;
        }
        rebuildMesh(lc);
        ++rebuilds;
    }

    // Unload chunks too far away
    unloadDistant(centerCX, centerCZ);
}

// ---------- rendering ----------

void ChunkManager::renderAll(const Shader& shader, const Frustum& frustum) const {
    for (const auto& [k, lc] : m_chunks) {
        if (!lc.renderer.hasData()) {
            continue;
        }

        int cx = lc.chunk->getChunkX();
        int cz = lc.chunk->getChunkZ();

        glm::vec3 minPt(
            static_cast<float>(cx * CHUNK_WIDTH),
            0.0f,
            static_cast<float>(cz * CHUNK_DEPTH));
        glm::vec3 maxPt(
            static_cast<float>(cx * CHUNK_WIDTH + CHUNK_WIDTH),
            static_cast<float>(CHUNK_HEIGHT),
            static_cast<float>(cz * CHUNK_DEPTH + CHUNK_DEPTH));

        if (!frustum.isBoxVisible(minPt, maxPt)) {
            continue;
        }

        lc.renderer.render();
    }
}

void ChunkManager::renderTransparent(const Shader& shader, const Frustum& frustum) const {
    for (const auto& [k, lc] : m_chunks) {
        if (!lc.renderer.hasTransparentData()) {
            continue;
        }

        int cx = lc.chunk->getChunkX();
        int cz = lc.chunk->getChunkZ();

        glm::vec3 minPt(
            static_cast<float>(cx * CHUNK_WIDTH),
            0.0f,
            static_cast<float>(cz * CHUNK_DEPTH));
        glm::vec3 maxPt(
            static_cast<float>(cx * CHUNK_WIDTH + CHUNK_WIDTH),
            static_cast<float>(CHUNK_HEIGHT),
            static_cast<float>(cz * CHUNK_DEPTH + CHUNK_DEPTH));

        if (!frustum.isBoxVisible(minPt, maxPt)) {
            continue;
        }

        lc.renderer.renderTransparent();
    }
}

// ---------- world-coordinate block access ----------

BlockType ChunkManager::getBlock(int wx, int wy, int wz) const {
    if (wy < 0 || wy >= CHUNK_HEIGHT) return BlockType::Air;

    int cx = (wx >= 0) ? (wx / CHUNK_WIDTH) : ((wx - CHUNK_WIDTH + 1) / CHUNK_WIDTH);
    int cz = (wz >= 0) ? (wz / CHUNK_DEPTH) : ((wz - CHUNK_DEPTH + 1) / CHUNK_DEPTH);

    Chunk* c = getChunk(cx, cz);
    if (!c) return BlockType::Air;

    int lx = wx - cx * CHUNK_WIDTH;
    int lz = wz - cz * CHUNK_DEPTH;
    return c->getBlock(lx, wy, lz);
}

void ChunkManager::setBlock(int wx, int wy, int wz, BlockType type) {
    if (wy < 0 || wy >= CHUNK_HEIGHT) return;

    int cx = (wx >= 0) ? (wx / CHUNK_WIDTH) : ((wx - CHUNK_WIDTH + 1) / CHUNK_WIDTH);
    int cz = (wz >= 0) ? (wz / CHUNK_DEPTH) : ((wz - CHUNK_DEPTH + 1) / CHUNK_DEPTH);

    auto it = m_chunks.find(key(cx, cz));
    if (it == m_chunks.end()) return;

    int lx = wx - cx * CHUNK_WIDTH;
    int lz = wz - cz * CHUNK_DEPTH;
    it->second.chunk->setBlock(lx, wy, lz, type);
    it->second.meshDirty = true;

    // Mark neighbor chunks dirty if block is on edge
    if (lx == 0)             { auto n = m_chunks.find(key(cx - 1, cz)); if (n != m_chunks.end()) n->second.meshDirty = true; }
    if (lx == CHUNK_WIDTH-1) { auto n = m_chunks.find(key(cx + 1, cz)); if (n != m_chunks.end()) n->second.meshDirty = true; }
    if (lz == 0)             { auto n = m_chunks.find(key(cx, cz - 1)); if (n != m_chunks.end()) n->second.meshDirty = true; }
    if (lz == CHUNK_DEPTH-1) { auto n = m_chunks.find(key(cx, cz + 1)); if (n != m_chunks.end()) n->second.meshDirty = true; }
}

} // namespace voxelforge
