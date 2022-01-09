#pragma once

#include <filesystem>

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
        case GL_CONSTANT_CLAMP_TO_EDGE:         return daxa::Result<VkSamplerAddressMode>::ok(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        case GL_CONSTANT_CLAMP_TO_BORDER:       return daxa::Result<VkSamplerAddressMode>::ok(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    	case GL_CONSTANT_MIRRORED_REPEAT:       return daxa::Result<VkSamplerAddressMode>::ok(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT);
    	case GL_CONSTANT_MIRROR_CLAMP_TO_EDGE:  return daxa::Result<VkSamplerAddressMode>::ok(VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE);
        case GL_CONSTANT_REPEAT:                return daxa::Result<VkSamplerAddressMode>::ok(VK_SAMPLER_ADDRESS_MODE_REPEAT);
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

    daxa::Result<std::vector<daxa::EntityHandle>> loadScene(
        daxa::gpu::CommandListHandle& cmdList,
        std::filesystem::path path,
        daxa::EntityComponentManager& ecm
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
        cgltf_result result = cgltf_parse_file(&options, path.string().c_str(), &data);
        if (result != cgltf_result_success) {
            return daxa::ResultErr{std::string_view("failed to parse gltf file")};
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

            texPath = path /= data->textures[i].image->uri;
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

        this->textures = textures;
        return daxa::Result(std::vector<daxa::EntityHandle>());
    }

    std::vector<daxa::gpu::ImageHandle> textures;
private:
    daxa::gpu::DeviceHandle device = {};
    std::vector<std::filesystem::path> rootPaths = {};
    std::shared_ptr<daxa::ImageCache> imgCache = {};
};

