#pragma once

#include <game/chunk.hpp>
#include <Daxa.hpp>
#include <deque>

#define OGT_VOX_IMPLEMENTATION
#include "ogt_vox.h"

struct RenderContext {
    daxa::DeviceHandle device;
    daxa::PipelineCompilerHandle pipeline_compiler;
    daxa::CommandQueueHandle queue;

    daxa::SwapchainHandle swapchain;
    daxa::SwapchainImage swapchain_image;
    daxa::ImageViewHandle render_color_image, render_depth_image;

    struct PerFrameData {
        daxa::SignalHandle present_signal;
        daxa::TimelineSemaphoreHandle timeline;
        u64 timeline_counter = 0;
    };
    std::deque<PerFrameData> frames;

    auto create_color_image(glm::ivec2 dim) {
        auto result = device->createImageView({
            .image = device->createImage({
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .extent = {static_cast<u32>(dim.x), static_cast<u32>(dim.y), 1},
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                .debugName = "Render Image",
            }),
            .format = VK_FORMAT_R8G8B8A8_UNORM,
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

    auto create_depth_image(glm::ivec2 dim) {
        auto result = device->createImageView({
            .image = device->createImage({
                .format = VK_FORMAT_D32_SFLOAT,
                .extent = {static_cast<u32>(dim.x), static_cast<u32>(dim.y), 1},
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

    RenderContext(VkSurfaceKHR surface, glm::ivec2 dim)
        : device(daxa::Device::create()),
          pipeline_compiler{this->device->createPipelineCompiler()},
          queue(device->createCommandQueue({.batchCount = 2})),
          swapchain(device->createSwapchain({
              .surface = surface,
              .width = static_cast<u32>(dim.x),
              .height = static_cast<u32>(dim.y),
              .presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
              .additionalUses = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
              .debugName = "Swapchain",
          })),
          swapchain_image(swapchain->aquireNextImage()),
          render_color_image(create_color_image(dim)),
          render_depth_image(create_depth_image(dim)) {
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

    auto begin_frame(glm::ivec2 dim) {
        resize(dim);
        // auto *currentFrame = &frames.front();
        auto cmd_list = queue->getCommandList({});
        cmd_list.queueImageBarrier(daxa::ImageBarrier{
            .barrier = daxa::FULL_MEMORY_BARRIER,
            .image = render_color_image,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        cmd_list.queueImageBarrier(daxa::ImageBarrier{
            .barrier = daxa::FULL_MEMORY_BARRIER,
            .image = render_depth_image,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        });
        return cmd_list;
    }

    void begin_rendering(daxa::CommandListHandle cmd_list) {
        std::array framebuffer{daxa::RenderAttachmentInfo{
            .image = swapchain_image.getImageViewHandle(),
            .clearValue = {.color = {.float32 = {1.0f, 0.0f, 1.0f, 1.0f}}},
        }};
        daxa::RenderAttachmentInfo depth_attachment{
            .image = render_depth_image,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .clearValue = {.depthStencil = {.depth = 1.0f}},
        };
        cmd_list.beginRendering(daxa::BeginRenderingInfo{
            .colorAttachments = framebuffer,
            .depthAttachment = &depth_attachment,
        });
    }

    void end_rendering(daxa::CommandListHandle cmd_list) {
        cmd_list.endRendering();
    }

    void end_frame(daxa::CommandListHandle cmd_list) {
        auto *current_frame = &frames.front();
        cmd_list.finalize();
        // std::array signalTimelines = {
        //     std::tuple{current_frame->timeline, ++current_frame->timeline_counter},
        // };
        daxa::SubmitInfo submitInfo;
        submitInfo.commandLists.push_back(std::move(cmd_list));
        submitInfo.signalOnCompletion = {&current_frame->present_signal, 1};
        // submitInfo.signalTimelines = signalTimelines;
        queue->submit(submitInfo);
        queue->present(std::move(swapchain_image), current_frame->present_signal);
        swapchain_image = swapchain->aquireNextImage();
        auto frameContext = std::move(frames.back());
        frames.pop_back();
        frames.push_front(std::move(frameContext));
        current_frame = &frames.front();
        queue->checkForFinishedSubmits();
        queue->nextBatch();
        // current_frame->timeline->wait(current_frame->timeline_counter);
    }

    void resize(glm::ivec2 dim) {
        if (dim.x != static_cast<i32>(swapchain->getSize().width) || dim.y != static_cast<i32>(swapchain->getSize().height)) {
            device->waitIdle();
            swapchain->resize(VkExtent2D{.width = static_cast<u32>(dim.x), .height = static_cast<u32>(dim.y)});
            swapchain_image = swapchain->aquireNextImage();
            render_color_image = create_color_image(dim);
            render_depth_image = create_depth_image(dim);
        }
    }

    void blit_to_swapchain(daxa::CommandListHandle cmd_list) {

        cmd_list.queueImageBarrier({
            .image = swapchain_image.getImageViewHandle(),
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        });
        cmd_list.queueImageBarrier({
            .image = render_color_image,
            .layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
            .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        });

        // cmd_list.singleCopyImageToImage({
        //     .src = render_color_image->getImageHandle(),
        //     .dst = swapchain_image.getImageViewHandle()->getImageHandle(),
        // });

        auto render_extent = swapchain_image.getImageViewHandle()->getImageHandle()->getVkExtent3D();
        auto swap_extent = swapchain_image.getImageViewHandle()->getImageHandle()->getVkExtent3D();
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
        cmd_list.insertQueuedBarriers();
        vkCmdBlitImage(
            cmd_list.getVkCommandBuffer(),
            render_color_image->getImageHandle()->getVkImage(),
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            swapchain_image.getImageViewHandle()->getImageHandle()->getVkImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        cmd_list.queueImageBarrier({
            .image = swapchain_image.getImageViewHandle(),
            .layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        });
        cmd_list.queueImageBarrier({
            .image = render_color_image,
            .layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
    }
};

struct RenderableChunk {
    daxa::ImageViewHandle chunkgen_image_a;
    bool initialized = false;
};

struct World {
    static constexpr glm::ivec3 DIM = glm::ivec3{1024, 256, 1024} / Chunk::DIM;

    template <typename T>
    using ChunkArray = std::array<std::array<std::array<T, DIM.x>, DIM.y>, DIM.z>;
    ChunkArray<std::unique_ptr<RenderableChunk>> chunks{};
    std::array<glm::uvec3, DIM.x * DIM.y * DIM.z> chunk_indices{};

    daxa::ImageViewHandle atlas_texture_array;
    daxa::SamplerHandle atlas_texture_sampler;

    daxa::BufferHandle compute_pipeline_globals;
    daxa::BufferHandle model_buffer;

    daxa::PipelineHandle raymarch_compute_pipeline;
    daxa::PipelineHandle pickblock_compute_pipeline;
    daxa::PipelineHandle blockedit_compute_pipeline;
    daxa::PipelineHandle modelload_compute_pipeline;
    daxa::PipelineHandle subchunk_x2x4_pipeline;
    daxa::PipelineHandle subchunk_x8p_pipeline;

    std::array<daxa::PipelineHandle, 1> chunkgen_compute_pipeline_passes;

    struct ChunkBlockPresence {
        u32 x2[1024];
        u32 x4[256];
        u32 x8[64];
        u32 x16[16];
        u32 x32[4];
    };

    struct ComputeGlobals {
        glm::mat4 viewproj_mat;
        glm::vec4 pos;
        glm::vec4 single_ray_pos;
        glm::vec4 single_ray_nrm;
        glm::vec4 pick_pos;
        glm::ivec2 frame_dim;
        float time, fov;

        u32 texture_index;
        u32 sampler_index;
        u32 model_load_index;
        u32 single_ray_steps;
        ChunkArray<u32> chunk_ids;
        // ChunkArray<ChunkBlockPresence> chunk_block_presence;
    };

    static inline constexpr size_t size = sizeof(ComputeGlobals);

    struct ChunkgenPush {
        glm::vec4 pos;
        u32 globals_sb;
        u32 output_image_i;
    };

    struct ChunkRaymarchPush {
        u32 globals_sb;
        u32 output_image_i;
    };

    struct ModelLoadBuffer {
        glm::vec4 pos, dim;
        std::array<u32, 128 * 128 * 128> data;
    };
    std::unique_ptr<ModelLoadBuffer> model_load_buffer = std::make_unique<ModelLoadBuffer>();

    RenderContext &render_ctx;

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point start;

    bool initialized = false;
    int chunk_updates_per_frame = 4;

    bool should_break = false;
    bool should_place = false;

    glm::vec3 single_ray_pos{0}, single_ray_nrm{1};
    i32 single_ray_steps = 10;

    glm::vec3 player_pos{0};
    glm::ivec3 player_rough_chunk_i{0};

    World(RenderContext &render_ctx) : render_ctx(render_ctx) {
        load_textures("DaxaMinecraft/assets/textures");
        load_shaders();

        for (size_t zi = 0; zi < DIM.z; ++zi)
            for (size_t yi = 0; yi < DIM.y; ++yi)
                for (size_t xi = 0; xi < DIM.x; ++xi)
                    chunk_indices[xi + yi * DIM.x + zi * DIM.x * DIM.y] = glm::uvec3(xi, yi, zi);

        start = Clock::now();
        constexpr auto IMG_DIM = glm::uvec3(Chunk::DIM);

        for (auto &chunk_layer : chunks) {
            for (auto &chunk_strip : chunk_layer) {
                for (auto &chunk : chunk_strip) {
                    chunk = std::make_unique<RenderableChunk>();
                    chunk->chunkgen_image_a = render_ctx.device->createImageView({
                        .image = render_ctx.device->createImage({
                            .imageType = VK_IMAGE_TYPE_3D,
                            .format = VK_FORMAT_R16_UINT,
                            .extent = {IMG_DIM.x, IMG_DIM.y, IMG_DIM.z},
                            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                            .debugName = "Chunkgen Image",
                        }),
                        .viewType = VK_IMAGE_VIEW_TYPE_3D,
                        .format = VK_FORMAT_R16_UINT,
                        .subresourceRange =
                            {
                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                .baseMipLevel = 0,
                                .levelCount = 1,
                                .baseArrayLayer = 0,
                                .layerCount = 1,
                            },
                        .debugName = "Chunkgen Image View",
                    });
                    // chunk->chunkgen_image_b = render_ctx.device->createImageView({
                    //     .image = render_ctx.device->createImage({
                    //         .imageType = VK_IMAGE_TYPE_3D,
                    //         .format = VK_FORMAT_R32_UINT,
                    //         .extent = {IMG_DIM.x, IMG_DIM.y, IMG_DIM.z},
                    //         .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    //         .debugName = "Chunkgen Image",
                    //     }),
                    //     .viewType = VK_IMAGE_VIEW_TYPE_3D,
                    //     .format = VK_FORMAT_R32_UINT,
                    //     .subresourceRange =
                    //         {
                    //             .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    //             .baseMipLevel = 0,
                    //             .levelCount = 1,
                    //             .baseArrayLayer = 0,
                    //             .layerCount = 1,
                    //         },
                    //     .debugName = "Chunkgen Image View",
                    // });
                }
            }
        }

        auto cmd_list = render_ctx.queue->getCommandList({});

        for (auto &chunk_layer : chunks) {
            for (auto &chunk_strip : chunk_layer) {
                for (auto &chunk : chunk_strip) {
                    cmd_list.queueImageBarrier(daxa::ImageBarrier{
                        .barrier = daxa::FULL_MEMORY_BARRIER,
                        .image = chunk->chunkgen_image_a,
                        .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
                        .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
                    });
                    // cmd_list.queueImageBarrier(daxa::ImageBarrier{
                    //     .barrier = daxa::FULL_MEMORY_BARRIER,
                    //     .image = chunk->chunkgen_image_b,
                    //     .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
                    //     .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
                    // });
                }
            }
        }

        cmd_list.finalize();
        daxa::SubmitInfo submitInfo;
        submitInfo.commandLists.push_back(std::move(cmd_list));
        render_ctx.queue->submit(submitInfo);

        compute_pipeline_globals = render_ctx.device->createBuffer({
            .size = sizeof(ComputeGlobals) + sizeof(ChunkArray<ChunkBlockPresence>),
            //.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            //.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
            .memoryType = daxa::MemoryType::GPU_ONLY,
        });
        model_buffer = render_ctx.device->createBuffer({
            .size = sizeof(ModelLoadBuffer),
            .memoryType = daxa::MemoryType::GPU_ONLY,
        });

        std::ifstream model_voxfile("DaxaMinecraft/assets/models/teapot.vox", std::ios::binary);
        std::vector<u8> buffer(std::istreambuf_iterator<char>(model_voxfile), {});
        const ogt_vox_scene *scene = ogt_vox_read_scene(buffer.data(), buffer.size());
        auto &model = *scene->models[0];
        model_load_buffer->pos = glm::vec4(-32, 0, -32, 0);
        model_load_buffer->dim = glm::vec4(std::min<f32>(model.size_x, 128), std::min<f32>(model.size_z, 128), std::min<f32>(model.size_y, 128), 0);
        for (size_t zi = 0; zi < Chunk::DIM.z * 2; ++zi) {
            for (size_t yi = 0; yi < Chunk::DIM.y * 2; ++yi) {
                for (size_t xi = 0; xi < Chunk::DIM.x * 2; ++xi) {
                    if (xi < model.size_x && zi < model.size_y && yi < model.size_z) {
                        auto v = model.voxel_data[xi + zi * model.size_x + (model.size_z - yi - 1) * model.size_x * model.size_y];
                        if (v != 0)
                            model_load_buffer->data[xi + yi * 128 + zi * 128 * 128] = v % 24 + 2;
                        else
                            model_load_buffer->data[xi + yi * 128 + zi * 128 * 128] = 1;
                    } else {
                        model_load_buffer->data[xi + yi * 128 + zi * 128 * 128] = 1;
                    }
                }
            }
        }

        ogt_vox_destroy_scene(scene);
    }

    ~World() {
        render_ctx.queue->waitIdle();
        render_ctx.queue->checkForFinishedSubmits();
    }

    glm::ivec3 mvchunk_size;

    void process_mvchunk(char *&buf_ptr, int i = 0) {
        if (i > 100) {
            buf_ptr += 10000;
            return;
        }

        std::string_view chunkid = std::string_view(buf_ptr, buf_ptr + 4);
        buf_ptr += 4;
        int data_size = *reinterpret_cast<int *>(buf_ptr);
        buf_ptr += 4;
        int subdata_size = *reinterpret_cast<int *>(buf_ptr);
        buf_ptr += 4;
        char *data = buf_ptr;
        buf_ptr += data_size;
        // char *subdata = buf_ptr;
        char *sub_end = buf_ptr + subdata_size;

        for (int j = 0; j < i; ++j)
            std::cout << "    ";

        if (chunkid.compare("SIZE") == 0) {
            mvchunk_size.x = *reinterpret_cast<int *>(data + 0x0);
            mvchunk_size.y = *reinterpret_cast<int *>(data + 0x4);
            mvchunk_size.z = *reinterpret_cast<int *>(data + 0x8);
            std::cout << mvchunk_size.x << ", " << mvchunk_size.y << ", " << mvchunk_size.z << std::endl;
        } else {
            std::cout << chunkid << ": " << data_size << ", " << subdata_size << std::endl;
        }

        if (subdata_size > 0) {
            while (buf_ptr < sub_end)
                process_mvchunk(buf_ptr, i + 1);
        }
    }

    bool try_recreate_pipeline(daxa::PipelineHandle &pipe) {
        if (render_ctx.pipeline_compiler->checkIfSourcesChanged(pipe)) {
            auto result = render_ctx.pipeline_compiler->recreatePipeline(pipe);
            std::cout << result << std::endl;
            if (result)
                pipe = result.value();
            return true;
        }
        return false;
    }

    void reload_shaders() {
        try_recreate_pipeline(raymarch_compute_pipeline);
        try_recreate_pipeline(blockedit_compute_pipeline);
        try_recreate_pipeline(pickblock_compute_pipeline);
        try_recreate_pipeline(modelload_compute_pipeline);
        bool should_reinit0 = false;
        for (auto &pipe : chunkgen_compute_pipeline_passes) {
            if (try_recreate_pipeline(pipe))
                should_reinit0 = true;
        }
        bool should_reinit1 =
            try_recreate_pipeline(subchunk_x2x4_pipeline) ||
            try_recreate_pipeline(subchunk_x8p_pipeline);
        if (should_reinit0 || should_reinit1) {
            initialized = false;
            for (size_t zi = 0; zi < DIM.z; ++zi)
                for (size_t yi = 0; yi < DIM.y; ++yi)
                    for (size_t xi = 0; xi < DIM.x; ++xi)
                        chunks[zi][yi][xi]->initialized = false;
        }
    }

    void update(float) {
        reload_shaders();
    }

    void do_break(daxa::CommandListHandle cmd_list, u32 compute_globals_i, const ComputeGlobals &compute_globals) {
        cmd_list.bindPipeline(blockedit_compute_pipeline);
        cmd_list.bindAll();
        for (size_t zi = 0; zi < DIM.z; ++zi) {
            for (size_t yi = 0; yi < DIM.y; ++yi) {
                for (size_t xi = 0; xi < DIM.x; ++xi) {
                    cmd_list.pushConstant(
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        ChunkgenPush{
                            .pos = {1.0f * xi * Chunk::DIM.x, 1.0f * yi * Chunk::DIM.y, 1.0f * zi * Chunk::DIM.z, 0},
                            .globals_sb = compute_globals_i,
                            .output_image_i = compute_globals.chunk_ids[zi][yi][xi],
                        });
                    cmd_list.dispatch(8, 8, 8);
                }
            }
        }

        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
    }

    void do_place(daxa::CommandListHandle) {
    }

    void draw(const glm::mat4 &vp_mat, const Player3D &player, daxa::CommandListHandle cmd_list, daxa::ImageViewHandle &render_image) {
        player_pos = player.pos;
        {
            auto prev_rough_chunk_i = player_rough_chunk_i;
            player_rough_chunk_i = glm::ivec3(player.pos) / Chunk::DIM;
            if (player_rough_chunk_i != prev_rough_chunk_i) {
                std::sort(chunk_indices.begin(), chunk_indices.end(), [&](glm::uvec3 a, glm::uvec3 b) {
                    glm::vec3 wa = glm::vec3(glm::ivec3(a)) * glm::vec3(Chunk::DIM);
                    glm::vec3 wb = glm::vec3(glm::ivec3(b)) * glm::vec3(Chunk::DIM);
                    return glm::dot(player_pos - wa, player_pos - wa) < glm::dot(player_pos - wb, player_pos - wb);
                });
            }
        }

        auto extent = render_image->getImageHandle()->getVkExtent3D();

        auto elapsed = std::chrono::duration<float>(Clock::now() - start).count();

        auto compute_globals = ComputeGlobals{
            .viewproj_mat = vp_mat,
            .pos = glm::vec4(player.pos, 0),
            .single_ray_pos = glm::vec4(single_ray_pos, 0),
            .single_ray_nrm = glm::vec4(single_ray_nrm, 0),
            .frame_dim = {extent.width, extent.height},
            .time = elapsed,
            .fov = tanf(player.camera.fov * std::numbers::pi_v<f32> / 360.0f),
            .texture_index = atlas_texture_array->getDescriptorIndex(),
            .sampler_index = atlas_texture_sampler->getDescriptorIndex(),
            .single_ray_steps = static_cast<u32>(single_ray_steps),
        };

        for (size_t zi = 0; zi < DIM.z; ++zi) {
            for (size_t yi = 0; yi < DIM.y; ++yi) {
                for (size_t xi = 0; xi < DIM.x; ++xi) {
                    compute_globals.chunk_ids[zi][yi][xi] = chunks[zi][yi][xi]->chunkgen_image_a->getDescriptorIndex();
                }
            }
        }

        auto compute_modelload_i = model_buffer.getDescriptorIndex();
        compute_globals.model_load_index = compute_modelload_i;

        cmd_list.singleCopyHostToBuffer({
            .src = reinterpret_cast<u8 *>(model_load_buffer.get()),
            .dst = model_buffer,
            .region = {.size = sizeof(ModelLoadBuffer)}, // - sizeof(decltype(compute_globals.chunk_block_presence))
        });
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);

        auto compute_globals_i = compute_pipeline_globals.getDescriptorIndex();

        cmd_list.singleCopyHostToBuffer({
            .src = reinterpret_cast<u8 *>(&compute_globals),
            .dst = compute_pipeline_globals,
            .region = {.size = sizeof(decltype(compute_globals))}, // - sizeof(decltype(compute_globals.chunk_block_presence))
        });
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);

        if (!initialized)
            for (int i = 0; i < chunk_updates_per_frame; ++i) {
                auto chunk_iter = std::find_if(chunk_indices.begin(), chunk_indices.end(), [&](glm::uvec3 chunk_i) {
                    return !chunks[chunk_i.z][chunk_i.y][chunk_i.x]->initialized;
                });
                if (chunk_iter == chunk_indices.end()) {
                    initialized = true;
                    break;
                }

                glm::uvec3 chunk_i = *chunk_iter;
                u32 xi = chunk_i.x;
                u32 yi = chunk_i.y;
                u32 zi = chunk_i.z;
                chunks[zi][yi][xi]->initialized = true;

                for (auto &pipe : chunkgen_compute_pipeline_passes) {
                    cmd_list.bindPipeline(pipe);
                    cmd_list.bindAll();
                    cmd_list.pushConstant(
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        ChunkgenPush{
                            .pos = {1.0f * xi * Chunk::DIM.x, 1.0f * yi * Chunk::DIM.y, 1.0f * zi * Chunk::DIM.z, 0},
                            .globals_sb = compute_globals_i,
                            .output_image_i = compute_globals.chunk_ids[zi][yi][xi],
                        });
                    cmd_list.dispatch(8, 8, 8);

                    cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
                }

                cmd_list.bindPipeline(modelload_compute_pipeline);
                cmd_list.bindAll();
                cmd_list.pushConstant(
                    VK_SHADER_STAGE_COMPUTE_BIT,
                    ChunkgenPush{
                        .pos = {1.0f * xi * Chunk::DIM.x, 1.0f * yi * Chunk::DIM.y, 1.0f * zi * Chunk::DIM.z, 0},
                        .globals_sb = compute_globals_i,
                        .output_image_i = compute_globals.chunk_ids[zi][yi][xi],
                    });
                cmd_list.dispatch(8, 8, 8);
                cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);

                struct SubChunkPush {
                    u32 chunk_i[4];
                    u32 globalsID;
                };

                cmd_list.bindPipeline(subchunk_x2x4_pipeline);
                cmd_list.bindAll();
                cmd_list.pushConstant(
                    VK_SHADER_STAGE_COMPUTE_BIT,
                    SubChunkPush{
                        .chunk_i = {xi, yi, zi, 0},
                        .globalsID = compute_globals_i,
                    });
                cmd_list.dispatch(1, 64, 1);
                cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);

                cmd_list.bindPipeline(subchunk_x8p_pipeline);
                cmd_list.bindAll();
                cmd_list.pushConstant(
                    VK_SHADER_STAGE_COMPUTE_BIT,
                    SubChunkPush{
                        .chunk_i = {xi, yi, zi, 0},
                        .globalsID = compute_globals_i,
                    });
                cmd_list.dispatch(1, 1, 1);
            }

        if (should_place) {
            should_place = false;
            do_place(cmd_list);
        }
        if (should_break) {
            should_break = false;
            do_break(cmd_list, compute_globals_i, compute_globals);
        }

        cmd_list.bindPipeline(pickblock_compute_pipeline);
        cmd_list.bindAll();
        cmd_list.pushConstant(
            VK_SHADER_STAGE_COMPUTE_BIT,
            ChunkRaymarchPush{
                .globals_sb = compute_globals_i,
            });
        cmd_list.dispatch(1, 1, 1);

        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);

        cmd_list.bindPipeline(raymarch_compute_pipeline);
        cmd_list.bindAll();
        cmd_list.pushConstant(
            VK_SHADER_STAGE_COMPUTE_BIT,
            ChunkRaymarchPush{
                .globals_sb = compute_globals_i,
                .output_image_i = render_image->getDescriptorIndex(),
            });
        cmd_list.dispatch((extent.width + 7) / 8, (extent.height + 7) / 8);
    }

    void create_pipeline(daxa::PipelineHandle &pipe, const std::filesystem::path &path, daxa::ShaderLang lang = daxa::ShaderLang::GLSL, const char *entry = "main") {
        auto result = render_ctx.pipeline_compiler->createComputePipeline({
            .shaderCI = {
                .pathToSource = std::filesystem::path("DaxaMinecraft/assets/shaders") / path,
                .shaderLang = lang,
                .entryPoint = entry,
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            },
            .overwriteSets = {daxa::BIND_ALL_SET_DESCRIPTION},
        });
        pipe = result.value();
    }

    void load_shaders() {
        create_pipeline(raymarch_compute_pipeline, "drawing/raymarch.hlsl", daxa::ShaderLang::HLSL);
        create_pipeline(pickblock_compute_pipeline, "utils/pickblock.comp");
        create_pipeline(blockedit_compute_pipeline, "utils/blockedit.comp");
        create_pipeline(modelload_compute_pipeline, "chunkgen/model_load.hlsl", daxa::ShaderLang::HLSL);
        create_pipeline(subchunk_x2x4_pipeline, "chunkgen/subchunk_x2x4.hlsl", daxa::ShaderLang::HLSL, "Main");
        create_pipeline(subchunk_x8p_pipeline, "chunkgen/subchunk_x8p.hlsl", daxa::ShaderLang::HLSL, "Main");

        std::array<std::filesystem::path, 1> chunkgen_pass_paths = {
            "chunkgen/world/pass0.hlsl",
        };
        for (size_t i = 0; i < chunkgen_pass_paths.size(); ++i)
            create_pipeline(chunkgen_compute_pipeline_passes[i], chunkgen_pass_paths[i], daxa::ShaderLang::HLSL);
    }

    void load_textures(const std::filesystem::path &filepath) {
        std::array<std::filesystem::path, 32> texture_names{
            "debug.png",
            "air.png",
            "bedrock.png",
            "brick.png",
            "cactus.png",
            "cobblestone.png",
            "compressed_stone.png",
            "diamond_ore.png",
            "dirt.png",
            "dried_shrub.png",
            "grass-side.png",
            "grass-top.png",
            "gravel.png",
            "lava_0.png",
            "lava_1.png",
            "lava_2.png",
            "lava_3.png",
            "lava_4.png",
            "lava_5.png",
            "lava_6.png",
            "lava_7.png",
            "leaves.png",
            "log-side.png",
            "log-top.png",
            "molten_rock.png",
            "planks.png",
            "rose.png",
            "sand.png",
            "sandstone.png",
            "stone.png",
            "tallgrass.png",
            "water.png",
        };

        atlas_texture_sampler = render_ctx.device->createSampler({
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_LINEAR,
            .anisotropyEnable = true,
            .maxAnisotropy = 16.0f,
            .maxLod = 3,
            .debugName = "Texture Sampler",
        });

        atlas_texture_array = render_ctx.device->createImageView({
            .image = render_ctx.device->createImage({
                .format = VK_FORMAT_R8G8B8A8_SRGB,
                .extent = {16, 16, 1},
                .mipLevels = 4,
                .arrayLayers = static_cast<u32>(texture_names.size()),
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
                    .layerCount = static_cast<u32>(texture_names.size()),
                },
            .defaultSampler = atlas_texture_sampler,
            .debugName = "Texture Image View",
        });

        auto cmd_list = render_ctx.queue->getCommandList({});
        cmd_list.queueImageBarrier({
            .image = atlas_texture_array,
            .layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .subRange = {VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = static_cast<u32>(texture_names.size()),
            }},
        });

        for (size_t i = 0; i < texture_names.size(); ++i) {
            stbi_set_flip_vertically_on_load(true);
            auto path = filepath / texture_names[i];
            int size_x, size_y, num_channels;
            std::uint8_t *data = stbi_load(path.string().c_str(), &size_x, &size_y, &num_channels, 4);
            if (!data) {
                std::cout << "could not find file: \"" << path << "\"" << std::endl;
                continue;
            }
            cmd_list.singleCopyHostToImage({
                .src = data,
                .dst = atlas_texture_array->getImageHandle(),
                .region = {
                    .subRessource = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .mipLevel = 0,
                        .baseArrayLayer = static_cast<uint32_t>(i),
                        .layerCount = 1,
                    },
                    .imageExtent = {static_cast<u32>(16), static_cast<u32>(16), 1},
                },
            });
        }
        daxa::generateMipLevels(
            cmd_list, atlas_texture_array,
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = static_cast<u32>(texture_names.size()),
            },
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        cmd_list.finalize();
        render_ctx.queue->submitBlocking({
            .commandLists = {cmd_list},
        });
        render_ctx.queue->checkForFinishedSubmits();
    }
};
