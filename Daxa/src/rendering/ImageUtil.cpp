#include "ImageUtil.hpp"

namespace daxa {

    void generateMipLevels(CommandListHandle& cmdList, ImageViewHandle& img, VkImageSubresourceLayers layers, VkImageLayout postImageLayerLayouts) {
        i32 mipwidth = img->getImageHandle()->getVkExtent3D().width;
        i32 mipheight = img->getImageHandle()->getVkExtent3D().height;
        i32 mipdepth = img->getImageHandle()->getVkExtent3D().depth;
        for (u32 i = layers.mipLevel; i < img->getImageHandle()->getMipLevels() - 1; i++) {
            cmdList.queueImageBarrier({
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
                    .aspectMask = layers.aspectMask,
                    .baseMipLevel = i,
                    .levelCount = 1,
                    .baseArrayLayer = layers.baseArrayLayer,
                    .layerCount = layers.layerCount,
                }}
            });
            cmdList.queueImageBarrier({
                    .barrier = {
                        .dstStages = VK_PIPELINE_STAGE_2_BLIT_BIT_KHR,
                        .dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT_KHR,
                    },
                    .image = img,
                    .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .subRange = {VkImageSubresourceRange{
                        .aspectMask = layers.aspectMask,
                        .baseMipLevel = i+1,
                        .levelCount = 1,
                        .baseArrayLayer = layers.baseArrayLayer,
                        .layerCount = layers.layerCount,
                    }}
            });
            cmdList.insertQueuedBarriers();
            VkImageBlit blit{
                .srcSubresource = VkImageSubresourceLayers{
                    .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = i,
                    .baseArrayLayer = layers.baseArrayLayer,
                    .layerCount = layers.layerCount,
                },
                .srcOffsets = {
                    VkOffset3D{ 0, 0, 0 },
                    VkOffset3D{ mipwidth, mipheight, mipdepth },
                },
                .dstSubresource = VkImageSubresourceLayers{
                    .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = i+1,
                    .baseArrayLayer = layers.baseArrayLayer,
                    .layerCount = layers.layerCount,
                },
                .dstOffsets = {
                    VkOffset3D{ 0, 0, 0 },
                    VkOffset3D{ std::max(1, mipwidth / 2), std::max(1, mipheight / 2), std::max(1, mipdepth / 2) },
                },
            };

            vkCmdBlitImage(
                cmdList.getVkCommandBuffer(), 
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
            mipdepth = std::max(1, mipdepth / 2);
        }

        std::array<ImageBarrier, 32> buff;
        size_t buffSize = 0;

        for (u32 i = 0; i < img->getImageHandle()->getMipLevels() - 1; i++) {
            cmdList.queueImageBarrier({
                .barrier = MemoryBarrier{
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
                    .baseArrayLayer = layers.baseArrayLayer,
                    .layerCount = layers.layerCount,
                }},
            });
        }
        cmdList.queueImageBarrier({
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
                .baseArrayLayer = layers.baseArrayLayer,
                .layerCount = layers.layerCount,
            }},
        });
        cmdList.insertQueuedBarriers();
    }
}