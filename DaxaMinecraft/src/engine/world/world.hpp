#pragma once
#include <engine/world/renderable_chunk.hpp>

#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <thread>

struct World {
    static constexpr int RENDER_DIST_XZ = 4;
    static constexpr glm::ivec3 CHUNK_MAX{16, 6, 16};

    template <typename T>
    using WorldArray = std::array<std::array<std::array<T, CHUNK_MAX.x>, CHUNK_MAX.y>, CHUNK_MAX.z>;

    WorldArray<RenderableChunk *> chunks{};

    using ChunkgenBuffer = WorldArray<Chunk::Buffer<u32>>;
    daxa::gpu::BufferHandle chunkgen_buffers[2];

    struct Globals {
        glm::mat4 viewproj_mat;
    };
    daxa::gpu::BindingSetAllocatorHandle globals_uniform_allocator;
    daxa::gpu::BufferHandle globals_uniform_buffer;

    daxa::gpu::BindingSetAllocatorHandle compute_binding_set_allocator;

    daxa::gpu::PipelineHandle graphics_pipeline;
    Texture atlas_texture;

    std::filesystem::path vert_path{"DaxaMinecraft/assets/chunk.vert"};
    std::filesystem::path frag_path{"DaxaMinecraft/assets/chunk.frag"};

    struct ChunkGenComputeState {
        daxa::gpu::PipelineHandle pipeline{};
        std::filesystem::path path;
        std::chrono::system_clock::rep last_reload_time = 0;
    };

    std::array<ChunkGenComputeState, 2> chunkgen_passes{
        ChunkGenComputeState{.path = "DaxaMinecraft/assets/chunk_block_pass1.comp"},
        ChunkGenComputeState{.path = "DaxaMinecraft/assets/chunk_block_pass2.comp"},
    };

    std::chrono::system_clock::rep last_vert_reload_time = 0, last_frag_reload_time = 0;

    static constexpr glm::ivec3 chunk_min{-8, -2, -8};
    static constexpr glm::ivec3 chunk_max = CHUNK_MAX + chunk_min;

    bool chunks_invalidated = true;

    World(RenderContext &render_ctx)
        : atlas_texture(render_ctx, "DaxaMinecraft/assets/textures") {
        try_reload_shaders(render_ctx);

        globals_uniform_allocator = render_ctx.device->createBindingSetAllocator({
            .setLayout = graphics_pipeline->getSetLayout(0),
        });
        globals_uniform_buffer = render_ctx.device->createBuffer({
            .size = sizeof(Globals),
            .usage =
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
        });

        compute_binding_set_allocator = render_ctx.device->createBindingSetAllocator({
            .setLayout = chunkgen_passes[0].pipeline->getSetLayout(0),
        });

        for (auto &chunkgen_buffer : chunkgen_buffers) {
            chunkgen_buffer = render_ctx.device->createBuffer({
                .size = sizeof(ChunkgenBuffer),
                .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                .memoryUsage = VmaMemoryUsage(VMA_MEMORY_USAGE_GPU_TO_CPU),
                .memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            });
        }
    }

    ~World() {
        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto &current_chunk_ptr = chunks[zi - chunk_min.z][yi - chunk_min.y][xi - chunk_min.x];
                    delete current_chunk_ptr;
                }
            }
        }
    }

    void reload_compute_pipeline(RenderContext &render_ctx) {
        for (auto &chunkgen_pass : chunkgen_passes) {
            auto new_pipeline1 = render_ctx.pipelineCompiler->createComputePipeline({
                .shaderCI = {
                    .pathToSource = chunkgen_pass.path.string().c_str(),
                    .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                }
            });
            if (!new_pipeline1)
                continue;
            chunkgen_pass.pipeline = new_pipeline1.value();
        }
    }

    void reload_graphics_pipeline(RenderContext &render_ctx) {
        try {
 
            daxa::gpu::GraphicsPipelineBuilder pipeline_builder;
            pipeline_builder
                .addShaderStage({
                    .pathToSource = vert_path,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                })
                .addShaderStage({
                    .pathToSource = frag_path,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                })
                .beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
                .addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)
                .addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)
                .addVertexInputAttribute(VK_FORMAT_R32_SINT)
                .endVertexInputAttributeBinding()
                .addColorAttachment(render_ctx.swapchain->getVkFormat())
                .setRasterization({
                    .cullMode = VK_CULL_MODE_FRONT_BIT,
                })
                .configurateDepthTest({
                    .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
                    .enableDepthTest = VK_TRUE,
                    .enableDepthWrite = VK_TRUE,
                    .depthTestCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
                });

            auto new_pipeline =
                render_ctx.pipelineCompiler->createGraphicsPipeline(pipeline_builder);
            if (!new_pipeline) {
                std::cout << new_pipeline.message() << std::endl;
                throw;
            }

            // render_ctx.queue->waitForFlush();
            // render_ctx.queue->checkForFinishedSubmits();
            // render_ctx.device->waitIdle();

            graphics_pipeline = new_pipeline.value();
        } catch (...) {
            std::cout << "Failed to re-compile the graphics pipeline's shaders, using "
                         "the previous pipeline\n";
        }
    }

    void try_reload_shaders(RenderContext &render_ctx) {
        // reload graphics
        auto last_vert_write_time =
            std::filesystem::last_write_time(vert_path).time_since_epoch().count();
        auto last_frag_write_time =
            std::filesystem::last_write_time(frag_path).time_since_epoch().count();

        if (last_vert_write_time > last_vert_reload_time ||
            last_frag_write_time > last_frag_reload_time) {
            reload_graphics_pipeline(render_ctx);
            last_vert_reload_time = last_vert_write_time;
            last_frag_reload_time = last_frag_write_time;
        }

        // reload compute
        bool should_reload_compute = false;
        for (auto &chunkgen_pass : chunkgen_passes) {
            auto last_write_time =
                std::filesystem::last_write_time(chunkgen_pass.path)
                    .time_since_epoch()
                    .count();
            if (last_write_time > chunkgen_pass.last_reload_time) {
                should_reload_compute = true;
                chunkgen_pass.last_reload_time = last_write_time;
            }
        }
        if (should_reload_compute) {
            chunks_invalidated = true;
            reload_compute_pipeline(render_ctx);
        }
    }

    void update_neighbors() {
        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    glm::ivec3 index(xi - chunk_min.x, yi - chunk_min.y, zi - chunk_min.z);
                    auto current_chunk_ptr = chunks[index.z][index.y][index.x];

                    if (xi != chunk_min.x) {
                        auto neighbor_chunk_ptr = chunks[index.z][index.y][index.x - 1];
                        current_chunk_ptr->chunk.neighbors[0] = &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors[0] = nullptr;
                    }
                    if (xi != chunk_max.x - 1) {
                        auto neighbor_chunk_ptr = chunks[index.z][index.y][index.x + 1];
                        current_chunk_ptr->chunk.neighbors[1] = &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors[1] = nullptr;
                    }

                    if (yi != chunk_min.y) {
                        auto neighbor_chunk_ptr = chunks[index.z][index.y - 1][index.x];
                        current_chunk_ptr->chunk.neighbors[2] = &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors[2] = nullptr;
                    }
                    if (yi != chunk_max.y - 1) {
                        auto neighbor_chunk_ptr = chunks[index.z][index.y + 1][index.x];
                        current_chunk_ptr->chunk.neighbors[3] = &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors[3] = nullptr;
                    }

                    if (zi != chunk_min.z) {
                        auto neighbor_chunk_ptr = chunks[index.z - 1][index.y][index.x];
                        current_chunk_ptr->chunk.neighbors[4] = &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors[4] = nullptr;
                    }
                    if (zi != chunk_max.z - 1) {
                        auto neighbor_chunk_ptr = chunks[index.z + 1][index.y][index.x];
                        current_chunk_ptr->chunk.neighbors[5] = &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors[5] = nullptr;
                    }
                }
            }
        }
    }

    void update(daxa::gpu::CommandListHandle cmd_list, glm::mat4 &viewproj_mat) {
        cmd_list->copyHostToBuffer(daxa::gpu::HostToBufferCopyInfo{
            .src = &viewproj_mat,
            .dst = globals_uniform_buffer,
            .size = sizeof(glm::mat4),
        });

        // size_t vert_n = 0;
        // size_t chunk_n = 0;

        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto &current_chunk_ptr = chunks[zi - chunk_min.z][yi - chunk_min.y][xi - chunk_min.x];
                    current_chunk_ptr->update(cmd_list);

                    // chunk_n++;
                    // vert_n += current_chunk_ptr->chunk.vert_n;
                }
            }
        }

        // std::cout << "vert_n = " << vert_n << "\n";
        // std::cout << "bytes = " << vert_n * sizeof(Vertex) << "\n";
        // std::cout << "chunk_n = " << chunk_n << "\n";
    }

    void draw(daxa::gpu::CommandListHandle cmd_list) {
        cmd_list->bindPipeline(graphics_pipeline);

        auto set = globals_uniform_allocator->getSet();
        set->bindBuffer(0, globals_uniform_buffer);
        set->bindImage(1, atlas_texture.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        cmd_list->bindSet(0, set);

        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto &current_chunk_ptr = chunks[zi - chunk_min.z][yi - chunk_min.y][xi - chunk_min.x];
                    current_chunk_ptr->draw(cmd_list);
                }
            }
        }
    }

    Chunk *get_containing_chunk_impl(const glm::vec3 &pos, Chunk *current_chunk) {
        if (current_chunk == nullptr)
            return nullptr;
        glm::vec3 current_chunk_pos = glm::vec3(current_chunk->pos) * Chunk::FLOAT_DIM;

        if (pos.x < current_chunk_pos.x) {
            return get_containing_chunk_impl(pos, current_chunk->neighbors[0]);
        } else if (pos.x >= current_chunk_pos.x + Chunk::FLOAT_DIM.x) {
            return get_containing_chunk_impl(pos, current_chunk->neighbors[1]);
        } else {
            if (pos.y < current_chunk_pos.y) {
                return get_containing_chunk_impl(pos, current_chunk->neighbors[2]);
            } else if (pos.y >= current_chunk_pos.y + Chunk::FLOAT_DIM.y) {
                return get_containing_chunk_impl(pos, current_chunk->neighbors[3]);
            } else {
                if (pos.z < current_chunk_pos.z) {
                    return get_containing_chunk_impl(pos, current_chunk->neighbors[4]);
                } else if (pos.z >= current_chunk_pos.z + Chunk::FLOAT_DIM.z) {
                    return get_containing_chunk_impl(pos, current_chunk->neighbors[5]);
                } else {
                    return current_chunk;
                }
            }
        }
    }

    Chunk *get_containing_chunk(glm::vec3 pos) {
        return get_containing_chunk_impl(
            pos, &chunks[CHUNK_MAX.z / 2][CHUNK_MAX.y / 2][CHUNK_MAX.x / 2]->chunk);
    }

    Block *get_containing_block(glm::vec3 pos) {
        auto containing_chunk = get_containing_chunk(pos);
        if (containing_chunk)
            return containing_chunk->get_containing_block(pos);
        return nullptr;
    }

    void generate_chunks(RenderContext &render_ctx) {
        chunks_invalidated = false;
        auto chunk_pos = glm::vec3(chunk_min);

        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto &current_chunk_ptr = chunks[zi - chunk_min.z][yi - chunk_min.y][xi - chunk_min.x];
                    if (current_chunk_ptr)
                        delete current_chunk_ptr;
                    current_chunk_ptr = new RenderableChunk(render_ctx, {xi, yi, zi});
                }
            }
        }

        using Clock = std::chrono::high_resolution_clock;
        auto start = Clock::now();

        auto cmd_list = render_ctx.queue->getCommandList({});

        auto set = compute_binding_set_allocator->getSet();
        cmd_list->bindPipeline(chunkgen_passes[0].pipeline);
        set->bindBuffer(0, chunkgen_buffers[0]);
        set->bindBuffer(1, chunkgen_buffers[1]);
        cmd_list->bindSet(0, set);
        cmd_list->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, chunk_pos);
        cmd_list->dispatch(CHUNK_MAX.x, CHUNK_MAX.y, CHUNK_MAX.z * Chunk::NZ);

        cmd_list->insertMemoryBarrier(daxa::gpu::MemoryBarrier{
            .srcStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
            .dstStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
        });

        cmd_list->bindPipeline(chunkgen_passes[1].pipeline);
        cmd_list->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, chunk_pos);
        cmd_list->dispatch(CHUNK_MAX.x, CHUNK_MAX.y, CHUNK_MAX.z * Chunk::NZ);

        cmd_list->finalize();
        daxa::gpu::SubmitInfo submit_info;
        submit_info.commandLists.push_back(std::move(cmd_list));
        auto t0 = std::chrono::duration<float, std::milli>(Clock::now() - start).count();
        render_ctx.queue->submitBlocking(submit_info);
        render_ctx.queue->checkForFinishedSubmits();

        auto generated_data = chunkgen_buffers[1].mapMemory<const u32>();

        if (generated_data.hostPtr) {
            for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
                for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                    for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                        glm::ivec3 index{xi - chunk_min.x, yi - chunk_min.y, zi - chunk_min.z};
                        auto &current_chunk_ptr = chunks[index.z][index.y][index.x];
                        size_t chunk_i = index.x + index.y * CHUNK_MAX.x + index.z * CHUNK_MAX.x * CHUNK_MAX.y;
                        size_t offset = sizeof(Chunk::Buffer<u32>) / sizeof(u32) * chunk_i;
                        current_chunk_ptr->chunk.copy_block_data(
                            *reinterpret_cast<const Chunk::BlockBuffer *>(
                                generated_data.hostPtr + offset));
                    }
                }
            }
        }

        auto elapsed = std::chrono::duration<float, std::milli>(Clock::now() - start).count();
        std::cout << "t0: " << t0 << ", elapsed: " << elapsed << " ms\n";

        update_neighbors();

        // std::vector<Structure> structures;
        // for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
        //     for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
        //         for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
        //             auto &current_chunk_ptr = chunks[zi - chunk_min.z][yi - chunk_min.y][xi - chunk_min.x];
        //             auto &chunkgen_buffer = chunkgen_buffers[zi - chunk_min.z][yi - chunk_min.y][xi - chunk_min.x];
        //             current_chunk_ptr->chunk.generate_block_data_pass2(structures);
        //         }
        //     }
        // }
        // for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
        //     for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
        //         for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
        //             auto &current_chunk_ptr = chunks[zi - chunk_min.z][yi - chunk_min.y][xi - chunk_min.x];
        //             auto &chunkgen_buffer = chunkgen_buffers[zi - chunk_min.z][yi - chunk_min.y][xi - chunk_min.x];
        //             current_chunk_ptr->chunk.generate_block_data_structures(structures);
        //         }
        //     }
        // }
    }
};
