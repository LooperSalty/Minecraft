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

    void upload(const ChunkMesh& mesh);
    void render() const;
    bool hasData() const { return m_indexCount > 0; }

private:
    void cleanup();

    GLuint   m_vao = 0;
    GLuint   m_vbo = 0;
    GLuint   m_ebo = 0;
    uint32_t m_indexCount = 0;
};

} // namespace voxelforge
