#pragma once

#include "../DaxaCore.hpp"

#include <unordered_map>
#include <filesystem>
#include <fstream>

#include "stb_image.h"

#include "../gpu/Device.hpp"
#include "ImageUtil.hpp"

namespace daxa {
    struct ImageCacheFetchInfo {
        std::filesystem::path path = {};
        VkFormat viewFormat = VK_FORMAT_R8G8B8A8_SRGB;
        u8* preload = nullptr;
        size_t preloadSize = 0;

        std::optional<SamplerCreateInfo> samplerInfo = std::nullopt;
        
        bool operator == (ImageCacheFetchInfo const& other) const {
            return 
                this->path == other.path && 
                this->preload == other.preload &&
                this->preloadSize == other.preloadSize &&
                this->viewFormat == other.viewFormat &&
                this->samplerInfo == other.samplerInfo;
        }
    };

    struct GPUSamplerCreateInfoHasher {
        std::size_t operator()(SamplerCreateInfo const& info) const {
            size_t hash = 0x43fb87da;
            u32 const* data = reinterpret_cast<u32 const*>(&info);
            for (size_t i = 0; i < sizeof(SamplerCreateInfo) / 4; i++) {
                hash ^= data[i];
                hash <<= 1;
            }
            return hash;
        }
    };

    struct ImageCacheFetchInfoHasher {
        std::size_t operator()(ImageCacheFetchInfo const& info) const {
            size_t hash = std::filesystem::hash_value(info.path);
            hash <<= 3;
            hash ^= reinterpret_cast<size_t>(info.preload);
            hash ^= info.preloadSize;
            hash <<= 3;
            hash ^= static_cast<size_t>(info.viewFormat);
            hash <<= 1;
            if (info.samplerInfo.has_value()) {
                hash ^= GPUSamplerCreateInfoHasher{}(info.samplerInfo.value());
            }
            hash <<= 2;
            return hash;
        }
    };

    class ImageCache {
    public:
        ImageCache(DeviceHandle device) 
            : device{ std::move(device) }
        { }

        ImageViewHandle get(ImageCacheFetchInfo const& info, CommandListHandle& cmdList) {
            if (!cache.contains(info)) {
                printf("image cache miss\n");
                cache[info] = loadImage(info, cmdList);
            } else {
                printf("image cache hit\n");
            }
            return cache[info];
        }

        std::unordered_map<ImageCacheFetchInfo, ImageViewHandle, ImageCacheFetchInfoHasher> cache = {};
    private:
        ImageViewHandle loadImage(ImageCacheFetchInfo const& info, CommandListHandle& cmdList) {
            //stbi_set_flip_vertically_on_load(1);
            int width, height, channels;
            u8* data;
            if (info.preload) {
                printf("try load from mem\n");
                data = stbi_load_from_memory(info.preload, static_cast<int>(info.preloadSize), &width, &height, &channels, 4);
            } else {
                data = stbi_load(info.path.string().c_str(), &width, &height, &channels, 4);
            }
            printf("loaded image from path: %s\n", info.path.string().c_str());
            
            u32 mipmaplevels = static_cast<u32>(std::log2(std::max(width, height)));

            if (!data) {
                return {};
            } else {
                auto ci = ImageViewCreateInfo{
                    .image = device->createImage({
                        .format = info.viewFormat,
                        .extent = { static_cast<u32>(width), static_cast<u32>(height), 1 },
                        .mipLevels = mipmaplevels,
                        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    }),
                    .format = info.viewFormat, 
                };
                if (info.samplerInfo.has_value()) {
                    printf("create with sampler\n");
                    auto sampler_v = info.samplerInfo.value();
                    sampler_v.anisotropyEnable = VK_TRUE;
                    sampler_v.maxAnisotropy = 16.0f;
                    sampler_v.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
                    sampler_v.maxLod = static_cast<float>(mipmaplevels) - 1.0f;
                    if (!samplers.contains(sampler_v)) {
                        samplers[sampler_v] = device->createSampler(sampler_v);
                    }
                    ci.defaultSampler = samplers[sampler_v];
                    ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    ci.subresourceRange.baseMipLevel = 0;
                    ci.subresourceRange.levelCount = mipmaplevels;
                    ci.subresourceRange.baseArrayLayer = 0;
                    ci.subresourceRange.layerCount = 1;
                }
                auto image = device->createImageView(ci);

                cmdList.queueImageBarrier({
                    .image = image,
                    .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                });
                cmdList.singleCopyHostToImage({
                    .src = data,
                    .dst = image->getImageHandle(),
                    //.size = static_cast<u32>(width*height*4),
                });
                generateMipLevels(
                    cmdList, 
                    image, 
                    VkImageSubresourceLayers{
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .mipLevel = 0,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                );

                std::free(data);
                return image;
            }
        }

        DeviceHandle device = {};

        std::unordered_map<SamplerCreateInfo, SamplerHandle, GPUSamplerCreateInfoHasher> samplers = {};
    };
}