#pragma once
#include <engine/world/renderable_chunk.hpp>

#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <thread>

struct World {
    static constexpr int        RENDER_DIST_XZ = 1;
    static constexpr glm::ivec3 CHUNK_MAX{1, 1, 1};

    std::array<std::array<std::array<RenderableChunk *, CHUNK_MAX.x>, CHUNK_MAX.y>,
               CHUNK_MAX.z>
        chunks;

    struct Globals {
        glm::mat4 viewproj_mat;
        glm::vec3 cam_pos;
    };
    daxa::gpu::BindingSetAllocatorHandle globals_uniform_allocator;
    daxa::gpu::BufferHandle              globals_uniform_buffer;

    daxa::gpu::BindingSetAllocatorHandle compute_binding_set_allocator;
    daxa::gpu::BufferHandle              chunk_buffer, chunk_upload_buffer;

    daxa::gpu::PipelineHandle graphics_pipeline;
    Texture                   atlas_texture;

    std::filesystem::path vert_path{"DaxaMinecraftRaytraced/assets/shaders/chunk.vert"};
    std::filesystem::path frag_path{"DaxaMinecraftRaytraced/assets/shaders/chunk.frag"};

    daxa::gpu::PipelineHandle chunk_block_pass1_compute_pipeline;

    std::filesystem::path chunk_block_pass1_comp_path =
        "DaxaMinecraftRaytraced/assets/shaders/chunk.comp";

    std::chrono::system_clock::rep last_vert_reload_time = 0, last_frag_reload_time = 0;
    std::chrono::system_clock::rep last_comp1_reload_time = 0, last_comp2_reload_time = 0,
                                   last_comp3_reload_time = 0;

    static constexpr glm::ivec3 chunk_min{0, 0, 0};
    static constexpr glm::ivec3 chunk_max{1, 1, 1};

    World(RenderContext & render_ctx)
        : atlas_texture(render_ctx, "DaxaMinecraft/assets/atlas.png") {
        try_reload_shaders(render_ctx);

        globals_uniform_allocator = render_ctx.device->createBindingSetAllocator({
            graphics_pipeline->getSetDescription(0)});
        globals_uniform_buffer = render_ctx.device->createBuffer({
            .size = sizeof(Globals),
            .usage =
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
        });

        compute_binding_set_allocator = render_ctx.device->createBindingSetAllocator({
            chunk_block_pass1_compute_pipeline->getSetDescription(0)});

        chunk_buffer = render_ctx.device->createBuffer({
            .size = sizeof(Chunk::BlockBuffer),
            .usage =
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryUsage      = VmaMemoryUsage(VMA_MEMORY_USAGE_GPU_TO_CPU),
            .memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        });

        chunk_upload_buffer = render_ctx.device->createBuffer({
            .size = sizeof(Chunk::BlockBuffer),
            .usage =
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryUsage      = VmaMemoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU),
            .memoryProperties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        });

        generate_chunk(render_ctx);

        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto & current_chunk_ptr =
                        chunks[xi - chunk_min.x][yi - chunk_min.y][zi - chunk_min.z];
                    current_chunk_ptr = new RenderableChunk(render_ctx, {xi, yi, zi});
                }
            }
        }
        update_neighbors();

        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto & current_chunk_ptr =
                        chunks[xi - chunk_min.x][yi - chunk_min.y][zi - chunk_min.z];
                    current_chunk_ptr->chunk.generate_block_data_pass2();
                }
            }
        }
    }

    ~World() {
        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto & current_chunk_ptr =
                        chunks[xi - chunk_min.x][yi - chunk_min.y][zi - chunk_min.z];
                    delete current_chunk_ptr;
                }
            }
        }
    }

    void reload_compute_pipeline(RenderContext & render_ctx) {
        try {
            std::ifstream comp1_file(chunk_block_pass1_comp_path);
            if (!comp1_file.is_open())
                throw std::runtime_error("failed to open comp1 shader file");
            std::stringstream comp1_sstr;
            comp1_sstr << comp1_file.rdbuf();
            comp1_file.close();
            const auto & comp1_str = comp1_sstr.str();

            auto comp1_shader =
                render_ctx.device
                    ->createShaderModule({
                        .sourceToGLSL = comp1_str.c_str(),
                        .stage = VK_SHADER_STAGE_COMPUTE_BIT
                    }).value();

            auto new_pipeline1 = render_ctx.device->createComputePipeline({comp1_shader});
            if (!new_pipeline1) throw;

            chunk_block_pass1_compute_pipeline = new_pipeline1;
        } catch (...) {
            std::cout << "Failed to re-compile the compute pipeline's shaders, using "
                         "the previous pipeline\n";
        }
    }

    void reload_graphics_pipeline(RenderContext & render_ctx) {
        try {
            std::ifstream vert_file(vert_path);
            if (!vert_file.is_open())
                throw std::runtime_error("failed to open vert shader file");
            std::stringstream vert_sstr;
            vert_sstr << vert_file.rdbuf();
            vert_file.close();
            const auto & vert_str = vert_sstr.str();

            std::ifstream frag_file(frag_path);
            if (!frag_file.is_open())
                throw std::runtime_error("failed to open frag shader file");
            std::stringstream frag_sstr;
            frag_sstr << frag_file.rdbuf();
            frag_file.close();
            const auto & frag_str = frag_sstr.str();

            auto vert_shader =
                render_ctx.device
                    ->createShaderModule({
                        .sourceToGLSL = vert_str.c_str(),
                        .stage = VK_SHADER_STAGE_VERTEX_BIT
                    }).value();
            auto frag_shader =
                render_ctx.device
                    ->createShaderModule({
                        .sourceToGLSL = frag_str.c_str(),
                        .stage = VK_SHADER_STAGE_FRAGMENT_BIT
                    }).value();

            daxa::gpu::GraphicsPipelineBuilder pipeline_builder;
            pipeline_builder
                .setRasterization({
                    .polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL,
                    .cullMode    = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT,
                    .frontFace   = VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    .lineWidth   = 1.0f,
                })
                .addShaderStage(vert_shader)
                .addShaderStage(frag_shader)
                .beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
                .addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)
                .addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)
                .addVertexInputAttribute(VK_FORMAT_R32G32_SFLOAT)
                .addColorAttachment(render_ctx.swapchain->getVkFormat())
                .configurateDepthTest({
                    .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
                    .enableDepthTest       = VK_TRUE,
                    .enableDepthWrite      = VK_TRUE,
                    .depthTestCompareOp    = VK_COMPARE_OP_LESS_OR_EQUAL,
                });

            auto new_pipeline =
                render_ctx.device->createGraphicsPipeline(pipeline_builder);
            if (!new_pipeline) throw;

            graphics_pipeline = new_pipeline;
        } catch (...) {
            std::cout << "Failed to re-compile the graphics pipeline's shaders, using "
                         "the previous pipeline\n";
        }
    }

    void try_reload_shaders(RenderContext & render_ctx) {
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
        auto last_comp1_write_time =
            std::filesystem::last_write_time(chunk_block_pass1_comp_path)
                .time_since_epoch()
                .count();

        if (last_comp1_write_time > last_comp1_reload_time) {
            reload_compute_pipeline(render_ctx);
            last_comp1_reload_time = last_comp1_write_time;
        }
    }

    void update_neighbors() {
        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    glm::ivec3 index(xi - chunk_min.x, yi - chunk_min.y,
                                     zi - chunk_min.z);

                    auto current_chunk_ptr = chunks[index.x][index.y][index.z];

                    if (xi != chunk_min.x) {
                        auto neighbor_chunk_ptr = chunks[index.x - 1][index.y][index.z];
                        current_chunk_ptr->chunk.neighbors.nx =
                            &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors.nx = nullptr;
                    }
                    if (xi != chunk_max.x - 1) {
                        auto neighbor_chunk_ptr = chunks[index.x + 1][index.y][index.z];
                        current_chunk_ptr->chunk.neighbors.px =
                            &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors.px = nullptr;
                    }

                    if (yi != chunk_min.y) {
                        auto neighbor_chunk_ptr = chunks[index.x][index.y - 1][index.z];
                        current_chunk_ptr->chunk.neighbors.ny =
                            &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors.ny = nullptr;
                    }
                    if (yi != chunk_max.y - 1) {
                        auto neighbor_chunk_ptr = chunks[index.x][index.y + 1][index.z];
                        current_chunk_ptr->chunk.neighbors.py =
                            &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors.py = nullptr;
                    }

                    if (zi != chunk_min.z) {
                        auto neighbor_chunk_ptr = chunks[index.x][index.y][index.z - 1];
                        current_chunk_ptr->chunk.neighbors.nz =
                            &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors.nz = nullptr;
                    }
                    if (zi != chunk_max.z - 1) {
                        auto neighbor_chunk_ptr = chunks[index.x][index.y][index.z + 1];
                        current_chunk_ptr->chunk.neighbors.pz =
                            &(neighbor_chunk_ptr->chunk);
                    } else {
                        current_chunk_ptr->chunk.neighbors.pz = nullptr;
                    }
                }
            }
        }
    }

    void update(daxa::gpu::CommandListHandle cmd_list, Globals globals) {
        cmd_list->copyHostToBuffer(daxa::gpu::HostToBufferCopyInfo{
            .src  = &globals,
            .dst  = globals_uniform_buffer,
            .size = sizeof(Globals),
        });

        // uint64_t TOTAL_VERTS = 0;
        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto & current_chunk_ptr =
                        chunks[xi - chunk_min.x][yi - chunk_min.y][zi - chunk_min.z];
                    current_chunk_ptr->update(cmd_list);
                    // TOTAL_VERTS += current_chunk_ptr->chunk.vert_n;
                }
            }
        }
        // std::cout << "Tris: " << TOTAL_VERTS / 3 << "\n";
    }

    void draw(daxa::gpu::CommandListHandle cmd_list) {
        cmd_list->bindPipeline(graphics_pipeline);

        auto set = globals_uniform_allocator->getSet();
        set->bindBuffer(0, globals_uniform_buffer);
        set->bindImage(1, atlas_texture.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        set->bindBuffer(2, chunk_buffer);
        cmd_list->bindSet(0, set);

        for (int zi = chunk_min.z; zi < chunk_max.z; ++zi) {
            for (int yi = chunk_min.y; yi < chunk_max.y; ++yi) {
                for (int xi = chunk_min.x; xi < chunk_max.x; ++xi) {
                    auto & current_chunk_ptr =
                        chunks[xi - chunk_min.x][yi - chunk_min.y][zi - chunk_min.z];
                    current_chunk_ptr->draw(cmd_list);
                }
            }
        }
    }

    Chunk * get_containing_chunk_impl(const glm::vec3 & pos, Chunk * current_chunk) {
        if (current_chunk == nullptr) return nullptr;
        glm::vec3 current_chunk_pos = glm::vec3(current_chunk->pos) * Chunk::FLOAT_DIM;

        if (pos.x < current_chunk_pos.x) {
            return get_containing_chunk_impl(pos, current_chunk->neighbors.nx);
        } else if (pos.x >= current_chunk_pos.x + Chunk::FLOAT_DIM.x) {
            return get_containing_chunk_impl(pos, current_chunk->neighbors.px);
        } else {
            if (pos.y < current_chunk_pos.y) {
                return get_containing_chunk_impl(pos, current_chunk->neighbors.ny);
            } else if (pos.y >= current_chunk_pos.y + Chunk::FLOAT_DIM.y) {
                return get_containing_chunk_impl(pos, current_chunk->neighbors.py);
            } else {
                if (pos.z < current_chunk_pos.z) {
                    return get_containing_chunk_impl(pos, current_chunk->neighbors.nz);
                } else if (pos.z >= current_chunk_pos.z + Chunk::FLOAT_DIM.z) {
                    return get_containing_chunk_impl(pos, current_chunk->neighbors.pz);
                } else {
                    return current_chunk;
                }
            }
        }
    }

    Chunk * get_containing_chunk(glm::vec3 pos) {
        return get_containing_chunk_impl(
            pos, &chunks[CHUNK_MAX.x / 2][CHUNK_MAX.y / 2][CHUNK_MAX.z / 2]->chunk);
    }

    Block * get_containing_block(glm::vec3 pos) {
        auto containing_chunk = get_containing_chunk(pos);
        if (containing_chunk) return containing_chunk->get_containing_block(pos);
        return nullptr;
    }

    void generate_chunk(RenderContext & render_ctx) {
        auto cmd_list = render_ctx.device->getCommandList();
        cmd_list->bindPipeline(chunk_block_pass1_compute_pipeline);

        auto set = compute_binding_set_allocator->getSet();
        set->bindBuffer(0, chunk_upload_buffer);
        cmd_list->bindSet(0, set);

        auto chunk_pos = glm::vec3{0, 0, 0};
        cmd_list->pushConstant(VK_SHADER_STAGE_COMPUTE_BIT, chunk_pos);
        cmd_list->dispatch(1, 1, Chunk::NZ);
        cmd_list->insertMemoryBarrier(daxa::gpu::MemoryBarrier{
            .srcStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
            .dstStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
        });

        // cmd_list->bindPipeline(chunk_block_pass2_compute_pipeline);
        // cmd_list->dispatch(1, 1, Chunk::NZ);

        cmd_list->finalize();

        daxa::gpu::SubmitInfo submit_info;
        submit_info.commandLists.push_back(std::move(cmd_list));
        render_ctx.queue->submitBlocking(submit_info);
        render_ctx.queue->checkForFinishedSubmits();

        auto upload_data = chunk_upload_buffer.mapMemory<Chunk::BlockBuffer>();
        for (auto & layer : *upload_data.hostPtr) {
            for (auto & strip : layer) {
                for (auto & tile : strip) {
                    u32 id = rand() % 5 + 1;
                    tile = {.id = BlockID(id)};
                }
            }
        }
    }
};
