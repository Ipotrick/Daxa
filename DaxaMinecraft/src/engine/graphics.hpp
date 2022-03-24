#pragma once

#include <Daxa.hpp>

// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>

struct RenderContext {
    daxa::gpu::DeviceHandle device;
    daxa::PipelineCompilerHandle pipeline_compiler;
    daxa::gpu::CommandQueueHandle queue;

    daxa::gpu::SwapchainHandle swapchain;
    daxa::gpu::SwapchainImage swapchain_image;
    daxa::gpu::ImageViewHandle render_image;
    daxa::gpu::ImageViewHandle depth_image;

    struct PerFrameData {
        daxa::gpu::SignalHandle present_signal;
        daxa::gpu::TimelineSemaphoreHandle timeline;
        u64 timeline_counter = 0;
    };
    std::deque<PerFrameData> frames;

    auto create_render_image(int32_t sx, int32_t sy) {
        auto result = device->createImageView({
            .image = device->createImage({
                .format = VK_FORMAT_B8G8R8A8_SRGB,
                .extent = {(uint32_t)sx, (uint32_t)sy, 1},
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                .debugName = "Render Image",
            }),
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .debugName = "Render Image View",
        });
        return result;
    }

    auto create_depth_image(int32_t sx, int32_t sy) {
        auto result = device->createImageView({
            .image = device->createImage({
                .format = VK_FORMAT_D32_SFLOAT,
                .extent = {(uint32_t)sx, (uint32_t)sy, 1},
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .debugName = "Depth Image",
            }),
            .format = VK_FORMAT_D32_SFLOAT,
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .debugName = "Depth Image View",
        });
        return result;
    }

    RenderContext(VkSurfaceKHR surface, int32_t sx, int32_t sy)
        : device(daxa::gpu::Device::create()), queue(device->createCommandQueue({.batchCount = 2})),
          swapchain(device->createSwapchain({
              .surface = surface,
              .width = (uint32_t)sx,
              .height = (uint32_t)sy,
              .presentMode = VK_PRESENT_MODE_FIFO_KHR,
              .additionalUses = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
              .debugName = "Swapchain",
          })),
          pipeline_compiler{this->device->createPipelineCompiler()},
          swapchain_image(swapchain->aquireNextImage()),
          render_image(create_render_image(sx, sy)),
          depth_image(create_depth_image(sx, sy)) {
        for (int i = 0; i < 3; i++) {
            frames.push_back(PerFrameData{
                .present_signal = device->createSignal({}),
                .timeline = device->createTimelineSemaphore({}),
                .timeline_counter = 0,
            });
        }
        pipeline_compiler->addShaderSourceRootPath("DaxaMinecraft/assets/shaders");
    }

    ~RenderContext() {
        queue->waitIdle();
        queue->checkForFinishedSubmits();
        device->waitIdle();
        frames.clear();
    }

    static inline uint32_t resizes = 0;

    auto begin_frame(int32_t sx, int32_t sy) {
        if (sx != swapchain->getSize().width || sy != swapchain->getSize().height) {
            device->waitIdle();
            resizes++;
            printf("resize count: %i\n", resizes);
            swapchain->resize(VkExtent2D{.width = (uint32_t)sx, .height = (uint32_t)sy});
            swapchain_image = swapchain->aquireNextImage();
            render_image = create_render_image(sx, sy);
            depth_image = create_depth_image(sx, sy);
        }
        auto *currentFrame = &frames.front();
        auto cmd_list = queue->getCommandList({});

        cmd_list->queueImageBarrier(daxa::gpu::ImageBarrier{
            .barrier = daxa::gpu::FULL_MEMORY_BARRIER,
            .image = render_image,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        });
        cmd_list->queueImageBarrier(daxa::gpu::ImageBarrier{
            .barrier = daxa::gpu::FULL_MEMORY_BARRIER,
            .image = depth_image,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        });

        return cmd_list;
    }

    void begin_rendering(daxa::gpu::CommandListHandle cmd_list) {
        std::array framebuffer{daxa::gpu::RenderAttachmentInfo{
            .image = render_image,
            .clearValue = {.color = {.float32 = {0.3f, 0.6f, 1.0f, 1.0f}}},
        }};

        daxa::gpu::RenderAttachmentInfo depth_attachment{
            .image = depth_image,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .clearValue = {.depthStencil = {.depth = 1.0f}},
        };
        cmd_list->beginRendering(daxa::gpu::BeginRenderingInfo{
            .colorAttachments = framebuffer,
            .depthAttachment = &depth_attachment,
        });
    }

    void end_rendering(daxa::gpu::CommandListHandle cmd_list) {
        cmd_list->endRendering();
    }

    void blit_to_swapchain(daxa::gpu::CommandListHandle cmd_list) {
        auto render_extent = swapchain_image.getImageViewHandle()->getImageHandle()->getVkExtent3D();
        auto swap_extent = swapchain_image.getImageViewHandle()->getImageHandle()->getVkExtent3D();

        cmd_list->insertImageBarriers(std::array{
            daxa::gpu::ImageBarrier{
                .barrier = {
                    .srcStages = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR | VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                    .srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR,
                    .dstStages = VK_PIPELINE_STAGE_2_BLIT_BIT_KHR,
                    .dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT_KHR,
                },
                .image = render_image,
                .layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .subRange = {VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }},
            },
            daxa::gpu::ImageBarrier{
                .barrier = {
                    .dstStages = VK_PIPELINE_STAGE_2_BLIT_BIT_KHR,
                    .dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT_KHR,
                },
                .image = swapchain_image.getImageViewHandle(),
                .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .subRange = {VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }},
            },
        });
        VkImageBlit blit{
            .srcSubresource = VkImageSubresourceLayers{
                .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .srcOffsets = {
                VkOffset3D{0, 0, 0},
                VkOffset3D{
                    static_cast<int32_t>(render_extent.width),
                    static_cast<int32_t>(render_extent.height),
                    static_cast<int32_t>(render_extent.depth),
                },
            },
            .dstSubresource = VkImageSubresourceLayers{
                .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .dstOffsets = {
                VkOffset3D{0, 0, 0},
                VkOffset3D{
                    static_cast<int32_t>(swap_extent.width),
                    static_cast<int32_t>(swap_extent.height),
                    static_cast<int32_t>(swap_extent.depth),
                },
            },
        };
        cmd_list->insertQueuedBarriers();

        vkCmdBlitImage(
            cmd_list->getVkCommandBuffer(),
            render_image->getImageHandle()->getVkImage(),
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            swapchain_image.getImageViewHandle()->getImageHandle()->getVkImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        cmd_list->insertImageBarriers(std::array{
            daxa::gpu::ImageBarrier{
                .barrier = {
                    .srcStages = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR | VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                    .srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR,
                    .dstStages = VK_PIPELINE_STAGE_2_BLIT_BIT_KHR,
                    .dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT_KHR,
                },
                .image = render_image,
                .layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .subRange = {VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }},
            },
            daxa::gpu::ImageBarrier{
                .barrier = {
                    .dstStages = VK_PIPELINE_STAGE_2_BLIT_BIT_KHR,
                    .dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT_KHR,
                },
                .image = swapchain_image.getImageViewHandle(),
                .layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .subRange = {VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }},
            },
        });
    }

    void end_frame(daxa::gpu::CommandListHandle cmd_list) {
        auto *currentFrame = &frames.front();
        blit_to_swapchain(cmd_list);
        cmd_list->finalize();
        std::array signalTimelines = {
            std::tuple{currentFrame->timeline, ++currentFrame->timeline_counter},
        };
        daxa::gpu::SubmitInfo submitInfo;
        submitInfo.commandLists.push_back(std::move(cmd_list));
        submitInfo.signalOnCompletion = {&currentFrame->present_signal, 1};
        submitInfo.signalTimelines = signalTimelines;
        queue->submit(submitInfo);
        queue->present(std::move(swapchain_image), currentFrame->present_signal);
        swapchain_image = swapchain->aquireNextImage();
        auto frameContext = std::move(frames.back());
        frames.pop_back();
        frames.push_front(std::move(frameContext));
        currentFrame = &frames.front();
        queue->checkForFinishedSubmits();
        queue->nextBatch();
        currentFrame->timeline->wait(currentFrame->timeline_counter);
    }
};

struct Texture {
    daxa::gpu::ImageViewHandle image;

    int size_x, size_y, num_channels;

    Texture(RenderContext &render_ctx, const std::filesystem::path &filepath) {
        image = render_ctx.device->createImageView({
            .image = render_ctx.device->createImage({
                .format = VK_FORMAT_R8G8B8A8_SRGB,
                .extent = {16, 16, 1},
                .mipLevels = 4,
                .arrayLayers = 19,
                .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                .debugName = "Texture Image",
            }),
            .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 4,
                    .baseArrayLayer = 0,
                    .layerCount = 19,
                },
            .defaultSampler = render_ctx.device->createSampler({
                .magFilter = VK_FILTER_NEAREST,
                .minFilter = VK_FILTER_LINEAR,
                .anisotropyEnable = true,
                .maxAnisotropy = 16.0f,
                .maxLod = 3,
                .debugName = "Texture Sampler",
            }),
            .debugName = "Texture Image View",
        });

        auto cmd_list = render_ctx.queue->getCommandList({});
        cmd_list->insertImageBarrier({
            .image = image,
            .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .subRange = {VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 19,
            }},
        });

        std::array<std::filesystem::path, 19> texture_names{
            "brick.png",
            "cactus.png",
            "cobblestone.png",
            "diamond_ore.png",
            "dirt.png",
            "dried_shrub.png",
            "grass-side.png",
            "grass-top.png",
            "gravel.png",
            "leaves.png",
            "log-side.png",
            "log-top.png",
            "planks.png",
            "rose.png",
            "sand.png",
            "sandstone.png",
            "stone.png",
            "tallgrass.png",
            "water.png",
        };

        for (size_t i = 0; i < 19; ++i) {
            stbi_set_flip_vertically_on_load(true);
            auto path = filepath / texture_names[i];
            std::uint8_t *data = stbi_load(path.string().c_str(), &size_x, &size_y, &num_channels, 0);

            cmd_list->copyHostToImage({
                .src = data,
                .dst = image,
                .dstImgSubressource = {VkImageSubresourceLayers{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = static_cast<uint32_t>(i),
                    .layerCount = 1,
                }},
                .size = size_x * size_y * num_channels * sizeof(uint8_t),
            });
        }
        daxa::generateMipLevels(
            cmd_list, image,
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 19,
            },
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        cmd_list->finalize();
        render_ctx.queue->submitBlocking({
            .commandLists = {cmd_list},
        });
        render_ctx.queue->checkForFinishedSubmits();
    }
};
