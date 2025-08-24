#pragma once

#include <GL/glew.h>

// Simple framebuffer - one color attachment + optional depth.
// Unmaintained for now.
// Work on this when we actually need multiple render targets.
class Framebuffer
{
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
