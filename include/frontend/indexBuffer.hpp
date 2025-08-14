#pragma once

#include <GL/glew.h>

#include <cstdint>
#include <vector>

class IndexBuffer
{
    GLuint ebo = 0;
    size_t indexCount = 0;

  public:
    IndexBuffer()
    {
        glGenBuffers(1, &ebo);
    }

    ~IndexBuffer()
    {
        if (ebo != 0) {
            glDeleteBuffers(1, &ebo);
        }
    }

    // Non-copyable, moveable
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    IndexBuffer(IndexBuffer&& other) noexcept
      : ebo(other.ebo), indexCount(other.indexCount)
    {
        other.ebo = 0;
        other.indexCount = 0;
    }

    IndexBuffer& operator=(IndexBuffer&& other) noexcept
    {
        if (this != &other) {
            if (ebo != 0)
                glDeleteBuffers(1, &ebo);
            ebo = other.ebo;
            indexCount = other.indexCount;
            other.ebo = 0;
            other.indexCount = 0;
        }
        return *this;
    }

    void uploadData(const std::vector<uint32_t>& indices,
                    GLenum usage = GL_STATIC_DRAW)
    {
        indexCount = indices.size();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
                     indices.data(), usage);
    }

    void uploadData(const uint32_t* indices, size_t count,
                    GLenum usage = GL_STATIC_DRAW)
    {
        indexCount = count;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices,
                     usage);
    }

    void bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }

    void unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    size_t getIndexCount() const
    {
        return indexCount;
    }
    GLuint getId() const
    {
        return ebo;
    }
};
