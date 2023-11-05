#pragma once
#include <gltf/accessor.h>
#include <gltf/asset.h>
#include <gltf/buffer.h>
#include <gltf/buffer_view.h>
#include <gltf/image.h>
#include <gltf/material.h>
#include <gltf/mesh.h>
#include <gltf/node.h>
#include <gltf/sampler.h>
#include <gltf/scene.h>
#include <gltf/texture.h>

namespace gltf
{

struct JSON
{
    std::vector<gltf::Accessor> accessors;
    std::optional<gltf::Asset> asset;
    std::vector<gltf::Buffer> buffers;
    std::vector<gltf::BufferView> bufferViews;
    std::vector<gltf::Image> images;
    std::vector<gltf::Material> materials;
    std::vector<gltf::Mesh> meshes;
    std::vector<gltf::Node> nodes;
    std::vector<gltf::Sampler> samplers;
    u32 scene;
    std::vector<gltf::Scene> scenes;
    std::vector<gltf::Texture> textures;

    JSON(std::vector<u8> const &) noexcept;
};

} // namespace gltf
