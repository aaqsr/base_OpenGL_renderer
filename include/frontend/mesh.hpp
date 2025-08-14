#pragma once

#include "frontend/shader.hpp"
#include "indexBuffer.hpp"
#include "vertexBuffer.hpp"
#include "vertexLayout.hpp"

#include <GL/glew.h>

#include <optional>

class Mesh
{
    GLuint vao = 0;
    VertexBuffer vertexBuffer;
    std::optional<IndexBuffer> indexBuffer;
    VertexLayout layout;
    size_t drawCount = 0;

  public:
    Mesh()
    {
        glGenVertexArrays(1, &vao);
    }

    template <typename VertexType>
    Mesh(const std::vector<VertexType>& vertices,
         const VertexLayout& vertexLayout)
    {
        glGenVertexArrays(1, &vao);
        setVertexData(vertices, vertexLayout);
    }

    template <typename VertexType>
    Mesh(const std::vector<VertexType>& vertices,
         const VertexLayout& vertexLayout, const std::vector<uint32_t>& indices)
    {
        glGenVertexArrays(1, &vao);
        setVertexData(vertices, vertexLayout);
        setIndexData(indices);
    }

    ~Mesh()
    {
        if (vao != 0) {
            glDeleteVertexArrays(1, &vao);
        }
    }

    // Non-copyable, moveable
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept
      : vao(other.vao), vertexBuffer(std::move(other.vertexBuffer)),
        indexBuffer(std::move(other.indexBuffer)),
        layout(std::move(other.layout)), drawCount(other.drawCount)
    {
        other.vao = 0;
        other.drawCount = 0;
    }

    Mesh& operator=(Mesh&& other) noexcept
    {
        if (this != &other) {
            if (vao != 0) {
                glDeleteVertexArrays(1, &vao);
            }
            vao = other.vao;
            vertexBuffer = std::move(other.vertexBuffer);
            indexBuffer = std::move(other.indexBuffer);
            layout = std::move(other.layout);
            drawCount = other.drawCount;
            other.vao = 0;
            other.drawCount = 0;
        }
        return *this;
    }

    template <typename VertexType>
    void setVertexData(const std::vector<VertexType>& vertices,
                       const VertexLayout& vertexLayout)
    {
        layout = vertexLayout;

        glBindVertexArray(vao);
        vertexBuffer.uploadData(vertices);
        layout.apply();

        drawCount = vertices.size();
        glBindVertexArray(0);
    }

    void setIndexData(const std::vector<uint32_t>& indices)
    {
        glBindVertexArray(vao);
        if (!indexBuffer) {
            indexBuffer.emplace();
        }
        indexBuffer->uploadData(indices);
        drawCount = indices.size();
        glBindVertexArray(0);
    }

    void draw(const Shader::BindObject& /*shader*/,
              GLenum primitive = GL_TRIANGLES) const
    {
        glBindVertexArray(vao);
        if (indexBuffer) {
            glDrawElements(primitive, static_cast<GLsizei>(drawCount),
                           GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(primitive, 0, static_cast<GLsizei>(drawCount));
        }
        glBindVertexArray(0);
    }

    [[nodiscard]] GLuint getVAO() const
    {
        return vao;
    }
};
