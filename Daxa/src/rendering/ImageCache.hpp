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

        std::optional<gpu::SamplerCreateInfo> samplerInfo = std::nullopt;
        
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

    struct ImageCacheFetchInfoHasher {
        std::size_t operator()(ImageCacheFetchInfo const& info) const {
            size_t hash = std::filesystem::hash_value(info.path);
            hash <<= 3;
            hash ^= (size_t)info.preload;
            hash ^= info.preloadSize;
            hash <<= 3;
            hash ^= (size_t)info.viewFormat;
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
        ImageCache(gpu::DeviceHandle device) 
            : device{ std::move(device) }
        { }

        gpu::ImageViewHandle get(ImageCacheFetchInfo const& info, gpu::CommandListHandle& cmdList) {
            if (!cache.contains(info)) {
                printf("image cache miss\n");
                cache[info] = loadImage(info, cmdList);
            } else {
                printf("image cache hit\n");
            }
            return cache[info];
        }

        std::unordered_map<ImageCacheFetchInfo, gpu::ImageViewHandle, ImageCacheFetchInfoHasher> cache = {};
    private:
        gpu::ImageViewHandle loadImage(ImageCacheFetchInfo const& info, gpu::CommandListHandle& cmdList) {
            //stbi_set_flip_vertically_on_load(1);
            int width, height, channels;
            u8* data;
            if (info.preload) {
                printf("try load from mem\n");
                data = stbi_load_from_memory(info.preload, info.preloadSize, &width, &height, &channels, 4);
            } else {
                data = stbi_load((char const*)info.path.string().c_str(), &width, &height, &channels, 4);
            }
            printf("loaded image from path: %s\n", (char const*)info.path.string().c_str());
            
            u32 mipmaplevels = std::log2(std::max(width, height));

            if (!data) {
                return {};
            } else {
                auto ci = gpu::ImageViewCreateInfo{
                    .image = device->createImage({
                        .format = info.viewFormat,
                        .extent = { (u32)width, (u32)height, 1 },
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
                    sampler_v.maxLod = mipmaplevels - 1.0f;
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

                cmdList->insertImageBarrier({
                    .image = image,
                    .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                });
                cmdList->copyHostToImage({
                    .src = data,
                    .dst = image,
                    .size = static_cast<u32>(width*height*4),
                });
                if (false) {
                    cmdList->insertImageBarrier({
                        .barrier = {
                            .srcStages = gpu::STAGE_TRANSFER,
                            .srcAccess = gpu::ACCESS_MEMORY_READ,
                            .dstStages = gpu::STAGE_ALL_COMMANDS,
                            .dstAccess = gpu::ACCESS_MEMORY_WRITE,
                        },
                        .image = image,
                        .layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    });
                }
                else {
                    generateMipLevels(cmdList, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                }

                std::free(data);
                return image;
            }
        }

        gpu::DeviceHandle device = {};

        std::unordered_map<gpu::SamplerCreateInfo, gpu::SamplerHandle, GPUSamplerCreateInfoHasher> samplers = {};
    };
}