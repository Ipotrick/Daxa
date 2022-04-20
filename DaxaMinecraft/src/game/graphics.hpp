#pragma once

#include <game/chunk.hpp>
#include <game/render_context.hpp>
#include <deque>

#define OGT_VOX_IMPLEMENTATION
#include <deps/ogt_vox.h>

struct RenderableChunk {
    daxa::ImageViewHandle chunkgen_image_a;
    bool initialized = false;
};

struct World {
    static constexpr glm::ivec3 DIM = glm::ivec3{1024, 512, 1024} / Chunk::DIM;
    static constexpr glm::ivec3 CHUNK_REPEAT = glm::ivec3{1, 1, 1};

    template <typename T>
    using ChunkArray = std::array<std::array<std::array<T, DIM.x>, DIM.y>, DIM.z>;
    template <typename T>
    using ChunkIndexArray = std::array<std::array<std::array<T, DIM.x * CHUNK_REPEAT.x>, DIM.y * CHUNK_REPEAT.y>, DIM.z * CHUNK_REPEAT.z>;
    ChunkArray<std::unique_ptr<RenderableChunk>> chunks{};
    std::unique_ptr<RenderableChunk> empty_chunk{};
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
        u32 empty_chunk_index;
        u32 model_load_index;
        u32 single_ray_steps;
        ChunkIndexArray<u32> chunk_ids;

        // Too big to keep on the stack! Let's just alloc it on the GPU only.
        // ChunkArray<ChunkBlockPresence> chunk_block_presence;
    };

    static inline constexpr size_t size = sizeof(ComputeGlobals);

    struct ChunkgenPush {
        glm::vec4 pos;
        u32 globals_sb;
        u32 output_image_i;
    };
    struct BlockeditPush {
        glm::vec4 pos;
        u32 globals_sb;
        u32 output_image_i;
        u32 set_id;
    };

    struct SubChunkPush {
        u32 chunk_i[4];
        u32 globalsID;
        u32 mode;
    };

    struct ChunkRaymarchPush {
        u32 globals_sb;
        u32 output_image_i;
    };

    struct ModelLoadBuffer {
        glm::vec4 pos, dim;
        std::array<u32, 128 * 128 * 128> data;
    };
    // Again too big for the stack, but we need to populate it
    // here on the CPU side, so we'll heap alloc it
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

        empty_chunk = create_chunk();
        for (auto &chunk_layer : chunks)
            for (auto &chunk_strip : chunk_layer)
                for (auto &chunk : chunk_strip)
                    chunk = create_chunk();

        auto cmd_list = render_ctx.queue->getCommandList({});

        cmd_list.queueImageBarrier(daxa::ImageBarrier{
            .barrier = daxa::FULL_MEMORY_BARRIER,
            .image = empty_chunk->chunkgen_image_a,
            .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
            .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
        });
        for (auto &chunk_layer : chunks) {
            for (auto &chunk_strip : chunk_layer) {
                for (auto &chunk : chunk_strip) {
                    cmd_list.queueImageBarrier(daxa::ImageBarrier{
                        .barrier = daxa::FULL_MEMORY_BARRIER,
                        .image = chunk->chunkgen_image_a,
                        .layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
                        .layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
                    });
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
        const ogt_vox_scene *scene = ogt_vox_read_scene(buffer.data(), static_cast<u32>(buffer.size()));
        auto &model = *scene->models[0];
        model_load_buffer->pos = glm::vec4(-32, 0, -32, 0);
        model_load_buffer->dim = glm::vec4(
            std::min<f32>(static_cast<f32>(model.size_x), 128.0f),
            std::min<f32>(static_cast<f32>(model.size_z), 128.0f),
            std::min<f32>(static_cast<f32>(model.size_y), 128.0f),
            0);
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

    void do_blockedit(daxa::CommandListHandle cmd_list, u32 compute_globals_i, u32 block_id) {
        i32 x_min = -1;
        i32 y_min = -1;
        i32 z_min = -1;
        i32 x_max = +1;
        i32 y_max = +1;
        i32 z_max = +1;
        cmd_list.bindPipeline(blockedit_compute_pipeline);
        cmd_list.bindAll();
        for (i32 zi = z_min; zi <= z_max; ++zi) {
            for (i32 yi = y_min; yi <= y_max; ++yi) {
                for (i32 xi = x_min; xi <= x_max; ++xi) {
                    cmd_list.pushConstant(
                        VK_SHADER_STAGE_COMPUTE_BIT,
                        BlockeditPush{
                            .pos = {1.0f * xi, 1.0f * yi, 1.0f * zi, 0},
                            .globals_sb = compute_globals_i,
                            // This is unnecessary, because we derive the ID in the
                            // shader based on where the pick_pos was calculated to be.
                            // We do that to save a read-back, and also it's more
                            // immediate this way.
                            // .output_image_i = compute_globals.chunk_ids[zi][yi][xi],
                            .set_id = block_id,
                        });
                    cmd_list.dispatch(8, 8, 8);
                }
            }
        }

        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);

        for (i32 zi = z_min; zi <= z_max; ++zi)
            for (i32 yi = y_min; yi <= y_max; ++yi)
                for (i32 xi = x_min; xi <= x_max; ++xi)
                    update_subchunk_x2x4(cmd_list, {xi, yi, zi}, 1); // derive id mode (1)
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        for (i32 zi = z_min; zi <= z_max; ++zi)
            for (i32 yi = y_min; yi <= y_max; ++yi)
                for (i32 xi = x_min; xi <= x_max; ++xi)
                    update_subchunk_x8p(cmd_list, {xi, yi, zi}, 1); // derive id mode (1)
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
    }

    void update_subchunk(daxa::CommandListHandle cmd_list, glm::uvec3 chunk_i, u32 mode = 0) {
        update_subchunk_x2x4(cmd_list, chunk_i, mode);
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
        update_subchunk_x8p(cmd_list, chunk_i, mode);
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
    }

    void update_subchunk_x2x4(daxa::CommandListHandle cmd_list, glm::uvec3 chunk_i, u32 mode = 0) {
        auto compute_globals_i = compute_pipeline_globals.getDescriptorIndex();
        cmd_list.bindPipeline(subchunk_x2x4_pipeline);
        cmd_list.bindAll();
        cmd_list.pushConstant(
            VK_SHADER_STAGE_COMPUTE_BIT,
            SubChunkPush{
                .chunk_i = {chunk_i.x, chunk_i.y, chunk_i.z, 0},
                .globalsID = compute_globals_i,
                .mode = mode,
            });
        cmd_list.dispatch(1, 64, 1);
    }

    void update_subchunk_x8p(daxa::CommandListHandle cmd_list, glm::uvec3 chunk_i, u32 mode = 0) {
        auto compute_globals_i = compute_pipeline_globals.getDescriptorIndex();
        cmd_list.bindPipeline(subchunk_x8p_pipeline);
        cmd_list.bindAll();
        cmd_list.pushConstant(
            VK_SHADER_STAGE_COMPUTE_BIT,
            SubChunkPush{
                .chunk_i = {chunk_i.x, chunk_i.y, chunk_i.z, 0},
                .globalsID = compute_globals_i,
                .mode = mode,
            });
        cmd_list.dispatch(1, 1, 1);
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
            .empty_chunk_index = empty_chunk->chunkgen_image_a->getDescriptorIndex(),
            .single_ray_steps = static_cast<u32>(single_ray_steps),
        };

        for (size_t zi = 0; zi < DIM.z * CHUNK_REPEAT.z; ++zi) {
            for (size_t yi = 0; yi < DIM.y * CHUNK_REPEAT.y; ++yi) {
                for (size_t xi = 0; xi < DIM.x * CHUNK_REPEAT.x; ++xi) {
                    if (chunks[zi % DIM.z][yi][xi % DIM.x]->initialized)
                        compute_globals.chunk_ids[zi][yi][xi] = chunks[zi % DIM.z][yi][xi % DIM.x]->chunkgen_image_a->getDescriptorIndex();
                    else
                        compute_globals.chunk_ids[zi][yi][xi] = empty_chunk->chunkgen_image_a->getDescriptorIndex();
                }
            }
        }

        auto compute_modelload_i = model_buffer.getDescriptorIndex();
        compute_globals.model_load_index = compute_modelload_i;

        cmd_list.singleCopyHostToBuffer({
            .src = reinterpret_cast<u8 *>(model_load_buffer.get()),
            .dst = model_buffer,
            .region = {.size = sizeof(ModelLoadBuffer)},
        });
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);

        auto compute_globals_i = compute_pipeline_globals.getDescriptorIndex();

        cmd_list.singleCopyHostToBuffer({
            .src = reinterpret_cast<u8 *>(&compute_globals),
            .dst = compute_pipeline_globals,
            .region = {.size = sizeof(decltype(compute_globals))},
        });
        cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);

        if (!initialized) {
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

                compute_globals.chunk_ids[zi][yi][xi] = chunks[zi][yi][xi]->chunkgen_image_a->getDescriptorIndex();
                cmd_list.singleCopyHostToBuffer({
                    .src = reinterpret_cast<u8 *>(&compute_globals),
                    .dst = compute_pipeline_globals,
                    .region = {.size = sizeof(decltype(compute_globals))},
                });
                cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);

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

                update_subchunk(cmd_list, {xi, yi, zi});
                cmd_list.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
            }
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

        // cmd_list.singleCopyBufferToHost({
        //     .src = compute_pipeline_globals,
        //     .region = {
        //         .srcOffset = offsetof(ComputeGlobals, pick_pos),
        //         .size = sizeof(ComputeGlobals::pick_pos),
        //     },
        // });

        if (should_place) {
            // should_place = false;
            do_blockedit(cmd_list, compute_globals_i, 3); // brick
        }
        if (should_break) {
            // should_break = false;
            do_blockedit(cmd_list, compute_globals_i, 1); // air
        }

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

    std::unique_ptr<RenderableChunk> create_chunk() {
        constexpr auto IMG_DIM = glm::uvec3(Chunk::DIM);

        auto chunk = std::make_unique<RenderableChunk>();
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
        return chunk;
    }

    void load_shaders() {
        create_pipeline(raymarch_compute_pipeline, "drawing/raymarch.hlsl", daxa::ShaderLang::HLSL);
        // create_pipeline(pickblock_compute_pipeline, "utils/pickblock.comp");
        create_pipeline(pickblock_compute_pipeline, "utils/pickblock.hlsl", daxa::ShaderLang::HLSL);
        create_pipeline(blockedit_compute_pipeline, "utils/blockedit.hlsl", daxa::ShaderLang::HLSL);
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
