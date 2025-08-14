#pragma once

#include "shader.hpp"

#include <filesystem>
#include <string>

// Does not actually store the image file inside!
// It gets copied over to the GPU memory.
class Texture
{
    uint32_t textureId = 0;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::string filePath;

    void loadFromFile(const std::filesystem::path& path);
    void loadFromData(const unsigned char* data, int dataWidth, int dataHeight,
                      GLenum format, GLenum internalFormat);

  public:
    Texture(const std::filesystem::path& path);

    // `data` is borrowed here, caller must clean-up after the function.
    // We assume that both the internal format and format of `GL_RGB`.
    Texture(int width, int height, const unsigned char* data);

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    ~Texture();

    void setInitUniform(Shader::BindObject& shader,
                        const std::string& textureUniformName,
                        GLuint textureUnit) const;

    void bind(Shader::BindObject& shader, GLuint textureUnit) const;

    [[nodiscard]] uint32_t getId() const;
    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;
    [[nodiscard]] int getChannels() const;
    [[nodiscard]] const std::string& getFilePath() const;
    [[nodiscard]] bool isValid() const;
};
