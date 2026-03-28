#pragma once
#include <glad/gl.h>
#include <cstdint>

namespace voxelforge {

constexpr int ATLAS_SIZE    = 512;  // 512x512 pixels (32x32 tiles)
constexpr int TILE_SIZE     = 16;   // each texture 16x16
constexpr int TILES_PER_ROW = ATLAS_SIZE / TILE_SIZE;

class TextureAtlas {
public:
    TextureAtlas() = default;
    ~TextureAtlas();

    TextureAtlas(const TextureAtlas&) = delete;
    TextureAtlas& operator=(const TextureAtlas&) = delete;

    void generate();
    void bind(int unit = 0) const;

    static void getUV(uint8_t textureIndex, float& u0, float& v0, float& u1, float& v1);

private:
    void fillTile(uint8_t* data, int tileX, int tileY,
                  uint8_t r, uint8_t g, uint8_t b);

    // Pixel-level helpers
    static void setPixel(uint8_t* data, int tileX, int tileY,
                         int px, int py,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    // Simple hash-based noise for deterministic variation
    static int hash(int x, int y, int seed);
    static float noise2D(int x, int y, int seed);

    GLuint m_texture = 0;
};

} // namespace voxelforge
