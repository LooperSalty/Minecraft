#include "TextureAtlas.h"
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <cmath>

namespace voxelforge {

TextureAtlas::~TextureAtlas() {
    if (m_texture) glDeleteTextures(1, &m_texture);
}

void TextureAtlas::fillTile(uint8_t* data, int tileX, int tileY,
                             uint8_t r, uint8_t g, uint8_t b)
{
    int bx = tileX * TILE_SIZE;
    int by = tileY * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            int idx = ((by + y) * ATLAS_SIZE + (bx + x)) * 4;
            int n = (std::rand() % 20) - 10;
            data[idx + 0] = static_cast<uint8_t>(std::clamp(int(r) + n, 0, 255));
            data[idx + 1] = static_cast<uint8_t>(std::clamp(int(g) + n, 0, 255));
            data[idx + 2] = static_cast<uint8_t>(std::clamp(int(b) + n, 0, 255));
            data[idx + 3] = 255;
        }
    }
}

void TextureAtlas::generate() {
    std::vector<uint8_t> px(ATLAS_SIZE * ATLAS_SIZE * 4, 255);
    std::srand(42);

    //  0 Stone
    fillTile(px.data(), 0, 0, 128, 128, 128);
    //  1 Dirt
    fillTile(px.data(), 1, 0, 134,  96,  67);
    //  2 Grass top
    fillTile(px.data(), 2, 0,  91, 148,  59);
    //  3 Grass side (green strip on top, dirt below)
    {
        int bx = 3 * TILE_SIZE, by = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                int idx = ((by + y) * ATLAS_SIZE + (bx + x)) * 4;
                int n = (std::rand() % 16) - 8;
                if (y < 4) { // green strip
                    px[idx+0]=uint8_t(std::clamp(91+n,0,255));
                    px[idx+1]=uint8_t(std::clamp(148+n,0,255));
                    px[idx+2]=uint8_t(std::clamp(59+n,0,255));
                } else {     // dirt
                    px[idx+0]=uint8_t(std::clamp(134+n,0,255));
                    px[idx+1]=uint8_t(std::clamp(96+n,0,255));
                    px[idx+2]=uint8_t(std::clamp(67+n,0,255));
                }
                px[idx+3]=255;
            }
        }
    }
    //  4 Cobblestone
    fillTile(px.data(), 4, 0, 110, 110, 110);
    //  5 Oak log top (rings)
    {
        int bx = 5 * TILE_SIZE, by = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                int idx = ((by + y) * ATLAS_SIZE + (bx + x)) * 4;
                float dx = x - 7.5f, dy = y - 7.5f;
                float d = std::sqrt(dx*dx + dy*dy);
                int n = (std::rand() % 10) - 5;
                bool ring = (int(d) % 3 == 0);
                uint8_t base = ring ? 140 : 170;
                px[idx+0]=uint8_t(std::clamp(int(base)+n, 0, 255));
                px[idx+1]=uint8_t(std::clamp(int(base)-20+n, 0, 255));
                px[idx+2]=uint8_t(std::clamp(int(base)-50+n, 0, 255));
                px[idx+3]=255;
            }
        }
    }
    //  6 Oak log side
    fillTile(px.data(), 6, 0, 102,  76,  40);
    //  7 Oak planks
    fillTile(px.data(), 7, 0, 180, 144,  90);
    //  8 Bedrock
    fillTile(px.data(), 8, 0,  50,  50,  50);
    //  9 Sand
    fillTile(px.data(), 9, 0, 219, 211, 160);
    // 10 Gravel
    fillTile(px.data(),10, 0, 136, 126, 122);
    // 11 Water
    {
        int bx = 11 * TILE_SIZE, by = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                int idx = ((by + y) * ATLAS_SIZE + (bx + x)) * 4;
                int n = (std::rand() % 14) - 7;
                px[idx+0]=uint8_t(std::clamp(30+n,0,255));
                px[idx+1]=uint8_t(std::clamp(100+n,0,255));
                px[idx+2]=uint8_t(std::clamp(200+n,0,255));
                px[idx+3]=180;
            }
        }
    }
    // 12 Lava
    fillTile(px.data(),12, 0, 207,  91,  20);

    // 13 OakLeaves (high noise for leafy variation)
    {
        int bx = 13 * TILE_SIZE, by = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                int idx = ((by + y) * ATLAS_SIZE + (bx + x)) * 4;
                int n = (std::rand() % 40) - 20;
                px[idx+0]=uint8_t(std::clamp(56+n, 0, 255));
                px[idx+1]=uint8_t(std::clamp(118+n, 0, 255));
                px[idx+2]=uint8_t(std::clamp(29+n, 0, 255));
                px[idx+3]=255;
            }
        }
    }
    // 14 BirchLog side (light bark with dark vertical streaks)
    {
        int bx = 14 * TILE_SIZE, by = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                int idx = ((by + y) * ATLAS_SIZE + (bx + x)) * 4;
                int n = (std::rand() % 10) - 5;
                bool streak = (x % 4 == 0 || x % 7 == 0);
                int dark = streak ? -40 : 0;
                px[idx+0]=uint8_t(std::clamp(200+dark+n, 0, 255));
                px[idx+1]=uint8_t(std::clamp(200+dark+n, 0, 255));
                px[idx+2]=uint8_t(std::clamp(195+dark+n, 0, 255));
                px[idx+3]=255;
            }
        }
    }
    // 15 BirchLog top (ring pattern, lighter than oak)
    {
        int bx = 15 * TILE_SIZE, by = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                int idx = ((by + y) * ATLAS_SIZE + (bx + x)) * 4;
                float dx = x - 7.5f, dy = y - 7.5f;
                float d = std::sqrt(dx*dx + dy*dy);
                int n = (std::rand() % 10) - 5;
                bool ring = (int(d) % 3 == 0);
                uint8_t base = ring ? 160 : 190;
                px[idx+0]=uint8_t(std::clamp(int(base)+n, 0, 255));
                px[idx+1]=uint8_t(std::clamp(int(base)-10+n, 0, 255));
                px[idx+2]=uint8_t(std::clamp(int(base)-40+n, 0, 255));
                px[idx+3]=255;
            }
        }
    }
    // 16 BirchLeaves (brighter green with noise)
    {
        int bx = 16 * TILE_SIZE, by = 0;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                int idx = ((by + y) * ATLAS_SIZE + (bx + x)) * 4;
                int n = (std::rand() % 30) - 15;
                px[idx+0]=uint8_t(std::clamp(100+n, 0, 255));
                px[idx+1]=uint8_t(std::clamp(160+n, 0, 255));
                px[idx+2]=uint8_t(std::clamp(70+n, 0, 255));
                px[idx+3]=255;
            }
        }
    }
    // 17 SpruceLog side (dark brown bark)
    fillTile(px.data(), 1, 1, 70, 45, 20);
    // 18 SpruceLeaves (dark green)
    {
        int bx = 2 * TILE_SIZE, by = 1 * TILE_SIZE;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                int idx = ((by + y) * ATLAS_SIZE + (bx + x)) * 4;
                int n = (std::rand() % 20) - 10;
                px[idx+0]=uint8_t(std::clamp(30+n, 0, 255));
                px[idx+1]=uint8_t(std::clamp(80+n, 0, 255));
                px[idx+2]=uint8_t(std::clamp(25+n, 0, 255));
                px[idx+3]=255;
            }
        }
    }

    // Ore helper: stone base with colored spots
    auto fillOreTile = [&](int tileX, int tileY,
                           uint8_t sr, uint8_t sg, uint8_t sb) {
        int bx = tileX * TILE_SIZE, by = tileY * TILE_SIZE;
        // Fill with stone base
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                int idx = ((by + y) * ATLAS_SIZE + (bx + x)) * 4;
                int n = (std::rand() % 20) - 10;
                px[idx+0]=uint8_t(std::clamp(128+n, 0, 255));
                px[idx+1]=uint8_t(std::clamp(128+n, 0, 255));
                px[idx+2]=uint8_t(std::clamp(128+n, 0, 255));
                px[idx+3]=255;
            }
        }
        // Add 7 colored ore spots
        for (int s = 0; s < 7; ++s) {
            int sx = std::rand() % TILE_SIZE;
            int sy = std::rand() % TILE_SIZE;
            for (int dy = 0; dy < 2 && sy+dy < TILE_SIZE; ++dy) {
                for (int dx = 0; dx < 2 && sx+dx < TILE_SIZE; ++dx) {
                    int idx = ((by+sy+dy) * ATLAS_SIZE + (bx+sx+dx)) * 4;
                    int n = (std::rand() % 20) - 10;
                    px[idx+0]=uint8_t(std::clamp(int(sr)+n, 0, 255));
                    px[idx+1]=uint8_t(std::clamp(int(sg)+n, 0, 255));
                    px[idx+2]=uint8_t(std::clamp(int(sb)+n, 0, 255));
                }
            }
        }
    };

    // 19 CoalOre
    fillOreTile(3, 1, 30, 30, 30);
    // 20 IronOre
    fillOreTile(4, 1, 180, 140, 100);
    // 21 GoldOre
    fillOreTile(5, 1, 220, 200, 50);
    // 22 DiamondOre
    fillOreTile(6, 1, 80, 220, 220);
    // 23 RedstoneOre
    fillOreTile(7, 1, 200, 30, 30);
    // 24 LapisOre
    fillOreTile(8, 1, 30, 50, 180);
    // 25 EmeraldOre
    fillOreTile(9, 1, 30, 180, 60);

    // 26 SnowBlock (white with very slight noise)
    {
        int bx = 10 * TILE_SIZE, by = 1 * TILE_SIZE;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                int idx = ((by + y) * ATLAS_SIZE + (bx + x)) * 4;
                int n = (std::rand() % 6) - 3;
                px[idx+0]=uint8_t(std::clamp(240+n, 0, 255));
                px[idx+1]=uint8_t(std::clamp(245+n, 0, 255));
                px[idx+2]=uint8_t(std::clamp(250+n, 0, 255));
                px[idx+3]=255;
            }
        }
    }
    // 27 Sandstone (tan with noise)
    fillTile(px.data(), 11, 1, 210, 195, 150);

    // Upload
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_SIZE, ATLAS_SIZE,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void TextureAtlas::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_texture);
}

void TextureAtlas::getUV(uint8_t texIdx, float& u0, float& v0, float& u1, float& v1) {
    int col = texIdx % TILES_PER_ROW;
    int row = texIdx / TILES_PER_ROW;
    constexpr float s = 1.0f / TILES_PER_ROW;
    u0 = col * s;  v0 = row * s;
    u1 = u0 + s;   v1 = v0 + s;
}

} // namespace voxelforge
