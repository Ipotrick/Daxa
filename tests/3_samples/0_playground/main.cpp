#include <0_common/window.hpp>
#include <0_common/player.hpp>
#include <thread>

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

struct Vertex
{
    u32 data;
    Vertex(u32 x, u32 y, u32 z, u32 side)
    {
        data = 0;
        data |= x << 0x00;
        data |= y << 0x05;
        data |= z << 0x0a;
        data |= side << 0x0f;
    }
};

// struct RasterInput
// {
//     glm::mat4 view_mat;
// };

// struct RasterGlobals
// {
// };

struct RasterPush
{
    glm::mat4 view_mat;
    glm::vec3 chunk_pos;
    daxa::BufferId vertex_buffer_id;
};

static inline constexpr u64 CHUNK_SIZE = 32;
static inline constexpr u64 CHUNK_VOXEL_N = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
static inline constexpr u64 CHUNK_MAX_VERTS = CHUNK_VOXEL_N * 6;
static inline constexpr u64 CHUNK_MAX_SIZE = CHUNK_MAX_VERTS * sizeof(Vertex);

static inline constexpr u64 CHUNK_N = 8;

struct VoxelChunk
{
    u32 voxels[CHUNK_VOXEL_N];
    glm::vec3 pos;

    u32 generate_block(glm::vec3 p)
    {
        return std::sin(p.x * 0.04) * 40 + std::sin(p.z * 0.05 + 10) * 45 < p.y - 100;
    }

    void generate()
    {
        for (u64 zi = 0; zi < 32; ++zi)
            for (u64 yi = 0; yi < 32; ++yi)
                for (u64 xi = 0; xi < 32; ++xi)
                {
                    u64 i = xi + yi * CHUNK_SIZE + zi * CHUNK_SIZE * CHUNK_SIZE;
                    glm::vec3 block_pos = glm::vec3(xi, yi, zi) + pos;
                    voxels[i] = generate_block(block_pos);
                }
    }
};

struct RenderableVoxelWorld;

struct RenderableChunk
{
    VoxelChunk voxel_chunk = {};
    daxa::Device device;
    daxa::BufferId vertex_buffer = device.create_buffer(daxa::BufferInfo{
        .size = CHUNK_MAX_SIZE,
        .debug_name = "Chunk Vertex buffer",
    });
    u32 vert_n = 0;
    RenderableVoxelWorld * renderable_world;

    ~RenderableChunk()
    {
        device.destroy_buffer(vertex_buffer);
    }

    void update_chunk_mesh(daxa::CommandList & cmd_list);

    void draw(daxa::CommandList & cmd_list, glm::mat4 const & view_mat)
    {
        cmd_list.push_constant(RasterPush{
            .view_mat = view_mat,
            .chunk_pos = voxel_chunk.pos,
            .vertex_buffer_id = vertex_buffer,
        });
        cmd_list.draw({.vertex_count = vert_n * 6});
    }
};

struct RenderableVoxelWorld
{
    RenderableChunk * chunks[CHUNK_N * CHUNK_N * CHUNK_N] = {};

    RenderableVoxelWorld(daxa::Device device)
    {
        for (i32 z = 0; z < CHUNK_N; ++z)
        {
            for (i32 y = 0; y < CHUNK_N; ++y)
            {
                for (i32 x = 0; x < CHUNK_N; ++x)
                {
                    chunks[x + y * CHUNK_N + z * CHUNK_N * CHUNK_N] = new RenderableChunk{.device = device};
                    auto & chunk = *chunks[x + y * CHUNK_N + z * CHUNK_N * CHUNK_N];
                    chunk.voxel_chunk.pos = glm::vec3(x * CHUNK_SIZE, y * CHUNK_SIZE, z * CHUNK_SIZE);
                    chunk.voxel_chunk.generate();
                    chunk.renderable_world = this;
                }
            }
        }

        for (i32 z = 0; z < CHUNK_N; ++z)
        {
            for (i32 y = 0; y < CHUNK_N; ++y)
            {
                for (i32 x = 0; x < CHUNK_N; ++x)
                {
                    auto & chunk = *chunks[x + y * CHUNK_N + z * CHUNK_N * CHUNK_N];
                    auto cmd_list = device.create_command_list({
                        .debug_name = "HelloTriangle Command List",
                    });
                    chunk.update_chunk_mesh(cmd_list);
                    cmd_list.complete();
                    device.submit_commands({
                        .command_lists = {std::move(cmd_list)},
                    });
                }
            }
        }
    }

    ~RenderableVoxelWorld()
    {
        for (auto & chunk : chunks)
            delete chunk;
    }

    void draw(daxa::CommandList & cmd_list, glm::mat4 const & view_mat)
    {
        for (auto & chunk : chunks)
        {
            if (chunk->vert_n > 0)
                chunk->draw(cmd_list, view_mat);
        }
    }

    u32 get_voxel(glm::ivec3 p)
    {
        i32 x = p.x / CHUNK_SIZE;
        i32 y = p.y / CHUNK_SIZE;
        i32 z = p.z / CHUNK_SIZE;

        if (p.x < 0 || x >= static_cast<i32>(CHUNK_N))
            return 0;
        if (p.y < 0 || y >= static_cast<i32>(CHUNK_N))
            return 0;
        if (p.z < 0 || z >= static_cast<i32>(CHUNK_N))
            return 0;

        auto & chunk = chunks[x + y * CHUNK_N + z * CHUNK_N * CHUNK_N];

        u64 xi = static_cast<u64>(p.x % CHUNK_SIZE);
        u64 yi = static_cast<u64>(p.y % CHUNK_SIZE);
        u64 zi = static_cast<u64>(p.z % CHUNK_SIZE);
        u64 i = xi + yi * CHUNK_SIZE + zi * CHUNK_SIZE * CHUNK_SIZE;
        return chunk->voxel_chunk.voxels[i];
    }
};

void RenderableChunk::update_chunk_mesh(daxa::CommandList & cmd_list)
{
    auto vertex_staging_buffer = device.create_buffer({
        .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
        .size = CHUNK_MAX_SIZE,
        .debug_name = "HelloTriangle Vertex Staging buffer",
    });
    cmd_list.destroy_buffer_deferred(vertex_staging_buffer);

    auto buffer_ptr = reinterpret_cast<Vertex *>(device.map_memory(vertex_staging_buffer));

    vert_n = 0;
    for (i32 zi = 0; zi < 32; ++zi)
    {
        for (i32 yi = 0; yi < 32; ++yi)
        {
            for (i32 xi = 0; xi < 32; ++xi)
            {
                glm::ivec3 voxel_pos = glm::ivec3{xi, yi, zi} + glm::ivec3(voxel_chunk.pos);
                if (renderable_world->get_voxel(voxel_pos) != 0)
                {
                    if (renderable_world->get_voxel(voxel_pos + glm::ivec3(1, 0, 0)) == 0)
                    {
                        *buffer_ptr = Vertex(xi, yi, zi, 0);
                        ++buffer_ptr;
                        ++vert_n;
                    }
                    if (renderable_world->get_voxel(voxel_pos + glm::ivec3(-1, 0, 0)) == 0)
                    {
                        *buffer_ptr = Vertex(xi, yi, zi, 1);
                        ++buffer_ptr;
                        ++vert_n;
                    }
                    if (renderable_world->get_voxel(voxel_pos + glm::ivec3(0, 1, 0)) == 0)
                    {
                        *buffer_ptr = Vertex(xi, yi, zi, 2);
                        ++buffer_ptr;
                        ++vert_n;
                    }
                    if (renderable_world->get_voxel(voxel_pos + glm::ivec3(0, -1, 0)) == 0)
                    {
                        *buffer_ptr = Vertex(xi, yi, zi, 3);
                        ++buffer_ptr;
                        ++vert_n;
                    }
                    if (renderable_world->get_voxel(voxel_pos + glm::ivec3(0, 0, 1)) == 0)
                    {
                        *buffer_ptr = Vertex(xi, yi, zi, 4);
                        ++buffer_ptr;
                        ++vert_n;
                    }
                    if (renderable_world->get_voxel(voxel_pos + glm::ivec3(0, 0, -1)) == 0)
                    {
                        *buffer_ptr = Vertex(xi, yi, zi, 5);
                        ++buffer_ptr;
                        ++vert_n;
                    }
                }
            }
        }
    }

    device.unmap_memory(vertex_staging_buffer);

    cmd_list.pipeline_barrier({
        .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
        .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
    });

    cmd_list.copy_buffer_to_buffer({
        .src_buffer = vertex_staging_buffer,
        .dst_buffer = vertex_buffer,
        .size = CHUNK_MAX_SIZE,
    });

    cmd_list.pipeline_barrier({
        .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
        .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ,
    });
}

struct App : AppWindow<App>
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = true,
    });
    daxa::Device device = daxa_ctx.create_default_device();

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window = get_native_handle(),
        .width = size_x,
        .height = size_y,
        .surface_format_selector = [](daxa::Format format)
        {
            switch (format)
            {
            case daxa::Format::R8G8B8A8_UINT: return 100;
            default: return daxa::default_format_score(format);
            }
        },
        .present_mode = daxa::PresentMode::DO_NOT_WAIT_FOR_VBLANK,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .debug_name = "HelloTriangle Swapchain",
    });

    daxa::ImageId depth_image = device.create_image({
        .format = daxa::Format::D32_SFLOAT,
        .aspect = daxa::ImageAspectFlagBits::DEPTH,
        .size = {size_x, size_y, 1},
        .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
    });

    daxa::PipelineCompiler pipeline_compiler = device.create_pipeline_compiler({
        .root_paths = {
            "tests/3_samples/0_playground/shaders",
            "include",
        },
        .debug_name = "HelloTriangle Compiler",
    });

    // clang-format off
    daxa::RasterPipeline raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"vert.hlsl"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"frag.hlsl"}},
        .color_attachments = {{.format = swapchain.get_format()}},
        .depth_test = {
            .depth_attachment_format = daxa::Format::D32_SFLOAT,
            .enable_depth_test = true,
            .enable_depth_write = true,
        },
        .raster = {
            .face_culling = daxa::FaceCullFlagBits::BACK_BIT,
        },
        .push_constant_size = sizeof(RasterPush),
        .debug_name = "HelloTriangle Pipeline",
    }).value();
    // clang-format on

    daxa::BinarySemaphore binary_semaphore = device.create_binary_semaphore({
        .debug_name = "HelloTriangle Present Semaphore",
    });

    static inline constexpr u64 FRAMES_IN_FLIGHT = 1;
    daxa::TimelineSemaphore gpu_framecount_timeline_sema = device.create_timeline_semaphore(daxa::TimelineSemaphoreInfo{
        .initial_value = 0,
        .debug_name = "HelloTriangle gpu framecount Timeline Semaphore",
    });
    u64 cpu_framecount = FRAMES_IN_FLIGHT - 1;

    Clock::time_point start = Clock::now(), prev_time = start;

    RenderableVoxelWorld renderable_world{device};
    Player3D player = {
        .rot = {2.0f, 0.0f, 0.0f},
    };
    bool should_resize = false, paused = true;
    // daxa::BufferId raster_input_buffer = device.create_buffer({
    //     .size = sizeof(RasterInput),
    // });
    // RasterInput raster_input;

    App()
    {
    }

    ~App()
    {
        device.destroy_image(depth_image);
    }

    bool update()
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr))
        {
            return true;
        }

        if (!minimized)
        {
            draw();
        }
        else
        {
            using namespace std::literals;
            std::this_thread::sleep_for(1ms);
        }

        return false;
    }

    void draw()
    {
        auto now = Clock::now();
        auto elapsed_s = std::chrono::duration<f32>(now - prev_time).count();
        prev_time = now;

        player.camera.resize(static_cast<i32>(size_x), static_cast<i32>(size_y));
        player.camera.set_pos(player.pos);
        player.camera.set_rot(player.rot.x, player.rot.y);
        player.update(elapsed_s);

        if (pipeline_compiler.check_if_sources_changed(raster_pipeline))
        {
            auto new_pipeline = pipeline_compiler.recreate_raster_pipeline(raster_pipeline);
            if (new_pipeline.is_ok())
            {
                raster_pipeline = new_pipeline.value();
            }
        }

        if (should_resize)
        {
            do_resize();
        }

        auto swapchain_image = swapchain.acquire_next_image();

        auto cmd_list = device.create_command_list({
            .debug_name = "HelloTriangle Command List",
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
            .image_id = swapchain_image,
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::UNDEFINED,
            .after_layout = daxa::ImageLayout::DEPTH_ATTACHMENT_OPTIMAL,
            .image_id = depth_image,
            .image_slice = {.image_aspect = daxa::ImageAspectFlagBits::DEPTH},
        });

        cmd_list.begin_renderpass({
            .color_attachments = {{
                .image = swapchain_image,
                .load_op = daxa::AttachmentLoadOp::CLEAR,
                .clear_value = std::array<f32, 4>{0.2f, 0.4f, 1.0f, 1.0f},
            }},
            .depth_attachment = {{
                .image = depth_image,
                .load_op = daxa::AttachmentLoadOp::CLEAR,
                .clear_value = daxa::DepthValue{1.0f, 0},
            }},
            .render_area = {.x = 0, .y = 0, .width = size_x, .height = size_y},
        });

        cmd_list.set_pipeline(raster_pipeline);
        renderable_world.draw(cmd_list, player.camera.get_vp());
        cmd_list.end_renderpass();

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .before_layout = daxa::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
            .after_layout = daxa::ImageLayout::PRESENT_SRC,
            .image_id = swapchain_image,
        });

        cmd_list.complete();

        ++cpu_framecount;
        device.submit_commands({
            .command_lists = {std::move(cmd_list)},
            .signal_binary_semaphores = {binary_semaphore},
            .signal_timeline_semaphores = {{gpu_framecount_timeline_sema, cpu_framecount}},
        });

        device.present_frame({
            .wait_binary_semaphores = {binary_semaphore},
            .swapchain = swapchain,
        });

        gpu_framecount_timeline_sema.wait_for_value(cpu_framecount - 1);
        // printf("ahead: %llu\n", cpu_framecount - gpu_framecount_timeline_sema.value());
    }

    void on_mouse_move(f32 x, f32 y)
    {
        if (!paused)
        {
            f32 center_x = static_cast<f32>(size_x / 2);
            f32 center_y = static_cast<f32>(size_y / 2);
            auto offset = glm::vec2{x - center_x, center_y - y};
            player.on_mouse_move(static_cast<f64>(offset.x), static_cast<f64>(offset.y));
            set_mouse_pos(center_x, center_y);
        }
    }

    void on_key(int key, int action)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            toggle_pause();
        }

        if (!paused)
        {
            player.on_key(key, action);
        }
    }

    void on_resize(u32 sx, u32 sy)
    {
        size_x = sx;
        size_y = sy;
        minimized = (sx == 0 || sy == 0);

        if (!minimized)
        {
            should_resize = true;
            do_resize();
        }
    }

    void do_resize()
    {
        should_resize = false;
        device.destroy_image(depth_image);
        depth_image = device.create_image({
            .format = daxa::Format::D32_SFLOAT,
            .aspect = daxa::ImageAspectFlagBits::DEPTH,
            .size = {size_x, size_y, 1},
            .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
        });
        swapchain.resize(size_x, size_y);
        draw();
    }

    void toggle_pause()
    {
        set_mouse_capture(paused);
        paused = !paused;
    }
};

int main()
{
    App app = {};
    while (true)
    {
        if (app.update())
            break;
    }
}
