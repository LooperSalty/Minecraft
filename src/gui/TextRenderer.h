#pragma once
#include "../render/Shader.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>

namespace voxelforge {

class TextRenderer {
public:
    void init(Shader* lineShader);
    void cleanup();

    void drawText(const std::string& text, float x, float y, float scale,
                  const glm::vec4& color, int screenW, int screenH);
    void drawRect(float x, float y, float w, float h,
                  const glm::vec4& color, int screenW, int screenH);

    // Centered text helpers
    void drawTextCentered(const std::string& text, float centerX, float y, float scale,
                          const glm::vec4& color, int screenW, int screenH);
    float textWidth(const std::string& text, float scale) const;
    float textHeight(float scale) const;

    // Button: returns true if clicked this frame
    bool drawButton(const std::string& label, float cx, float cy, float padX, float padY,
                    float scale, const glm::vec4& bgColor, const glm::vec4& textColor,
                    int screenW, int screenH, const glm::vec2& mousePos, bool mouseClick);

private:
    Shader* m_shader = nullptr;

    static constexpr int GLYPH_W = 5;
    static constexpr int GLYPH_H = 7;
    static constexpr int CHAR_SPACING = 1;
};

} // namespace voxelforge
