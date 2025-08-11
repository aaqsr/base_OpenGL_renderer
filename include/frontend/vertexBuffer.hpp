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

