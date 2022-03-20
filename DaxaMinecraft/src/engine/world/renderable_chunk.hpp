#pragma once

#include <engine/graphics.hpp>
#include <engine/world/chunk.hpp>

std::array<Vertex, Chunk::VERT_N> host_vertex_buffer{};

struct RenderableChunk {
    daxa::gpu::BufferHandle vertex_buffer;

    Chunk chunk;
    glm::mat4 modl_mat = glm::translate(glm::mat4(1), Chunk::FLOAT_DIM *glm::vec3(chunk.pos));

    RenderableChunk(RenderContext &render_ctx, glm::ivec3 p) : chunk{.pos = p} {
        vertex_buffer = render_ctx.device->createBuffer({
            .size = Chunk::MAX_SIZE,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
        });
        // chunk.generate_block_data();
    }

    void update(daxa::gpu::CommandListHandle cmd_list) {
        if (chunk.needs_remesh) {
            auto mapped_vertices = host_vertex_buffer.data();
            chunk.generate_mesh_data(mapped_vertices);
            if (chunk.vert_n == 0)
                return;
            cmd_list->copyHostToBuffer(daxa::gpu::HostToBufferCopyInfo{
                .src = host_vertex_buffer.data(),
                .dst = vertex_buffer,
                .size = chunk.vert_n * sizeof(Vertex),
            });
        }
    }

    void draw(daxa::gpu::CommandListHandle cmd_list) {
        if (chunk.vert_n == 0)
            return;
        cmd_list->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, modl_mat);
        cmd_list->bindVertexBuffer(0, vertex_buffer);
        cmd_list->draw(chunk.vert_n, 1, 0, 0);
    }
};
