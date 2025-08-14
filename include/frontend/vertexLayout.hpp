#pragma once

#include "util/error.hpp"

#include <GL/glew.h>

#include <cstdint>
#include <vector>

struct VertexAttribute
{
    uint32_t location{};
    uint32_t componentCount{};
    GLenum type{};
    bool normalized{};
    size_t offset{};
    size_t size{};

    // VertexAttribute(uint32_t loc, uint32_t count, GLenum t, bool norm,
    //                 size_t off, size_t sz)
    //   : location(loc), componentCount(count), type(t), normalized(norm),
    //     offset(off), size(sz)
    // {
    // }
};

class VertexLayout
{
    std::vector<VertexAttribute> attributes;
    size_t stride = 0;

  public:
    VertexLayout& addAttribute(uint32_t location, uint32_t componentCount,
                               GLenum type, bool normalized = false)
    {
        size_t typeSize = 0;
        switch (type) {
            case GL_FLOAT: typeSize = sizeof(float); break;
            case GL_UNSIGNED_INT: typeSize = sizeof(uint32_t); break;
            case GL_INT: typeSize = sizeof(int32_t); break;
            case GL_UNSIGNED_BYTE: typeSize = sizeof(uint8_t); break;
            default:
                throw IrrecoverableError{"Type not defined in addAttribute()"};
        }

        attributes.emplace_back(location, componentCount, type, normalized,
                                stride, typeSize * componentCount);
        stride += typeSize * componentCount;

        return *this;
    }

    void apply() const
    {
        for (const auto& attr : attributes) {
            glVertexAttribPointer(
              attr.location, static_cast<int>(attr.componentCount), attr.type,
              attr.normalized ? GL_TRUE : GL_FALSE,
              static_cast<GLsizei>(stride),
              reinterpret_cast<void*>(attr.offset));
            glEnableVertexAttribArray(attr.location);
        }
    }

    [[nodiscard]] size_t getStride() const
    {
        return stride;
    }
};
