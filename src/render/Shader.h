#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>

namespace voxelforge {

class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath);
    void use() const;

    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

    GLuint getID() const { return m_program; }

private:
    GLuint compileShader(GLenum type, const std::string& source);
    GLuint m_program = 0;
};

} // namespace voxelforge
