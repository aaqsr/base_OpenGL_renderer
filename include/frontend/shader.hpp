#pragma once

#include "GL/glew.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

// TODO: Could make this class smaller by making functions static, etc.
class Shader
{
    friend class BindObject;

    void linkProgram(uint32_t vertexShader, uint32_t geoShader,
                     uint32_t fragmentShader);
    void discoverUniforms();

    uint32_t programId{};
    static inline bool isBound = false;

  public:
    struct UniformInfo
    {
        GLint location;
        GLenum type;
        std::string name;
    };

  private:
    std::unordered_map<std::string, UniformInfo> uniforms;
    mutable std::unordered_set<std::string> warnedMissingUniforms;
    mutable std::unordered_set<std::string> warnedTypeMismatches;

    void loadFromSource(const std::string& vertexSource,
                        const std::string& geoSource,
                        const std::string& fragmentSource);

    void loadFromFile(const std::filesystem::path& vertexPath,
                      const std::filesystem::path& geoPath,
                      const std::filesystem::path& fragmentPath);

  public:
    class BindObject
    {
        friend class Shader;

        uint32_t programId;
        Shader& shader;

        BindObject(uint32_t programId, Shader& shader);

        // validate uniform and warn if needed
        [[nodiscard]] std::optional<Shader::UniformInfo>
        validateUniform(const std::string& name, GLenum expectedType) const;

      public:
        BindObject(const BindObject&) = delete;
        BindObject(BindObject&&) = delete;
        BindObject& operator=(const BindObject&) = delete;
        BindObject& operator=(BindObject&&) = delete;

        ~BindObject();

        void setUniformInt(const std::string& name, int value);
        void setUniform(const std::string& name, float value);
        void setUniform(const std::string& name, const glm::vec2& value);
        void setUniform(const std::string& name, const glm::vec3& value);
        void setUniform(const std::string& name, const glm::vec4& value);
        void setUniform(const std::string& name, const glm::mat4& value);

        [[nodiscard]] const UniformInfo&
        getUniformInfo(const std::string& name) const;
        [[nodiscard]] bool hasUniform(const std::string& name) const;
    };

    // uniforms are discovered automatically
    Shader(const std::string& vertexSource, const std::string& fragmentSource);
    Shader(const std::filesystem::path& vertexPath,
           const std::filesystem::path& fragmentPath);
    Shader(const std::filesystem::path& vertexPath,
           const std::filesystem::path& geoPath,
           const std::filesystem::path& fragmentPath);

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) = delete;
    Shader& operator=(Shader&&) = delete;

    ~Shader();

    [[nodiscard]] BindObject bind();

    // get all discovered uniforms
    [[nodiscard]] const std::unordered_map<std::string, UniformInfo>&
    getUniforms() const;
};
