#include "frontend/shader.hpp"
#include "util/error.hpp"
#include "util/logger.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace
{
[[nodiscard]] uint32_t compileShader(const std::string& source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    const char* sourceCStr = source.c_str();

    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == 0) {
        std::array<GLchar, 1024> infoLog{};
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog.data());

        const char* shaderType = (type == GL_VERTEX_SHADER)     ? "VERTEX"
                                 : (type == GL_GEOMETRY_SHADER) ? "GEOMETRY"
                                                                : "FRAGMENT";

        glDeleteShader(shader);

        throw IrrecoverableError{std::string{"ERROR: "} + shaderType +
                                 " shader compilation failed:\n" +
                                 infoLog.data()};
    }

    return shader;
}

std::string readFile(const std::filesystem::path& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw IrrecoverableError{"ERROR: Could not open file: " +
                                 filePath.string()};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

const char* getGLTypeName(GLenum type)
{
    switch (type) {
        case GL_INT: return "int";
        case GL_FLOAT: return "float";
        case GL_FLOAT_VEC2: return "vec2";
        case GL_FLOAT_VEC3: return "vec3";
        case GL_FLOAT_VEC4: return "vec4";
        case GL_FLOAT_MAT4: return "mat4";
        case GL_SAMPLER_2D: return "sampler2D";
        case GL_SAMPLER_CUBE: return "samplerCube";
        default: return "unknown";
    }
}
} // namespace

Shader::Shader(const std::string& vertexSource,
               const std::string& fragmentSource)
{
    loadFromSource(vertexSource, "", fragmentSource);
}

Shader::Shader(const std::filesystem::path& vertexPath,
               const std::filesystem::path& fragmentPath)
{
    loadFromFile(vertexPath, "", fragmentPath);
}

Shader::Shader(const std::filesystem::path& vertexPath,
               const std::filesystem::path& geoPath,
               const std::filesystem::path& fragmentPath)
{
    loadFromFile(vertexPath, geoPath, fragmentPath);
}

Shader::~Shader()
{
    if (programId != 0) {
        glDeleteProgram(programId);
    }
}

void Shader::loadFromSource(const std::string& vertexSource,
                            const std::string& geoSource,
                            const std::string& fragmentSource)
{
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        throw IrrecoverableError{"Vertex shader failed to compile"};
    }

    GLuint geoShader = 0;
    if (!geoSource.empty()) {
        geoShader = compileShader(geoSource, GL_GEOMETRY_SHADER);
        if (geoShader == 0) {
            glDeleteShader(vertexShader);
            throw IrrecoverableError{"Geometry shader failed to compile"};
        }
    }

    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        if (geoShader != 0) {
            glDeleteShader(geoShader);
        }
        throw IrrecoverableError{"Fragment shader failed to compile"};
    }

    try {
        linkProgram(vertexShader, geoShader, fragmentShader);
        discoverUniforms();
    } catch (const IrrecoverableError& e) {
        glDeleteShader(vertexShader);
        if (geoShader != 0) {
            glDeleteShader(geoShader);
        }
        glDeleteShader(fragmentShader);
        if (programId != 0) {
            glDeleteProgram(programId);
            programId = 0;
        }
        throw;
    }

    glDeleteShader(vertexShader);
    if (geoShader != 0) {
        glDeleteShader(geoShader);
    }
    glDeleteShader(fragmentShader);
}

void Shader::loadFromFile(const std::filesystem::path& vertexPath,
                          const std::filesystem::path& geoPath,
                          const std::filesystem::path& fragmentPath)
{
    std::string vertexSource = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        throw IrrecoverableError{"Failed to read shader file"};
    }

    std::string geoSource;
    if (!geoPath.empty()) {
        geoSource = readFile(geoPath);
        if (geoSource.empty()) {
            throw IrrecoverableError{"Failed to read geometry shader file"};
        }
    }

    loadFromSource(vertexSource, geoSource, fragmentSource);
}

Shader::BindObject Shader::bind()
{
    if (isBound) {
        throw IrrecoverableError{
          "Attempt to bind a shader whilst one is already bound. "
          "Are you sure you want to do this?"};
    }

    isBound = true;
    return BindObject{programId, *this};
}

void Shader::linkProgram(GLuint vertexShader, GLuint geoShader,
                         GLuint fragmentShader)
{
    programId = glCreateProgram();

    glAttachShader(programId, vertexShader);
    if (geoShader != 0) {
        glAttachShader(programId, geoShader);
    }
    glAttachShader(programId, fragmentShader);

    glLinkProgram(programId);

    GLint success = 0;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);

    if (success == 0) {
        std::array<GLchar, 1024> infoLog{};
        glGetProgramInfoLog(programId, sizeof(infoLog), nullptr,
                            infoLog.data());

        glDeleteProgram(programId);
        programId = 0;

        throw IrrecoverableError{
          std::string{"ERROR: Shader program linking failed:\n"} +
          infoLog.data()};
    }
}

void Shader::discoverUniforms()
{
    uniforms.clear();

    GLint uniformCount = 0;
    glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &uniformCount);

    for (GLint i = 0; i < uniformCount; ++i) {
        std::array<char, 256> name{};
        GLsizei length = 0;
        GLint size = 0;
        GLenum type = 0;

        glGetActiveUniform(programId, static_cast<GLuint>(i), name.size(),
                           &length, &size, &type, name.data());

        if (length > 0) {
            std::string uniformName(name.data(), length);
            GLint location =
              glGetUniformLocation(programId, uniformName.c_str());

            if (location != -1) {
                UniformInfo info{
                  .location = location, .type = type, .name = uniformName};
                uniforms[uniformName] = info;
            }
        }
    }
}

const std::unordered_map<std::string, Shader::UniformInfo>&
Shader::getUniforms() const
{
    return uniforms;
}

const Shader::UniformInfo&
Shader::BindObject::getUniformInfo(const std::string& name) const
{
    auto it = shader.uniforms.find(name);
    if (it == shader.uniforms.end()) {
        throw IrrecoverableError{"Uniform '" + name +
                                 "' does not exist in shader program"};
    }
    return it->second;
}

bool Shader::BindObject::hasUniform(const std::string& name) const
{
    return shader.uniforms.contains(name);
}

std::optional<Shader::UniformInfo>
Shader::BindObject::validateUniform(const std::string& name,
                                    GLenum expectedType) const
{
    auto it = shader.uniforms.find(name);
    if (it == shader.uniforms.end()) {
        if (!shader.warnedMissingUniforms.contains(name)) {
            Logger::log("WARNING: Uniform '" + name +
                        "' does not exist in shader program");
            shader.warnedMissingUniforms.insert(name);
        }
        return {};
    }

    if (it->second.type != expectedType) {
        if (!shader.warnedTypeMismatches.contains(name)) {
            Logger::log("WARNING: Uniform '" + name +
                        "' type mismatch. Expected " +
                        getGLTypeName(expectedType) + ", got " +
                        getGLTypeName(it->second.type));
            shader.warnedTypeMismatches.insert(name);
        }
    }

    return it->second;
}

void Shader::BindObject::setUniformSampler2D(const std::string& name, int value)
{
    if (const auto& info = validateUniform(name, GL_SAMPLER_2D)) {
        glUniform1i(info->location, value);
    }
}

void Shader::BindObject::setUniformInt(const std::string& name, int value)
{
    if (const auto& info = validateUniform(name, GL_INT)) {
        glUniform1i(info->location, value);
    }
}

void Shader::BindObject::setUniform(const std::string& name, float value)
{
    if (const auto& info = validateUniform(name, GL_FLOAT)) {
        glUniform1f(info->location, value);
    }
}

void Shader::BindObject::setUniform(const std::string& name,
                                    const glm::vec2& value)
{
    if (const auto& info = validateUniform(name, GL_FLOAT_VEC2)) {
        glUniform2f(info->location, value.x, value.y);
    }
}

void Shader::BindObject::setUniform(const std::string& name,
                                    const glm::vec3& value)
{
    if (const auto& info = validateUniform(name, GL_FLOAT_VEC3)) {
        glUniform3f(info->location, value.x, value.y, value.z);
    }
}

void Shader::BindObject::setUniform(const std::string& name,
                                    const glm::vec4& value)
{
    if (const auto& info = validateUniform(name, GL_FLOAT_VEC4)) {
        glUniform4f(info->location, value.x, value.y, value.z, value.w);
    }
}

void Shader::BindObject::setUniform(const std::string& name,
                                    const glm::mat4& value)
{
    if (const auto& info = validateUniform(name, GL_FLOAT_MAT4)) {
        glUniformMatrix4fv(info->location, 1, GL_FALSE, glm::value_ptr(value));
    }
}

Shader::BindObject::BindObject(uint32_t programId, Shader& shader)
  : programId{programId}, shader{shader}
{
    glUseProgram(programId);
}

Shader::BindObject::~BindObject()
{
    Shader::isBound = false;
    glUseProgram(0);
}
