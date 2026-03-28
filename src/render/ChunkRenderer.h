#pragma once
#include "ChunkMesher.h"
#include <glad/gl.h>

namespace voxelforge {

class ChunkRenderer {
public:
    ChunkRenderer() = default;
    ~ChunkRenderer();

    ChunkRenderer(const ChunkRenderer&) = delete;
    ChunkRenderer& operator=(const ChunkRenderer&) = delete;
    ChunkRenderer(ChunkRenderer&& o) noexcept;
    ChunkRenderer& operator=(ChunkRenderer&& o) noexcept;

    void upload(const ChunkMeshPair& meshPair);
    void render() const;
    void renderTransparent() const;
    bool hasData() const { return m_indexCount > 0; }
    bool hasTransparentData() const { return m_transIndexCount > 0; }

private:
    void cleanup();

    // Opaque mesh
    GLuint   m_vao = 0;
    GLuint   m_vbo = 0;
    GLuint   m_ebo = 0;
    uint32_t m_indexCount = 0;

    // Transparent mesh
    GLuint   m_transVao = 0;
    GLuint   m_transVbo = 0;
    GLuint   m_transEbo = 0;
    uint32_t m_transIndexCount = 0;
};

} // namespace voxelforge
