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
    daxa::gpu::CommandQueueHandle queue;

    daxa::gpu::SwapchainHandle swapchain;
    daxa::gpu::SwapchainImage swapchain_image;
    daxa::gpu::ImageViewHandle depth_image;

    struct PerFrameData {
        daxa::gpu::SignalHandle present_signal;
        daxa::gpu::TimelineSemaphoreHandle timeline;
        u64 timeline_counter = 0;
    };
    std::deque<PerFrameData> frames;

    RenderContext(VkSurfaceKHR surface, int32_t sx, int32_t sy)
        : device(daxa::gpu::Device::create()), queue(device->createCommandQueue({.batchCount = 2})),
          swapchain(device->createSwapchain({
              .surface = surface,
              .width = (uint32_t)sx,
              .height = (uint32_t)sy,
              .presentMode = VK_PRESENT_MODE_FIFO_KHR,
              .debugName = "Swapchain",
          })),
          swapchain_image(swapchain->aquireNextImage()),
          depth_image(device->createImageView({
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
          })) {
        for (int i = 0; i < 3; i++) {
            frames.push_back(PerFrameData{
                .present_signal = device->createSignal({}),
                .timeline = device->createTimelineSemaphore({}),
                .timeline_counter = 0,
            });
        }
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
            depth_image = device->createImageView({
                .image = device->createImage({
                    .format = VK_FORMAT_D32_SFLOAT,
                    .extent = {(uint32_t)sx, (uint32_t)sy, 1},
                    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    .debugName = "Depth Image (resize)",
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
                .debugName = "Depth Image View (resize)",
            });
        }
        auto *currentFrame = &frames.front();
        auto cmd_list = queue->getCommandList({});

        std::array imgBarrier0 = {daxa::gpu::ImageBarrier{
            .barrier = daxa::gpu::FULL_MEMORY_BARRIER,
            .image = swapchain_image.getImageViewHandle(),
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        }};
        std::array memBarrier0 = {daxa::gpu::MemoryBarrier{
            .srcAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
            .dstStages = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,
        }};
        cmd_list->insertBarriers(memBarrier0, imgBarrier0);

        return cmd_list;
    }

    void begin_rendering(daxa::gpu::CommandListHandle cmd_list) {
        std::array framebuffer{daxa::gpu::RenderAttachmentInfo{
            .image = swapchain_image.getImageViewHandle(),
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
        cmd_list->endRendering(); //
    }

    void end_frame(daxa::gpu::CommandListHandle cmd_list) {
        auto *currentFrame = &frames.front();

        std::array imgBarrier1 = {daxa::gpu::ImageBarrier{
            .image = swapchain_image.getImageViewHandle(),
            .layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        }};
        cmd_list->insertBarriers({}, imgBarrier1);
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
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
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
                .size = size_x * size_y * num_channels * sizeof(uint8_t),
                .dstImgSubressource = {VkImageSubresourceLayers{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseArrayLayer = static_cast<uint32_t>(i),
                    .layerCount = 1,
                    .mipLevel = 0,
                }},
            });
        }
        daxa::generateMipLevels(
            cmd_list, image,
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseArrayLayer = 0,
                .layerCount = 19,
                .mipLevel = 0,
            },
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        cmd_list->finalize();
        render_ctx.queue->submitBlocking({
            .commandLists = {cmd_list},
        });
        render_ctx.queue->checkForFinishedSubmits();
    }
};
