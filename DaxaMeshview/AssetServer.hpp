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

class AssetCache {
public:
    AssetCache(daxa::DeviceHandle& d, std::vector<std::filesystem::path> const& rp, std::shared_ptr<daxa::ImageCache>& ic)
        : device{ d }
        , rootPaths{ rp }
        , imgCache{ ic }
    {  

    }

    daxa::BufferHandle loadBuffer(daxa::CommandListHandle& cmdList, cgltf_accessor& accessor, VkBufferUsageFlagBits usage) {

        // VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;

        daxa::BufferHandle gpuBuffer;

        if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
            gpuBuffer = device->createBuffer({
                .size = sizeof(u32) * accessor.count,
                //.usage = usageFlags,
                .debugName = "an index buffer",
            });
        } else if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
            gpuBuffer = device->createBuffer({
                .size = accessor.stride * accessor.count,
                //.usage = usageFlags,
                .debugName = "a vertex buffer",
            });
        }

        void* cpuSideBuffPtr = reinterpret_cast<void*>(reinterpret_cast<u8*>(accessor.buffer_view->buffer->data) + accessor.buffer_view->offset + accessor.offset);

        if ((usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) && accessor.stride != 4) {
            auto mm = cmdList.mapMemoryStagedBuffer(gpuBuffer, sizeof(u32) * accessor.count, 0);

            switch (accessor.stride) {
                case 1:
                for (size_t i = 0; i < accessor.count; i++) {
                    reinterpret_cast<u32*>(mm.hostPtr)[i] = (reinterpret_cast<u8*>(cpuSideBuffPtr))[i];
                }
                break;
                case 2:
                for (size_t i = 0; i < accessor.count; i++) {
                    reinterpret_cast<u32*>(mm.hostPtr)[i] = (reinterpret_cast<u16*>(cpuSideBuffPtr))[i];
                }
                break;
                case 8:
                for (size_t i = 0; i < accessor.count; i++) {
                    reinterpret_cast<u32*>(mm.hostPtr)[i] = static_cast<u32>(reinterpret_cast<u64*>(cpuSideBuffPtr)[i]);
                }
                break;
            }
        } else {
            cmdList.singleCopyHostToBuffer({
                .src = reinterpret_cast<u8*>(cpuSideBuffPtr),
                .dst = gpuBuffer,
                .region = {
                    .size = accessor.stride * accessor.count
                }
                //.size = accessor.stride * accessor.count,
            });
        }

        return gpuBuffer;
    }

    daxa::EntityHandle nodeToEntity(
        daxa::CommandListHandle& cmdList,
        daxa::EntityHandle* parentEnt, 
        cgltf_node* node, 
        daxa::EntityComponentManager& ecm, 
        daxa::EntityComponentView<daxa::TransformComp, ModelComp, ChildComp, ParentComp>& view,
        cgltf_data* data,
        std::vector<daxa::BufferHandle>& buffers
    ) {
        glm::mat4 transform{1.0f};
        if (node->has_matrix) {
            transform = *reinterpret_cast<glm::mat4*>(&node->matrix);
        } else {
            if (node->has_translation) {
                transform *= glm::translate(glm::mat4{ 1.0f }, *reinterpret_cast<glm::vec3*>(&node->translation));
            }
            if (node->has_rotation) {
                transform *= glm::toMat4(*reinterpret_cast<glm::f32quat*>(&node->rotation));
            }
            if (node->has_scale) {
                transform *= glm::scale(glm::mat4{1.0f}, *reinterpret_cast<glm::vec3*>(&node->scale));
            }
        }

        auto ent = ecm.createEntity();
        view.addComp(ent, daxa::TransformComp{ .mat = transform });

        if (parentEnt) {
            view.addComp(ent, ChildComp{ .parent = *parentEnt });
            if (!view.hasComp<ParentComp>(*parentEnt)) {
                view.addComp<ParentComp>(*parentEnt, {});
            }
            view.getComp<ParentComp>(*parentEnt).children.push_back(ent);
        }
        
        if (node->mesh) {
            auto& rootMesh = *node->mesh;
            
            ModelComp& model = view.addComp(ent, ModelComp{});

            for (size_t primI = 0; primI < rootMesh.primitives_count; primI++) {
                auto& prim = rootMesh.primitives[primI];
                Primitive meshPrim;

                meshPrim.indexCount = static_cast<u32>(prim.indices->count);
                size_t bufferIndex = static_cast<size_t>(std::distance(data->accessors, prim.indices));
                //printf("use buffer with index %i as index buffer\n", bufferIndex);
                if (!buffers[bufferIndex]) {
                    buffers[bufferIndex] = loadBuffer(cmdList, *prim.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
                }
                meshPrim.indiexBuffer = buffers[bufferIndex];

                for (size_t attrI = 0; attrI < prim.attributes_count; attrI++) {
                    auto& attribute = prim.attributes[attrI];
                    size_t bufferIndexOfAttrib = static_cast<size_t>(std::distance(data->accessors, attribute.data));
                    
                    if (!buffers[bufferIndexOfAttrib]) {
                        buffers[bufferIndexOfAttrib] = loadBuffer(cmdList, *attribute.data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
                    }

                    switch (attribute.type) {
                        case cgltf_attribute_type_color: 
                            //printf("models dont support vertex colors\n");
                            break;
                        case cgltf_attribute_type_position:
                            //printf("use buffer with index %i as position vertex buffer\n", bufferIndexOfAttrib);
                            meshPrim.vertexPositions = buffers[bufferIndexOfAttrib];
                            break;
                        case cgltf_attribute_type_texcoord:
                            //printf("use buffer with index %i as tex coord vertex buffer\n", bufferIndexOfAttrib);
                            meshPrim.vertexUVs = buffers[bufferIndexOfAttrib];
                            break;
                        case cgltf_attribute_type_normal:
                            //printf("use buffer with index %i as normals vertex buffer\n", bufferIndexOfAttrib);
                            meshPrim.vertexNormals = buffers[bufferIndexOfAttrib];
                            break;
                        case cgltf_attribute_type_tangent:
                            printf("use buffer with index: %llu as tantents vertex buffer\n", bufferIndexOfAttrib);
                            meshPrim.vertexTangents = buffers[bufferIndexOfAttrib];
                            break;
                        default: break;
                    }
                }

                if (prim.material->pbr_metallic_roughness.base_color_texture.texture) {
                    size_t textureIndex = static_cast<size_t>(std::distance(data->textures, prim.material->pbr_metallic_roughness.base_color_texture.texture));
                    meshPrim.albedoMap = imgCache->get(
                        {
                            .path = texturePaths[textureIndex],
                            .viewFormat = VK_FORMAT_R8G8B8A8_SRGB,
                            .samplerInfo = textureSamplerInfos[textureIndex],
                        },
                        cmdList
                    );
                }

                if (prim.material->normal_texture.texture) {
                    printf("normals\n");
                    size_t textureIndex = static_cast<size_t>(std::distance(data->textures, prim.material->normal_texture.texture));
                    meshPrim.normalMap = imgCache->get(
                        {
                            .path = texturePaths[textureIndex],
                            .viewFormat = VK_FORMAT_R8G8B8A8_UNORM,
                            .samplerInfo = textureSamplerInfos[textureIndex],
                        },
                        cmdList
                    );
                }

                model.meshes.push_back(std::move(meshPrim));
            }
        } else {
            //printf("node has no mesh\n");
        }

        for (size_t childI = 0; childI < node->children_count; childI++) {
            nodeToEntity(cmdList, &ent, node->children[childI], ecm, view, data, buffers);
        }

        return ent;
    }

    daxa::Result<daxa::EntityHandle> loadScene(
        daxa::CommandListHandle& cmdList,
        std::filesystem::path path
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
            cgltf_free(data);
            return daxa::ResultErr{"failed to parse gltf file"};
        }

        auto resultBinLoading = cgltf_load_buffers(&options, data, path.string().c_str());
        if (resultBinLoading != cgltf_result_success) {
            cgltf_free(data);
            return daxa::ResultErr{ "failed to load .bin of gltf file" };
        }

        // path now is the relative path to the folder of the gltf file, this is used to load other data now
        path.remove_filename(); 

        for (size_t i = 0; i < data->textures_count; i++) {
            auto& texture = data->textures[i];

            daxa::SamplerCreateInfo samplerInfo = {};
            {
                auto ret = glSamplingToVkSampling(texture.sampler->min_filter);
                if (ret.isErr()) {
                    cgltf_free(data);
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
                    cgltf_free(data);
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
                    cgltf_free(data);
                    return daxa::ResultErr{ ret.message() };
                }
                samplerInfo.addressModeU = ret.value();
            }
            {
                auto ret = glSamplerAdressModeToVk(texture.sampler->wrap_t);
                if (!ret) {
                    cgltf_free(data);
                    return daxa::ResultErr{ ret.message() };
                }
                samplerInfo.addressModeV = ret.value();
            }

            std::filesystem::path texPath;

            texPath = path / data->textures[i].image->uri;

            textureSamplerInfos.push_back(samplerInfo);
            texturePaths.push_back(texPath);
        }

        std::vector<daxa::BufferHandle> buffers;
        buffers.resize(data->accessors_count);

        auto view = entityCache.view<daxa::TransformComp, ModelComp, ChildComp, ParentComp>();
        auto ret = entityCache.createEntity();
        view.addComp(ret, daxa::TransformComp{ .mat = glm::mat4{1.0f} });
        view.addComp(ret, ParentComp{});

        for (size_t scene_i = 0; scene_i < data->scenes_count; scene_i++) {
            auto& scene = data->scenes[scene_i];
            for (size_t rootNodeI = 0; rootNodeI < scene.nodes_count; rootNodeI++) {
                auto* rootNode = scene.nodes[rootNodeI];

                nodeToEntity(cmdList, &ret, rootNode, entityCache, view, data, buffers);
            }
        }

        cmdList.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        cmdList.insertQueuedBarriers();
        
        textureSamplerInfos.clear();
        texturePaths.clear();
        
        cgltf_free(data);
        return ret;
    }

    void copyEntitiesFromCache(
        daxa::EntityHandle from, 
        daxa::EntityHandle to, 
        daxa::EntityComponentView<daxa::TransformComp, ModelComp, ChildComp, ParentComp>& cacheView,
        daxa::EntityComponentView<daxa::TransformComp, ModelComp, ChildComp, ParentComp>& outView,
        daxa::EntityComponentManager& out
    ) {
        outView.addComp<daxa::TransformComp>(to, cacheView.getComp<daxa::TransformComp>(from));
        if (cacheView.hasComp<ModelComp>(from)) {
            outView.addComp(to, cacheView.getComp<ModelComp>(from));
        }
        if (cacheView.hasComp<ParentComp>(from)) {
            outView.addComp(to, ParentComp{});
            outView.getComp<ParentComp>(to).children.reserve(cacheView.getComp<ParentComp>(from).children.size());
            for (auto& fromChild : cacheView.getComp<ParentComp>(from).children) {
                auto toChild = out.createEntity();
                outView.addComp(toChild, ChildComp{ .parent = to });
                copyEntitiesFromCache(fromChild, toChild, cacheView, outView, out);
                outView.getComp<ParentComp>(to).children.push_back(toChild);
            }
        }
    }

    daxa::Result<daxa::EntityHandle> getScene(daxa::CommandListHandle& cmdList, std::filesystem::path const& path, daxa::EntityComponentManager& out) {
        if (!cache.contains(path.string())) {
            auto rootEnt = loadScene(cmdList, path);
            if (rootEnt.isErr()) {
                return daxa::ResultErr{ rootEnt.message() };
            }
            cache[path.string()] = rootEnt.value();
        }

        auto from = cache[path.string()];
        auto to = out.createEntity();
        auto cacheView = entityCache.view<daxa::TransformComp, ModelComp, ChildComp, ParentComp>();
        auto outView = out.view<daxa::TransformComp, ModelComp, ChildComp, ParentComp>();
        copyEntitiesFromCache(from, to, cacheView, outView, out);
        return to;
    }

private:
    std::vector<daxa::SamplerCreateInfo> textureSamplerInfos;
    std::vector<std::filesystem::path> texturePaths;
    daxa::DeviceHandle device = {};
    std::vector<std::filesystem::path> rootPaths = {};
    std::shared_ptr<daxa::ImageCache> imgCache = {};
    daxa::EntityComponentManager entityCache = {};
    std::unordered_map<std::string, daxa::EntityHandle> cache;
};
