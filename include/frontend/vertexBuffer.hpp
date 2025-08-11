#pragma once
#include <GL/glew.h>
#include <cstdint>
#include <vector>

class VertexBuffer
{
  private:
    GLuint vbo = 0;
    size_t vertexCount = 0;

  public:
    VertexBuffer()
    {
        glGenBuffers(1, &vbo);
    }

    ~VertexBuffer()
    {
        if (vbo != 0) {
            glDeleteBuffers(1, &vbo);
        }
    }

    // Non-copyable, moveable
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& other) noexcept
      : vbo(other.vbo), vertexCount(other.vertexCount)
    {
        other.vbo = 0;
        other.vertexCount = 0;
    }

    VertexBuffer& operator=(VertexBuffer&& other) noexcept
    {
        if (this != &other) {
            if (vbo != 0) {
                glDeleteBuffers(1, &vbo);
            }
            vbo = other.vbo;
            vertexCount = other.vertexCount;
            other.vbo = 0;
            other.vertexCount = 0;
        }
        return *this;
    }

    template <typename T>
    void uploadData(const std::vector<T>& data, GLenum usage = GL_STATIC_DRAW)
    {
        vertexCount = data.size();
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(),
                     usage);
    }

    template <typename T>
    void uploadData(const T* data, size_t count, GLenum usage = GL_STATIC_DRAW)
    {
        vertexCount = count;
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(T), data, usage);
    }

    void bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    void unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    [[nodiscard]] size_t getVertexCount() const
    {
        return vertexCount;
    }

    [[nodiscard]] GLuint getId() const
    {
        return vbo;
    }
};


//===== Framebuffer.hpp =====
#pragma once
#include <GL/glew.h>

// Simple framebuffer - one color attachment + optional depth
// Expand when you actually need multiple render targets
class Framebuffer
{
  private:
    GLuint fbo = 0;
    GLuint colorTexture = 0;
    GLuint depthTexture = 0;
    uint32_t width = 0;
    uint32_t height = 0;

  public:
    Framebuffer(uint32_t w, uint32_t h) : width(w), height(h)
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // Color attachment
        glGenTextures(1, &colorTexture);
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, colorTexture, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~Framebuffer()
    {
        if (fbo != 0)
            glDeleteFramebuffers(1, &fbo);
        if (colorTexture != 0)
            glDeleteTextures(1, &colorTexture);
        if (depthTexture != 0)
            glDeleteTextures(1, &depthTexture);
    }

    // Non-copyable, moveable
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    Framebuffer(Framebuffer&& other) noexcept
      : fbo(other.fbo), colorTexture(other.colorTexture),
        depthTexture(other.depthTexture), width(other.width),
        height(other.height)
    {
        other.fbo = 0;
        other.colorTexture = 0;
        other.depthTexture = 0;
        other.width = 0;
        other.height = 0;
    }

    Framebuffer& operator=(Framebuffer&& other) noexcept
    {
        if (this != &other) {
            if (fbo != 0)
                glDeleteFramebuffers(1, &fbo);
            if (colorTexture != 0)
                glDeleteTextures(1, &colorTexture);
            if (depthTexture != 0)
                glDeleteTextures(1, &depthTexture);

            fbo = other.fbo;
            colorTexture = other.colorTexture;
            depthTexture = other.depthTexture;
            width = other.width;
            height = other.height;

            other.fbo = 0;
            other.colorTexture = 0;
            other.depthTexture = 0;
            other.width = 0;
            other.height = 0;
        }
        return *this;
    }

    void addDepthAttachment()
    {
        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, depthTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    bool isComplete() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    void bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, width, height);
    }

    void unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLuint getColorTexture() const
    {
        return colorTexture;
    }
    GLuint getDepthTexture() const
    {
        return depthTexture;
    }
    uint32_t getWidth() const
    {
        return width;
    }
    uint32_t getHeight() const
    {
        return height;
    }
};
