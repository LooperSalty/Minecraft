#include "TextRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <algorithm>

namespace voxelforge {

// ---------------------------------------------------------------------------
// Procedural 5x7 bitmap font -- ASCII 32..127 (96 glyphs)
// Each glyph is 7 rows (top to bottom); lower 5 bits of each byte are pixels.
// ---------------------------------------------------------------------------
static const uint8_t FONT_DATA[96][7] = {
    // 32  ' '
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},
    // 33  '!'
    {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100},
    // 34  '"'
    {0b01010, 0b01010, 0b01010, 0b00000, 0b00000, 0b00000, 0b00000},
    // 35  '#'
    {0b01010, 0b01010, 0b11111, 0b01010, 0b11111, 0b01010, 0b01010},
    // 36  '$'
    {0b00100, 0b01111, 0b10100, 0b01110, 0b00101, 0b11110, 0b00100},
    // 37  '%'
    {0b11001, 0b11001, 0b00010, 0b00100, 0b01000, 0b10011, 0b10011},
    // 38  '&'
    {0b01100, 0b10010, 0b10100, 0b01000, 0b10101, 0b10010, 0b01101},
    // 39  '\''
    {0b00100, 0b00100, 0b00100, 0b00000, 0b00000, 0b00000, 0b00000},
    // 40  '('
    {0b00010, 0b00100, 0b01000, 0b01000, 0b01000, 0b00100, 0b00010},
    // 41  ')'
    {0b01000, 0b00100, 0b00010, 0b00010, 0b00010, 0b00100, 0b01000},
    // 42  '*'
    {0b00000, 0b00100, 0b10101, 0b01110, 0b10101, 0b00100, 0b00000},
    // 43  '+'
    {0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000},
    // 44  ','
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00100, 0b01000},
    // 45  '-'
    {0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000},
    // 46  '.'
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00100},
    // 47  '/'
    {0b00001, 0b00010, 0b00010, 0b00100, 0b01000, 0b01000, 0b10000},
    // 48  '0'
    {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110},
    // 49  '1'
    {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},
    // 50  '2'
    {0b01110, 0b10001, 0b00001, 0b00110, 0b01000, 0b10000, 0b11111},
    // 51  '3'
    {0b01110, 0b10001, 0b00001, 0b00110, 0b00001, 0b10001, 0b01110},
    // 52  '4'
    {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010},
    // 53  '5'
    {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110},
    // 54  '6'
    {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110},
    // 55  '7'
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000},
    // 56  '8'
    {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110},
    // 57  '9'
    {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100},
    // 58  ':'
    {0b00000, 0b00000, 0b00100, 0b00000, 0b00100, 0b00000, 0b00000},
    // 59  ';'
    {0b00000, 0b00000, 0b00100, 0b00000, 0b00100, 0b00100, 0b01000},
    // 60  '<'
    {0b00010, 0b00100, 0b01000, 0b10000, 0b01000, 0b00100, 0b00010},
    // 61  '='
    {0b00000, 0b00000, 0b11111, 0b00000, 0b11111, 0b00000, 0b00000},
    // 62  '>'
    {0b01000, 0b00100, 0b00010, 0b00001, 0b00010, 0b00100, 0b01000},
    // 63  '?'
    {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b00000, 0b00100},
    // 64  '@'
    {0b01110, 0b10001, 0b10111, 0b10101, 0b10110, 0b10000, 0b01110},
    // 65  'A'
    {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},
    // 66  'B'
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110},
    // 67  'C'
    {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110},
    // 68  'D'
    {0b11100, 0b10010, 0b10001, 0b10001, 0b10001, 0b10010, 0b11100},
    // 69  'E'
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111},
    // 70  'F'
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000},
    // 71  'G'
    {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111},
    // 72  'H'
    {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},
    // 73  'I'
    {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},
    // 74  'J'
    {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100},
    // 75  'K'
    {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001},
    // 76  'L'
    {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111},
    // 77  'M'
    {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001},
    // 78  'N'
    {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001},
    // 79  'O'
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},
    // 80  'P'
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000},
    // 81  'Q'
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101},
    // 82  'R'
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001},
    // 83  'S'
    {0b01110, 0b10001, 0b10000, 0b01110, 0b00001, 0b10001, 0b01110},
    // 84  'T'
    {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100},
    // 85  'U'
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},
    // 86  'V'
    {0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b01010, 0b00100},
    // 87  'W'
    {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b11011, 0b10001},
    // 88  'X'
    {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001},
    // 89  'Y'
    {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100},
    // 90  'Z'
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111},
    // 91  '['
    {0b01110, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01110},
    // 92  '\\'
    {0b10000, 0b01000, 0b01000, 0b00100, 0b00010, 0b00010, 0b00001},
    // 93  ']'
    {0b01110, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01110},
    // 94  '^'
    {0b00100, 0b01010, 0b10001, 0b00000, 0b00000, 0b00000, 0b00000},
    // 95  '_'
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111},
    // 96  '`'
    {0b01000, 0b00100, 0b00010, 0b00000, 0b00000, 0b00000, 0b00000},
    // 97  'a' (same as 'A')
    {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},
    // 98  'b' (same as 'B')
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110},
    // 99  'c' (same as 'C')
    {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110},
    // 100 'd' (same as 'D')
    {0b11100, 0b10010, 0b10001, 0b10001, 0b10001, 0b10010, 0b11100},
    // 101 'e' (same as 'E')
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111},
    // 102 'f' (same as 'F')
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000},
    // 103 'g' (same as 'G')
    {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111},
    // 104 'h' (same as 'H')
    {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},
    // 105 'i' (same as 'I')
    {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},
    // 106 'j' (same as 'J')
    {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100},
    // 107 'k' (same as 'K')
    {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001},
    // 108 'l' (same as 'L')
    {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111},
    // 109 'm' (same as 'M')
    {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001},
    // 110 'n' (same as 'N')
    {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001},
    // 111 'o' (same as 'O')
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},
    // 112 'p' (same as 'P')
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000},
    // 113 'q' (same as 'Q')
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101},
    // 114 'r' (same as 'R')
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001},
    // 115 's' (same as 'S')
    {0b01110, 0b10001, 0b10000, 0b01110, 0b00001, 0b10001, 0b01110},
    // 116 't' (same as 'T')
    {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100},
    // 117 'u' (same as 'U')
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},
    // 118 'v' (same as 'V')
    {0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b01010, 0b00100},
    // 119 'w' (same as 'W')
    {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b11011, 0b10001},
    // 120 'x' (same as 'X')
    {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001},
    // 121 'y' (same as 'Y')
    {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100},
    // 122 'z' (same as 'Z')
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111},
    // 123 '{'
    {0b00010, 0b00100, 0b00100, 0b01000, 0b00100, 0b00100, 0b00010},
    // 124 '|'
    {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100},
    // 125 '}'
    {0b01000, 0b00100, 0b00100, 0b00010, 0b00100, 0b00100, 0b01000},
    // 126 '~'
    {0b00000, 0b00000, 0b01000, 0b10101, 0b00010, 0b00000, 0b00000},
    // 127 DEL (blank)
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},
};

// ---------------------------------------------------------------------------
// init
// ---------------------------------------------------------------------------
void TextRenderer::init(Shader* lineShader) {
    m_shader = lineShader;
}

// ---------------------------------------------------------------------------
// drawRect
// ---------------------------------------------------------------------------
void TextRenderer::drawRect(float x, float y, float w, float h,
                             const glm::vec4& color,
                             int screenW, int screenH) {
    if (!m_shader) return;

    // Save/set GL state
    GLboolean prevBlend = GL_FALSE;
    GLboolean prevDepth = GL_FALSE;
    GLboolean prevCull  = GL_FALSE;
    glGetBooleanv(GL_BLEND, &prevBlend);
    glGetBooleanv(GL_DEPTH_TEST, &prevDepth);
    glGetBooleanv(GL_CULL_FACE, &prevCull);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    const glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(screenW),
                                         static_cast<float>(screenH), 0.0f);

    m_shader->use();
    m_shader->setMat4("uMVP", ortho);
    m_shader->setVec4("uColor", color);

    const float verts[] = {
        x,     y,     0.0f,
        x + w, y,     0.0f,
        x + w, y + h, 0.0f,
        x,     y,     0.0f,
        x + w, y + h, 0.0f,
        x,     y + h, 0.0f,
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    // Restore GL state
    if (!prevBlend) glDisable(GL_BLEND);
    if (prevDepth)  glEnable(GL_DEPTH_TEST);
    if (prevCull)   glEnable(GL_CULL_FACE);
}

// ---------------------------------------------------------------------------
// drawText
// ---------------------------------------------------------------------------
void TextRenderer::drawText(const std::string& text, float x, float y,
                             float scale, const glm::vec4& color,
                             int screenW, int screenH) {
    if (!m_shader || text.empty()) return;

    // Save/set GL state
    GLboolean prevBlend = GL_FALSE;
    GLboolean prevDepth = GL_FALSE;
    GLboolean prevCull  = GL_FALSE;
    glGetBooleanv(GL_BLEND, &prevBlend);
    glGetBooleanv(GL_DEPTH_TEST, &prevDepth);
    glGetBooleanv(GL_CULL_FACE, &prevCull);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    const glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(screenW),
                                         static_cast<float>(screenH), 0.0f);

    m_shader->use();
    m_shader->setMat4("uMVP", ortho);
    m_shader->setVec4("uColor", color);

    // Batch all character pixel quads into one VBO
    std::vector<float> verts;
    const float ps = scale; // pixel size
    float px = x;

    for (const char ch : text) {
        if (ch < 32 || ch > 127) {
            px += static_cast<float>(GLYPH_W + CHAR_SPACING) * scale;
            continue;
        }
        const int ci = ch - 32;

        for (int row = 0; row < GLYPH_H; ++row) {
            const uint8_t bits = FONT_DATA[ci][row];
            for (int col = 0; col < GLYPH_W; ++col) {
                if ((bits >> (4 - col)) & 1) {
                    const float qx = px + col * ps;
                    const float qy = y + row * ps;
                    // Two triangles for one pixel quad
                    verts.insert(verts.end(), {
                        qx,      qy,      0.0f,
                        qx + ps, qy,      0.0f,
                        qx + ps, qy + ps, 0.0f,
                        qx,      qy,      0.0f,
                        qx + ps, qy + ps, 0.0f,
                        qx,      qy + ps, 0.0f,
                    });
                }
            }
        }
        px += static_cast<float>(GLYPH_W + CHAR_SPACING) * scale;
    }

    if (!verts.empty()) {
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                     verts.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(verts.size() / 3));
        glBindVertexArray(0);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    // Restore GL state
    if (!prevBlend) glDisable(GL_BLEND);
    if (prevDepth)  glEnable(GL_DEPTH_TEST);
    if (prevCull)   glEnable(GL_CULL_FACE);
}

// ---------------------------------------------------------------------------
// drawTextCentered
// ---------------------------------------------------------------------------
void TextRenderer::drawTextCentered(const std::string& text, float centerX,
                                     float y, float scale,
                                     const glm::vec4& color,
                                     int screenW, int screenH) {
    const float w = textWidth(text, scale);
    drawText(text, centerX - w * 0.5f, y, scale, color, screenW, screenH);
}

// ---------------------------------------------------------------------------
// textWidth
// ---------------------------------------------------------------------------
float TextRenderer::textWidth(const std::string& text, float scale) const {
    if (text.empty()) {
        return 0.0f;
    }
    return static_cast<float>(text.length()) * (GLYPH_W + CHAR_SPACING) * scale
           - CHAR_SPACING * scale;
}

// ---------------------------------------------------------------------------
// textHeight
// ---------------------------------------------------------------------------
float TextRenderer::textHeight(float scale) const {
    return static_cast<float>(GLYPH_H) * scale;
}

// ---------------------------------------------------------------------------
// drawButton
// ---------------------------------------------------------------------------
bool TextRenderer::drawButton(const std::string& label, float cx, float cy,
                               float padX, float padY, float scale,
                               const glm::vec4& bgColor,
                               const glm::vec4& textColor,
                               int screenW, int screenH,
                               const glm::vec2& mousePos, bool mouseClick) {
    const float tw = textWidth(label, scale);
    const float th = textHeight(scale);

    const float rx = cx - tw * 0.5f - padX;
    const float ry = cy - th * 0.5f - padY;
    const float rw = tw + 2.0f * padX;
    const float rh = th + 2.0f * padY;

    const bool hovered = mousePos.x >= rx && mousePos.x <= rx + rw &&
                         mousePos.y >= ry && mousePos.y <= ry + rh;

    // Slightly lighter background when hovered
    const glm::vec4 bg = hovered
        ? glm::vec4(std::min(bgColor.r + 0.15f, 1.0f),
                    std::min(bgColor.g + 0.15f, 1.0f),
                    std::min(bgColor.b + 0.15f, 1.0f),
                    bgColor.a)
        : bgColor;

    drawRect(rx, ry, rw, rh, bg, screenW, screenH);
    drawTextCentered(label, cx, cy - th * 0.5f, scale, textColor,
                     screenW, screenH);

    return hovered && mouseClick;
}

// ---------------------------------------------------------------------------
// cleanup
// ---------------------------------------------------------------------------
void TextRenderer::cleanup() {
    // No owned resources to clean up -- shader is external
    m_shader = nullptr;
}

} // namespace voxelforge
