#pragma once

#include "frontend/mesh.hpp"
#include "frontend/shader.hpp"
#include "frontend/texture.hpp"
#include "frontend/worldPose.hpp"

#include <tiny_obj_loader.h>

#include <filesystem>
#include <unordered_map>
#include <vector>

struct LoadedObject
{
    // a single drawable part of the larger object.
    struct Shape
    {
        Mesh mesh;
        // index into the materials vector. -1 if no material
        int materialId = -1;
    };

    std::vector<Shape> shapes;
    std::vector<tinyobj::material_t> materials;
    std::unordered_map<std::string, Texture> textures;
    WorldPose pose;

    [[nodiscard]] LoadedObject() = default;
    [[nodiscard]] LoadedObject(const std::filesystem::path& path);

    void setInitUniforms(Shader::BindObject& shader) const;
    void draw(Shader::BindObject& shader) const;
};
