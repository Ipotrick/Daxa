#pragma once

#include "../DaxaCore.hpp"

#include <unordered_map>
#include <filesystem>
#include <fstream>

#include "stb_image.h"

#include "../gpu/Device.hpp"

namespace daxa {
    struct ImageCacheFetchInfo {
        std::filesystem::path path = {};
        u8* preload = nullptr;
        size_t preloadSize = 0;

        std::optional<gpu::SamplerCreateInfo> samplerInfo = std::nullopt;
        
        bool operator == (ImageCacheFetchInfo const& other) const {
            return this->path == other.path && this->samplerInfo == other.samplerInfo;
        }
    };

    struct ImageCacheFetchInfoHasher {
        std::size_t operator()(ImageCacheFetchInfo const& info) const {
            return std::filesystem::hash_value(info.path);
        }
    };

    struct GPUSamplerCreateInfoHasher {
        std::size_t operator()(gpu::SamplerCreateInfo const& info) const {
            size_t hash = 0x43fb87da;
            u32 const* data = (u32 const*)(&info);
            for (int i = 0; i < sizeof(gpu::SamplerCreateInfo) / 4; i++) {
                hash ^= data[i];
                hash <<= 1;
            }
            return hash;
        }
    };

    class ImageCache {
    public:
        ImageCache(gpu::DeviceHandle device) 
            : device{ std::move(device) }
        { }

        void initDefaultTexture(gpu::CommandListHandle& cmdList) {
            dummyImage = this->device->createImage2d({.sampler = this->device->createSampler({})});
            u32 dummyColor = 0x8B008BFF;    // purple
            cmdList->copyHostToImageSynced({
                .size = sizeof(u32), 
                .dst = dummyImage, 
                .dstFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
                .src = &dummyColor,
            });
        }

        gpu::ImageHandle get(ImageCacheFetchInfo const& info, gpu::CommandListHandle& cmdList) {
            if (!cache.contains(info)) {
                cache[info] = loadImage(info, cmdList);
            }
            return cache[info];
        }

    private:
        gpu::ImageHandle loadImage(ImageCacheFetchInfo const& info, gpu::CommandListHandle& cmdList) {
            //stbi_set_flip_vertically_on_load(1);
            int width, height, channels;
            u8* data;
            if (info.preload) {
                printf("try load from mem\n");
                data = stbi_load_from_memory(info.preload, info.preloadSize, &width, &height, &channels, 4);
            } else {
                data = stbi_load((char const*)info.path.string().c_str(), &width, &height, &channels, 4);
            }

            if (!data) {
                return dummyImage;
            } else {
                auto ci = gpu::Image2dCreateInfo{.width = (u32)width, .height = (u32)height, .imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};
                if (info.samplerInfo.has_value()) {
                    printf("create with sampler\n");
                    auto& sampler_v = info.samplerInfo.value();
                    if (!samplers.contains(sampler_v)) {
                        samplers[sampler_v] = device->createSampler(sampler_v);
                    }
                    ci.sampler = samplers[sampler_v];
                }
                auto image = device->createImage2d(ci);
                cmdList->copyHostToImageSynced({.dst = image, .dstFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, .src = data, .size = (u32)(width*height*4)});
                std::free(data);
                return image;
            }
        }

        gpu::DeviceHandle device = {};

        gpu::ImageHandle dummyImage = {};
        std::unordered_map<ImageCacheFetchInfo, gpu::ImageHandle, ImageCacheFetchInfoHasher> cache = {};
        std::unordered_map<gpu::SamplerCreateInfo, gpu::SamplerHandle, GPUSamplerCreateInfoHasher> samplers = {};
    };
}