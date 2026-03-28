#include "Shader.h"
#include <fstream>
#include <sstream>
#include <cstdio>

namespace voxelforge {

Shader::~Shader() {
    if (m_program) glDeleteProgram(m_program);
}

bool Shader::loadFromFiles(const std::string& vertPath, const std::string& fragPath) {
    auto readFile = [](const std::string& path) -> std::string {
        std::ifstream f(path);
        if (!f.is_open()) return {};
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    };

    std::string vertSrc = readFile(vertPath);
    std::string fragSrc = readFile(fragPath);
    if (vertSrc.empty()) { std::fprintf(stderr, "Cannot open %s\n", vertPath.c_str()); return false; }
    if (fragSrc.empty()) { std::fprintf(stderr, "Cannot open %s\n", fragPath.c_str()); return false; }

    GLuint vs = compileShader(GL_VERTEX_SHADER,   vertSrc);
    if (!vs) return false;
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!fs) { glDeleteShader(vs); return false; }

    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);

    GLint ok;
    glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(m_program, 512, nullptr, log);
        std::fprintf(stderr, "Shader link error: %s\n", log);
        glDeleteProgram(m_program);
        m_program = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return m_program != 0;
}

void Shader::use() const { glUseProgram(m_program); }

void Shader::setInt(const std::string& n, int v) const {
    glUniform1i(glGetUniformLocation(m_program, n.c_str()), v);
}
void Shader::setFloat(const std::string& n, float v) const {
    glUniform1f(glGetUniformLocation(m_program, n.c_str()), v);
}
void Shader::setVec3(const std::string& n, const glm::vec3& v) const {
    glUniform3fv(glGetUniformLocation(m_program, n.c_str()), 1, &v[0]);
}
void Shader::setVec4(const std::string& n, const glm::vec4& v) const {
    glUniform4fv(glGetUniformLocation(m_program, n.c_str()), 1, &v[0]);
}
void Shader::setMat4(const std::string& n, const glm::mat4& v) const {
    glUniformMatrix4fv(glGetUniformLocation(m_program, n.c_str()), 1, GL_FALSE, &v[0][0]);
}

GLuint Shader::compileShader(GLenum type, const std::string& source) {
    GLuint s = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        std::fprintf(stderr, "Shader compile error (%s): %s\n",
                     type == GL_VERTEX_SHADER ? "vert" : "frag", log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

} // namespace voxelforge
