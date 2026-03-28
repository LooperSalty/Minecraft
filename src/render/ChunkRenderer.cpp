#include "ChunkRenderer.h"

namespace voxelforge {

ChunkRenderer::~ChunkRenderer() { cleanup(); }

ChunkRenderer::ChunkRenderer(ChunkRenderer&& o) noexcept
    : m_vao(o.m_vao), m_vbo(o.m_vbo), m_ebo(o.m_ebo), m_indexCount(o.m_indexCount)
{
    o.m_vao = o.m_vbo = o.m_ebo = 0;
    o.m_indexCount = 0;
}

ChunkRenderer& ChunkRenderer::operator=(ChunkRenderer&& o) noexcept {
    if (this != &o) {
        cleanup();
        m_vao = o.m_vao;  m_vbo = o.m_vbo;  m_ebo = o.m_ebo;
        m_indexCount = o.m_indexCount;
        o.m_vao = o.m_vbo = o.m_ebo = 0;
        o.m_indexCount = 0;
    }
    return *this;
}

void ChunkRenderer::upload(const ChunkMesh& mesh) {
    cleanup();
    if (mesh.isEmpty()) return;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.vertices.size() * sizeof(Vertex),
                 mesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.indices.size() * sizeof(uint32_t),
                 mesh.indices.data(), GL_STATIC_DRAW);

    // location 0 : position (vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, x)));
    glEnableVertexAttribArray(0);

    // location 1 : texcoord (vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, u)));
    glEnableVertexAttribArray(1);

    // location 2 : light (float)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, light)));
    glEnableVertexAttribArray(2);

    // location 3 : normal (vec3)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, nx)));
    glEnableVertexAttribArray(3);

    // location 4 : ambient occlusion (float)
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, ao)));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);
    m_indexCount = static_cast<uint32_t>(mesh.indices.size());
}

void ChunkRenderer::render() const {
    if (m_indexCount == 0) return;
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void ChunkRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_ebo) { glDeleteBuffers(1, &m_ebo); m_ebo = 0; }
    m_indexCount = 0;
}

} // namespace voxelforge
