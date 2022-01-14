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
    daxa::gpu::QueueHandle  queue;

    daxa::gpu::SwapchainHandle swapchain;
    daxa::gpu::SwapchainImage  swapchain_image;
    daxa::gpu::ImageHandle     depth_image;

    struct PerFrameData {
        daxa::gpu::SignalHandle            present_signal;
        daxa::gpu::TimelineSemaphoreHandle timeline;
        u64                                timeline_counter = 0;
    };
    std::deque<PerFrameData> frames;

    RenderContext(VkSurfaceKHR surface, int32_t sx, int32_t sy)
        : device(daxa::gpu::Device::create()), queue(device->createQueue({})),
          swapchain(device->createSwapchain({
              .surface     = surface,
              .width       = (uint32_t)sx,
              .height      = (uint32_t)sy,
              .presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
          })),
          swapchain_image(swapchain->aquireNextImage()),
          depth_image(device->createImage2d({
              .width       = (uint32_t)sx,
              .height      = (uint32_t)sy,
              .format      = VK_FORMAT_D32_SFLOAT,
              .imageUsage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              .imageAspekt = VK_IMAGE_ASPECT_DEPTH_BIT,
          })) {
        for (int i = 0; i < 3; i++) {
            frames.push_back(PerFrameData{
                .present_signal   = device->createSignal({}),
                .timeline         = device->createTimelineSemaphore({}),
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
            depth_image     = device->createImage2d({
                    .width       = (uint32_t)sx,
                    .height      = (uint32_t)sy,
                    .format      = VK_FORMAT_D32_SFLOAT,
                    .imageUsage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    .imageAspekt = VK_IMAGE_ASPECT_DEPTH_BIT,
            });
        }
        auto * currentFrame = &frames.front();
        auto   cmd_list     = device->getCommandList();

        std::array imgBarrier0 = {daxa::gpu::ImageBarrier{
            .waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
            .image         = swapchain_image.getImageHandle(),
            .layoutBefore  = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        }};
        std::array memBarrier0 = {daxa::gpu::MemoryBarrier{
            .awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
            .waitingStages = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,
        }};
        cmd_list->insertBarriers(memBarrier0, imgBarrier0);

        return cmd_list;
    }

    void begin_rendering(daxa::gpu::CommandListHandle cmd_list) {
        std::array framebuffer{daxa::gpu::RenderAttachmentInfo{
            .image      = swapchain_image.getImageHandle(),
            .clearValue = {.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}},
        }};

        daxa::gpu::RenderAttachmentInfo depth_attachment{
            .image      = depth_image,
            .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .clearValue = {.depthStencil = {.depth = 1.0f}},
        };
        cmd_list->beginRendering(daxa::gpu::BeginRenderingInfo{
            .colorAttachments = framebuffer,
            .depthAttachment  = &depth_attachment,
        });
    }

    void end_rendering(daxa::gpu::CommandListHandle cmd_list) {
        cmd_list->endRendering(); //
    }

    void end_frame(daxa::gpu::CommandListHandle cmd_list) {
        auto * currentFrame = &frames.front();

        std::array imgBarrier1 = {daxa::gpu::ImageBarrier{
            .image        = swapchain_image.getImageHandle(),
            .layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .layoutAfter  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        }};
        cmd_list->insertBarriers({}, imgBarrier1);
        cmd_list->finalize();
        std::array signalTimelines = {
            std::tuple{currentFrame->timeline, ++currentFrame->timeline_counter},
        };
        daxa::gpu::SubmitInfo submitInfo;
        submitInfo.commandLists.push_back(std::move(cmd_list));
        submitInfo.signalOnCompletion = {&currentFrame->present_signal, 1};
        submitInfo.signalTimelines    = signalTimelines;
        queue->submit(submitInfo);
        queue->present(std::move(swapchain_image), currentFrame->present_signal);
        swapchain_image   = swapchain->aquireNextImage();
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
    daxa::gpu::ImageHandle image;

    int size_x, size_y, num_channels;

    Texture(RenderContext & render_ctx, const char * const filepath) {
        stbi_set_flip_vertically_on_load(true);
        std::uint8_t * data = stbi_load(filepath, &size_x, &size_y, &num_channels, 0);
        VkFormat       data_format;
        switch (num_channels) {
        case 1: data_format = VK_FORMAT_R8_SRGB; break;
        case 3: data_format = VK_FORMAT_R8G8B8_SRGB; break;
        case 4:
        default: data_format = VK_FORMAT_R8G8B8A8_SRGB; break;
        }

        image = render_ctx.device->createImage2d({
            .width       = (uint32_t)size_x,
            .height      = (uint32_t)size_y,
            .format      = data_format,
            .imageUsage  = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .imageAspekt = VK_IMAGE_ASPECT_COLOR_BIT,
            .sampler     = render_ctx.device->createSampler({
                    .magFilter = VK_FILTER_NEAREST,
            }),
        });

        auto cmd_list = render_ctx.device->getCommandList();
        cmd_list->copyHostToImageSynced({
            .src            = data,
            .dst            = image,
            .size           = size_x * size_y * num_channels * sizeof(uint8_t),
            .dstFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });
        cmd_list->finalize();
        render_ctx.queue->submitBlocking({
            .commandLists = {cmd_list},
        });
        render_ctx.queue->checkForFinishedSubmits();
    }
};
