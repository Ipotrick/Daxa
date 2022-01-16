#pragma once

#include <filesystem>

#include <glm/gtx/quaternion.hpp>

#include "Daxa.hpp"
#include "Components.hpp"
#include "cgltf.h"

daxa::Result<std::filesystem::path> completePath(
    std::vector<std::filesystem::path> const& possibleRoots, 
    std::filesystem::path path
) {
    if (!path.has_extension()) {
        if (!path.has_filename()) {
            path /= "scene";
        }
        path.replace_extension(".gltf");
    }

    std::optional<std::filesystem::path> completePathOpt = std::nullopt;
    std::filesystem::path buff;
    if (std::filesystem::exists(path)) {
        completePathOpt = path;
    } else {
        for (auto& root : possibleRoots) {
            buff.clear();
            buff = root / path;
            if (std::filesystem::exists(buff)) {
                completePathOpt = buff;
                break;
            }
        }
    }

    if (!completePathOpt.has_value()) {
        return daxa::ResultErr{"failed to find file with given path."};
    }
    return completePathOpt.value();
}

constexpr int GL_CONSTANT_LINEAR = 9729;
constexpr int GL_CONSTANT_LINEAR_MIPMAP_LINEAR = 9987;
constexpr int GL_CONSTANT_LINEAR_MIPMAP_NEAREST = 9985;
constexpr int GL_CONSTANT_NEAREST = 9728;
constexpr int GL_CONSTANT_NEAREST_MIPMAP_LINEAR = 9986;
constexpr int GL_CONSTANT_NEAREST_MIPMAP_NEAREST = 9984;

daxa::Result<std::pair<VkFilter, std::optional<VkSamplerMipmapMode>>> glSamplingToVkSampling(int glCode) {
    std::pair<VkFilter, std::optional<VkSamplerMipmapMode>> ret = {VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST};
    switch (glCode) {
        case GL_CONSTANT_LINEAR: 
            ret.first = VK_FILTER_LINEAR; break;
            ret.second = std::nullopt;
        case GL_CONSTANT_LINEAR_MIPMAP_LINEAR:
            ret.first = VK_FILTER_LINEAR;
            ret.second = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case GL_CONSTANT_LINEAR_MIPMAP_NEAREST:
            ret.first = VK_FILTER_LINEAR;
            ret.second = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case GL_CONSTANT_NEAREST:
            ret.first = VK_FILTER_NEAREST;
            ret.second = std::nullopt;
            break;
        case GL_CONSTANT_NEAREST_MIPMAP_LINEAR:
            ret.first = VK_FILTER_NEAREST;
            ret.second = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case GL_CONSTANT_NEAREST_MIPMAP_NEAREST:
            ret.first = VK_FILTER_NEAREST;
            ret.second = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        default: 
            return daxa::ResultErr{"invalid opengl sampler filter code"};
    }
    return ret;
}

constexpr int GL_CONSTANT_CLAMP_TO_EDGE = 33071;
constexpr int GL_CONSTANT_CLAMP_TO_BORDER = 33069;
constexpr int GL_CONSTANT_MIRRORED_REPEAT = 33648;
constexpr int GL_CONSTANT_MIRROR_CLAMP_TO_EDGE = 34627;
constexpr int GL_CONSTANT_REPEAT = 10497;

daxa::Result<VkSamplerAddressMode> glSamplerAdressModeToVk(int glCode) {
    switch (glCode) {
        case GL_CONSTANT_CLAMP_TO_EDGE:         return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case GL_CONSTANT_CLAMP_TO_BORDER:       return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    	case GL_CONSTANT_MIRRORED_REPEAT:       return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    	case GL_CONSTANT_MIRROR_CLAMP_TO_EDGE:  return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        case GL_CONSTANT_REPEAT:                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        default: return daxa::ResultErr{"invalid opengl sampler clamp code"};
    }
}

class SceneLoader {
public:
    SceneLoader(daxa::gpu::DeviceHandle& d, std::vector<std::filesystem::path> const& rp, std::shared_ptr<daxa::ImageCache>& ic)
        : device{ d }
        , rootPaths{ rp }
        , imgCache{ ic }
    {  

    }

    daxa::gpu::BufferHandle loadBuffer(daxa::gpu::CommandListHandle& cmdList, cgltf_accessor& accessor, VkBufferUsageFlagBits usage) {

        VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;

        daxa::gpu::BufferHandle gpuBuffer;

        if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
            gpuBuffer = device->createBuffer({
                .size = sizeof(u32) * accessor.count,
                .usage = usageFlags,
            });
        } else if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
            gpuBuffer = device->createBuffer({
                .size = accessor.stride * accessor.count,
                .usage = usageFlags,
            });
        }

        void* cpuSideBuffPtr = (void*)((u8*)(accessor.buffer_view->buffer->data) + accessor.buffer_view->offset + accessor.offset);

        if ((usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) && accessor.stride != 4) {
            auto mm = cmdList->mapMemoryStaged<u32>(gpuBuffer, sizeof(u32) * accessor.count, 0);

            switch (accessor.stride) {
                case 1:
                for (int i = 0; i < accessor.count; i++) {
                    mm.hostPtr[i] = ((u8*)cpuSideBuffPtr)[i];
                }
                break;
                case 2:
                for (int i = 0; i < accessor.count; i++) {
                    mm.hostPtr[i] = ((u16*)cpuSideBuffPtr)[i];
                }
                break;
                case 8:
                for (int i = 0; i < accessor.count; i++) {
                    mm.hostPtr[i] = ((u64*)cpuSideBuffPtr)[i];
                }
                break;
            }
        } else {
            cmdList->copyHostToBuffer({
                .dst = gpuBuffer,
                .size = accessor.stride * accessor.count,
                .src = cpuSideBuffPtr,
            });
        }

        return gpuBuffer;
    }

    daxa::EntityHandle nodeToEntity(
        daxa::gpu::CommandListHandle& cmdList,
        daxa::EntityHandle* parentEnt, 
        cgltf_node* node, 
        daxa::EntityComponentManager& ecm, 
        daxa::EntityComponentView<daxa::TransformComp, ModelComp, ChildComp>& view,
        cgltf_data* data,
        std::vector<daxa::gpu::BufferHandle>& buffers,
        std::vector<daxa::gpu::ImageHandle>& textures
    ) {
        glm::mat4 transform{1.0f};
        if (node->has_matrix) {
            transform = *(glm::mat4*)&node->matrix;
        } else {
            if (node->has_translation) {
                transform *= glm::translate(glm::mat4{ 1.0f }, *(glm::vec3*)&node->translation);
            }
            if (node->has_rotation) {
                transform *= glm::toMat4(*(glm::f32quat*)&node->rotation);
            }
            if (node->has_scale) {
                transform *= glm::scale(glm::mat4{1.0f}, *(glm::vec3*)&node->scale);
            }
        }

        auto ent = ecm.createEntity();
        view.addComp(ent, daxa::TransformComp{ .mat = transform });

        if (parentEnt) {
            view.addComp(ent, ChildComp{ .parent = *parentEnt });
        }
        
        if (node->mesh) {
            auto& rootMesh = *node->mesh;
            
            ModelComp& model = view.addComp(ent, ModelComp{});

            for (auto primI = 0; primI < rootMesh.primitives_count; primI++) {
                auto& prim = rootMesh.primitives[primI];
                Primitive meshPrim;

                meshPrim.indexCount = prim.indices->count;
                size_t bufferIndex = prim.indices - data->accessors;
                printf("use buffer with index %i as index buffer\n", bufferIndex);
                if (!buffers[bufferIndex]) {
                    buffers[bufferIndex] = loadBuffer(cmdList, *prim.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
                }
                meshPrim.indiexBuffer = buffers[bufferIndex];

                for (int attrI = 0; attrI < prim.attributes_count; attrI++) {
                    auto& attribute = prim.attributes[attrI];
                    size_t bufferIndexOfAttrib = attribute.data - data->accessors;
                    
                    if (!buffers[bufferIndexOfAttrib]) {
                        buffers[bufferIndexOfAttrib] = loadBuffer(cmdList, *attribute.data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
                    }

                    switch (attribute.type) {
                        case cgltf_attribute_type_color: 
                            printf("models dont support vertex colors\n");
                            break;
                        case cgltf_attribute_type_position:
                            printf("use buffer with index %i as position vertex buffer\n", bufferIndexOfAttrib);
                            meshPrim.vertexPositions = buffers[bufferIndexOfAttrib];
                            break;
                        case cgltf_attribute_type_texcoord:
                            printf("use buffer with index %i as tex coord vertex buffer\n", bufferIndexOfAttrib);
                            meshPrim.vertexUVs = buffers[bufferIndexOfAttrib];
                            break;
                        case cgltf_attribute_type_normal:
                            printf("use buffer with index %i as normals vertex buffer\n", bufferIndexOfAttrib);
                            meshPrim.vertexNormals = buffers[bufferIndexOfAttrib];
                            break;
                    }
                }

                if (prim.material->pbr_metallic_roughness.base_color_texture.texture) {
                    size_t textureIndex = prim.material->pbr_metallic_roughness.base_color_texture.texture - data->textures;
                    meshPrim.albedoTexture = textures[textureIndex];
                }

                if (prim.material->normal_texture.texture) {
                    size_t textureIndex = prim.material->normal_texture.texture - data->textures;
                    meshPrim.normalTexture = textures[textureIndex];
                }

                model.meshes.push_back(std::move(meshPrim));
            }
        } else {
            printf("node has no mesh\n");
        }

        for (int childI = 0; childI < node->children_count; childI++) {
            nodeToEntity(cmdList, &ent, node->children[childI], ecm, view, data, buffers, textures);
        }

        return ent;
    }

    daxa::Result<std::vector<daxa::EntityHandle>> loadScene(
        daxa::gpu::CommandListHandle& cmdList,
        std::filesystem::path path,
        daxa::EntityComponentManager& ecm,
        bool convertYtoZup
    ) {
        if (!std::filesystem::exists(path)) {
            auto pathRes = completePath(rootPaths, path);
            if (pathRes.isErr()) {
                return daxa::ResultErr{ .message = pathRes.message() };
            }
            path = pathRes.value();
        }

        cgltf_options options = {};
        cgltf_data* data = NULL;
        cgltf_result resultParsing = cgltf_parse_file(&options, path.string().c_str(), &data);
        if (resultParsing != cgltf_result_success) {
            return daxa::ResultErr{std::string_view("failed to parse gltf file")};
        }

        auto resultBinLoading = cgltf_load_buffers(&options, data, path.string().c_str());
        if (resultBinLoading != cgltf_result_success) {
            return daxa::ResultErr{ "failed to load .bin of gltf file" };
        }

        // path now is the relative path to the folder of the gltf file, this is used to load other data now
        path.remove_filename(); 

        std::vector<daxa::gpu::ImageHandle> textures;
        for (int i = 0; i < data->textures_count; i++) {
            auto& texture = data->textures[i];

            // TODO make it possible to read images, wich are embedded inside the .bin

            printf("texture nr %i\n", i);
            printf("texture embedded in .bin: %s\n", texture.image->buffer_view != nullptr ? "true" : "false");
            printf("texture uri: %s\n", data->textures[i].image->uri);
            printf("texture min filter: %i\n", texture.sampler->min_filter);
            printf("texture mag filter: %i\n", texture.sampler->mag_filter);
            printf("texture wrap s: %i\n", texture.sampler->wrap_s);
            printf("texture wrap t: %i\n", texture.sampler->wrap_t);

            daxa::gpu::SamplerCreateInfo samplerInfo = {};
            {
                auto ret = glSamplingToVkSampling(texture.sampler->min_filter);
                if (ret.isErr()) {
                    return daxa::ResultErr{ret.message()};
                }
                samplerInfo.minFilter = ret.value().first;
                if (ret.value().second.has_value()) {
                    samplerInfo.mipmapMode = ret.value().second.value();
                }
            }
            {
                auto ret = glSamplingToVkSampling(texture.sampler->mag_filter);
                if (ret.isErr()) {
                    return daxa::ResultErr{ ret.message() };
                }
                samplerInfo.magFilter = ret.value().first;
                if (ret.value().second.has_value()) {
                    samplerInfo.mipmapMode = ret.value().second.value();
                }
            }
            {
                auto ret = glSamplerAdressModeToVk(texture.sampler->wrap_s);
                if (!ret) {
                    return daxa::ResultErr{ ret.message() };
                }
                samplerInfo.addressModeU = ret.value();
            }
            {
                auto ret = glSamplerAdressModeToVk(texture.sampler->wrap_t);
                if (!ret) {
                    return daxa::ResultErr{ ret.message() };
                }
                samplerInfo.addressModeV = ret.value();
            }

            std::filesystem::path texPath;

            texPath = path / data->textures[i].image->uri;
            printf("texture path %s\n",texPath.c_str());

            daxa::ImageCacheFetchInfo fetchI{
                .path = texPath.c_str(), 
                .samplerInfo = samplerInfo,
            };
            if (texture.image->buffer_view) {
                fetchI.preload = (u8*)texture.image->buffer_view->data;
                fetchI.preloadSize = texture.image->buffer_view->size;
            }

            textures.push_back(imgCache->get(fetchI, cmdList));
        }

        printf("\n");

        std::vector<daxa::gpu::BufferHandle> buffers;
        buffers.resize(data->accessors_count, {});

        printf("\n");

        auto view = ecm.view<daxa::TransformComp, ModelComp, ChildComp>();

        std::vector<daxa::EntityHandle> ret;

        for (int scene_i = 0; scene_i < data->scenes_count; scene_i++) {
            auto& scene = data->scenes[scene_i];
            for (int rootNodeI = 0; rootNodeI < scene.nodes_count; rootNodeI++) {
                auto* rootNode = scene.nodes[rootNodeI];

                auto ent = nodeToEntity(cmdList, nullptr, rootNode, ecm, view, data, buffers, textures);
                ret.push_back(ent);
            }
        }

        if (convertYtoZup) {
            for (auto ent: ret) {
                if (daxa::TransformComp* trans = view.getCompIf<daxa::TransformComp>(ent)) {
                    trans->mat = glm::rotate(trans->mat, glm::radians(90.0f), glm::vec3(1,0,0)); 
                }
            }
        }

        cmdList->insertMemoryBarrier(daxa::gpu::FULL_MEMORY_BARRIER);

        this->textures = textures;
        return daxa::Result(std::vector<daxa::EntityHandle>());
    }

    std::vector<daxa::gpu::ImageHandle> textures;
private:
    daxa::gpu::DeviceHandle device = {};
    std::vector<std::filesystem::path> rootPaths = {};
    std::shared_ptr<daxa::ImageCache> imgCache = {};
};

