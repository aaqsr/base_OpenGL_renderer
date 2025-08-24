#include "frontend/loadedObj.hpp"

#include "frontend/shader.hpp"
#include "frontend/texture.hpp"
#include "frontend/vertexLayout.hpp"
#include "util/error.hpp"
#include "util/logger.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <glm/glm.hpp>

#include <unordered_map>

namespace
{
struct LoadedObjVertex
{
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 texCoord{};

    // Needed for use as a key in std::unordered_map
    bool operator==(const LoadedObjVertex& other) const
    {
        return position == other.position && normal == other.normal &&
               texCoord == other.texCoord;
    }

    LoadedObjVertex(const tinyobj::index_t& index,
                    const tinyobj::attrib_t& attrib)
    {
        // Position
        if (index.vertex_index >= 0) {
            position = {attrib.vertices[(3 * index.vertex_index) + 0],
                        attrib.vertices[(3 * index.vertex_index) + 1],
                        attrib.vertices[(3 * index.vertex_index) + 2]};
        }

        // Normal
        if (index.normal_index >= 0) {
            normal = {attrib.normals[(3 * index.normal_index) + 0],
                      attrib.normals[(3 * index.normal_index) + 1],
                      attrib.normals[(3 * index.normal_index) + 2]};
        }

        // TexCoord (flip V)
        if (index.texcoord_index >= 0) {
            texCoord = {attrib.texcoords[(2 * index.texcoord_index) + 0],
                        1.0F -
                          attrib.texcoords[(2 * index.texcoord_index) + 1]};
        }
    }
} __attribute__((packed));

const VertexLayout loadedObjVertexLayout =
  VertexLayout{}
    .addAttribute(0, 3, GL_FLOAT)  // position
    .addAttribute(1, 3, GL_FLOAT)  // normal
    .addAttribute(2, 2, GL_FLOAT); // texCoord

struct VertexHasher
{
    static size_t hashVec(const glm::vec2& v)
    {
        return std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1);
    }
    static size_t hashVec(const glm::vec3& v)
    {
        return ((std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1)) >>
                1) ^
               (std::hash<float>()(v.z) << 1);
    }

    std::size_t operator()(const LoadedObjVertex& v) const
    {
        // A simple hash combination
        size_t h1 = hashVec(v.position);
        size_t h2 = hashVec(v.normal);
        size_t h3 = hashVec(v.texCoord);
        return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
    }
};
} // anonymous namespace

namespace
{

// TODO: Better abstraction needed.
// To add support for a new texture map,
// 1. Add a new load function
//      (and call it in loadTextures)
// 2. Add a new bind function
//      (and call it in draw)
// 3. Register what uniform name corresponds to the texture number
//      (in `setInitUniforms()`)
// 4. Make sure to update the shader to support it

void loadDiffuseMapFrom(const tinyobj::material_t& mat,
                        const std::filesystem::path& parentDir,
                        std::unordered_map<std::string, Texture>& textures)
{
    if (!mat.diffuse_texname.empty()) {
        if (!textures.contains(mat.diffuse_texname)) {
            try {
                std::filesystem::path texturePath =
                  parentDir / mat.diffuse_texname;
                textures.emplace(mat.diffuse_texname, Texture(texturePath));
            } catch (const std::exception& e) {
                LOG("Failed to load texture: " << mat.diffuse_texname << " ("
                                               << e.what() << ")");
            }
        }
    }
}

void bindDiffuseMapFrom(
  const tinyobj::material_t& mat,
  const std::unordered_map<std::string, Texture>& textures,
  Shader::BindObject& shader)
{
    if (!mat.diffuse_texname.empty()) {
        const auto& tex = textures.at(mat.diffuse_texname);
        tex.bind(shader, 0);
    }
}

void loadTextures(const std::filesystem::path& parentDir,
                  const std::vector<tinyobj::material_t>& materials,
                  std::unordered_map<std::string, Texture>& textures)
{
    //
    // Load textures from materials
    //
    for (const auto& mat : materials) {
        // DIFFUSE MAP
        loadDiffuseMapFrom(mat, parentDir, textures);
        // load other textures as needed...
    }
}
} // anonymous namespace

LoadedObject::LoadedObject(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        throw IrrecoverableError("Object file not found: " + path.string());
    }

    std::filesystem::path parentDir =
      path.has_parent_path() ? path.parent_path() : "";

    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = parentDir.string();
    reader_config.triangulate = true; // ensure triangles

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(path.string(), reader_config)) {
        std::string msg = reader.Error();
        if (!reader.Warning().empty()) {
            msg += "\nWarning: " + reader.Warning();
        }
        throw IrrecoverableError{"Failed to load .obj file: " + msg};
    }

    if (!reader.Warning().empty()) {
        LOG("Warning while loading .obj: " << reader.Warning());
    }

    const tinyobj::attrib_t& attrib = reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& objShapes = reader.GetShapes();
    materials = reader.GetMaterials();

    loadTextures(parentDir, materials, textures);

    // Build meshes for shapes
    for (const auto& shape : objShapes) {
        std::vector<LoadedObjVertex> vertices;
        std::vector<uint32_t> indices;
        std::unordered_map<LoadedObjVertex, uint32_t, VertexHasher>
          uniqueVertices;

        for (const auto& index : shape.mesh.indices) {
            LoadedObjVertex vertex{index, attrib};

            // Deduplicate
            if (!uniqueVertices.contains(vertex)) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }

        // Upload mesh
        Shape loadedShape;
        loadedShape.mesh.setVertexData(vertices, loadedObjVertexLayout);
        loadedShape.mesh.setIndexData(indices);

        if (!shape.mesh.material_ids.empty()) {
            loadedShape.materialId = shape.mesh.material_ids[0];
        }

        shapes.push_back(std::move(loadedShape));
    }
}

void LoadedObject::setInitUniforms(Shader::BindObject& shader) const
{
    //
    // bind texture maps we might load
    //
    Texture::setInitUniform(shader, "theTexture", 0); // DIFFUSE MAP
}

void LoadedObject::draw(Shader::BindObject& shader) const
{
    shader.setUniform("model", pose.computeTransform());

    for (const auto& shape : shapes) {
        if (shape.materialId >= 0) {
            //
            // bind relevant material properties
            //
            const auto& mat = materials[shape.materialId];
            bindDiffuseMapFrom(mat, textures, shader);
        }
        shape.mesh.draw(shader);
    }
}
