#include "ImageUtil.hpp"

namespace daxa {

    void generateMipLevels(gpu::CommandListHandle& cmdList, gpu::ImageViewHandle& img, VkImageLayout dstImageLayoutsAllLevels) {
        i32 mipwidth = img->getImageHandle()->getVkExtent3D().width;
        i32 mipheight = img->getImageHandle()->getVkExtent3D().height;
        for (u32 i = 0; i < img->getImageHandle()->getMipLevels() - 1; i++) {
            cmdList->insertImageBarriers(std::array{
                gpu::ImageBarrier{
                    .barrier = {
                        .srcStages = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
                        .srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR,
                        .dstStages = VK_PIPELINE_STAGE_2_BLIT_BIT_KHR,
                        .dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT_KHR,
                    },
                    .image = img,
                    .layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    .subRange = {VkImageSubresourceRange{
                        .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = i,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    }}
                },
                gpu::ImageBarrier{
                    .barrier = {
                        .dstStages = VK_PIPELINE_STAGE_2_BLIT_BIT_KHR,
                        .dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT_KHR,
                    },
                    .image = img,
                    .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .subRange = {VkImageSubresourceRange{
                        .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = i+1,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    }}
                },
            });
            VkImageBlit blit{
                .srcSubresource = VkImageSubresourceLayers{
                    .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = i,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
                .srcOffsets = {
                    VkOffset3D{ 0, 0, 0 },
                    VkOffset3D{ mipwidth, mipheight, 1},
                },
                .dstSubresource = VkImageSubresourceLayers{
                    .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = i+1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
                .dstOffsets = {
                    VkOffset3D{ 0, 0, 0 },
                    VkOffset3D{ std::max(1, mipwidth / 2), std::max(1, mipheight / 2), 1},
                },
            };

            vkCmdBlitImage(
                cmdList->getVkCommandBuffer(), 
                img->getImageHandle()->getVkImage(),
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                img->getImageHandle()->getVkImage(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &blit,
                VK_FILTER_LINEAR
            );

            mipwidth = std::max(1, mipwidth / 2);
            mipheight = std::max(1, mipheight / 2);
        }

        std::array<gpu::ImageBarrier, 32> buff;
        size_t buffSize = 0;

        for (u32 i = 0; i < img->getImageHandle()->getMipLevels() - 1; i++) {
            buff[buffSize++] = gpu::ImageBarrier{
                .barrier = gpu::MemoryBarrier{
                    .srcStages = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
                    .srcAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
                    .dstStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
                    .dstAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
                },
                .image = img,
                .layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .subRange = {VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = i,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }},
            };
        }

        buff[buffSize++] = gpu::ImageBarrier{
            .barrier = {
                .srcStages = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
                .srcAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
                .dstStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR,
                .dstAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
            },
            .image = img,
            .layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .subRange = {VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = img->getImageHandle()->getMipLevels() - 1,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }},
        };

        cmdList->insertBarriers({}, {buff.data(), buffSize});
    }
}