#include <0_common/window.hpp>
#include <0_common/player.hpp>
#include <thread>
#include <iostream>

#include "util.hpp"

using namespace daxa::types;
using Clock = std::chrono::high_resolution_clock;

struct Vertex
{
    u32 data;
    Vertex(i32 x, i32 y, i32 z, u32 side)
    {
        data = 0;
        data |= (static_cast<u32>(x) & 0x1f) << 0x00;
        data |= (static_cast<u32>(y) & 0x1f) << 0x05;
        data |= (static_cast<u32>(z) & 0x1f) << 0x0a;
        data |= side << 0x0f;
    }
};

struct RasterPush
{
    glm::mat4 view_mat;
    glm::vec3 chunk_pos;
    daxa::BufferId vertex_buffer_id;
    u32 mode;
};

static inline constexpr u64 CHUNK_SIZE = 32;
static inline constexpr u64 CHUNK_VOXEL_N = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
static inline constexpr u64 CHUNK_MAX_VERTS = CHUNK_VOXEL_N * 6;
static inline constexpr u64 CHUNK_MAX_SIZE = CHUNK_MAX_VERTS * sizeof(Vertex);

static inline constexpr u64 CHUNK_N = 4;

struct Voxel
{
    u32 id;

    bool is_occluding()
    {
        switch (id)
        {
        case 0:
        case 2: return false;
        default: return true;
        }
    }

    bool is_occluding_nx() { return is_occluding(); }
    bool is_occluding_px() { return is_occluding(); }
    bool is_occluding_ny() { return is_occluding(); }
    bool is_occluding_py() { return is_occluding(); }
    bool is_occluding_nz() { return is_occluding(); }
    bool is_occluding_pz() { return is_occluding(); }
};

struct VoxelChunk
{
    Voxel voxels[CHUNK_VOXEL_N];
    glm::vec3 pos;

    u32 generate_block_id(glm::vec3 p)
    {
        p = p * 0.5f;
        FractalNoiseConfig noise_conf = {
            /* .amplitude   = */ 1.0f,
            /* .persistance = */ 0.12f,
            /* .scale       = */ 0.03f,
            /* .lacunarity  = */ 4.7f,
            /* .octaves     = */ 2,
        };
        float val = fractal_noise(p + 100.0f, noise_conf);
        val = val - (-p.y + 30.0f) * 0.04f;
        val -= std::pow(smoothstep(-1.0f, 1.0f, -p.y + 32.0f), 2.0f) * 0.15f;
        val = std::max(val, 0.0f);
        if (val > 0.0f)
            return 1;
        if (p.y > 33.0f)
            return 2;
        return 0;
    }

    void generate()
    {
        for (u64 zi = 0; zi < 32; ++zi)
            for (u64 yi = 0; yi < 32; ++yi)
                for (u64 xi = 0; xi < 32; ++xi)
                {
                    u64 i = xi + yi * CHUNK_SIZE + zi * CHUNK_SIZE * CHUNK_SIZE;
                    glm::vec3 block_pos = glm::vec3(xi, yi, zi) + pos;
                    voxels[i].id = generate_block_id(block_pos);
                }
    }
};

struct RenderableVoxelWorld;

struct RenderableChunk
{
    VoxelChunk voxel_chunk = {};
    daxa::Device device;
    daxa::BufferId face_buffer;
    daxa::BufferId water_face_buffer;
    u32 face_n = 0, water_face_n = 0;
    RenderableVoxelWorld * renderable_world = nullptr;

    RenderableChunk(daxa::Device & a_device) : device{a_device}
    {
        face_buffer = device.create_buffer(daxa::BufferInfo{
            .size = CHUNK_MAX_SIZE,
            .debug_name = "Chunk Face buffer",
        });
        water_face_buffer = device.create_buffer(daxa::BufferInfo{
            .size = CHUNK_MAX_SIZE,
            .debug_name = "Chunk Water Face buffer",
        });
    }

    ~RenderableChunk()
    {
        device.destroy_buffer(face_buffer);
        device.destroy_buffer(water_face_buffer);
    }

    void update_chunk_mesh(daxa::CommandList & cmd_list);

    void draw(daxa::CommandList & cmd_list, glm::mat4 const & view_mat)
    {
        if (face_n > 0)
        {
            cmd_list.push_constant(RasterPush{
                .view_mat = view_mat,
                .chunk_pos = voxel_chunk.pos,
                .vertex_buffer_id = face_buffer,
                .mode = 0,
            });
            cmd_list.draw({.vertex_count = face_n * 6});
        }
    }
    void draw_water(daxa::CommandList & cmd_list, glm::mat4 const & view_mat)
    {
        if (water_face_n > 0)
        {
            cmd_list.push_constant(RasterPush{
                .view_mat = view_mat,
                .chunk_pos = voxel_chunk.pos,
                .vertex_buffer_id = water_face_buffer,
                .mode = 1,
            });
            cmd_list.draw({.vertex_count = water_face_n * 6});
        }
    }
};

struct RenderableVoxelWorld
{
    std::unique_ptr<RenderableChunk> chunks[CHUNK_N * CHUNK_N * CHUNK_N] = {};

    RenderableVoxelWorld(daxa::Device & device)
    {
        for (u64 z = 0; z < CHUNK_N; ++z)
        {
            for (u64 y = 0; y < CHUNK_N; ++y)
            {
                for (u64 x = 0; x < CHUNK_N; ++x)
                {
                    chunks[x + y * CHUNK_N + z * CHUNK_N * CHUNK_N] = std::make_unique<RenderableChunk>(device);
                    auto & chunk = *chunks[x + y * CHUNK_N + z * CHUNK_N * CHUNK_N];
                    chunk.voxel_chunk.pos = glm::vec3(static_cast<f32>(x * CHUNK_SIZE), static_cast<f32>(y * CHUNK_SIZE), static_cast<f32>(z * CHUNK_SIZE));
                    chunk.voxel_chunk.generate();
                    chunk.renderable_world = this;
                }
            }
        }

        for (u64 z = 0; z < CHUNK_N; ++z)
        {
            for (u64 y = 0; y < CHUNK_N; ++y)
            {
                for (u64 x = 0; x < CHUNK_N; ++x)
                {
                    auto & chunk = *chunks[x + y * CHUNK_N + z * CHUNK_N * CHUNK_N];
                    auto cmd_list = device.create_command_list({
                        .debug_name = "Playground Command List",
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
    }

    void draw(daxa::CommandList & cmd_list, glm::mat4 const & view_mat)
    {
        for (auto & chunk : chunks)
            chunk->draw(cmd_list, view_mat);
        for (auto & chunk : chunks)
            chunk->draw_water(cmd_list, view_mat);
    }

    Voxel get_voxel(glm::ivec3 p)
    {
        i32 x = p.x / static_cast<i32>(CHUNK_SIZE);
        i32 y = p.y / static_cast<i32>(CHUNK_SIZE);
        i32 z = p.z / static_cast<i32>(CHUNK_SIZE);

        if (p.x < 0 || x >= static_cast<i32>(CHUNK_N))
            return {.id = 0};
        if (p.y < 0 || y >= static_cast<i32>(CHUNK_N))
            return {.id = 0};
        if (p.z < 0 || z >= static_cast<i32>(CHUNK_N))
            return {.id = 0};

        auto & chunk = chunks[static_cast<u64>(x) + static_cast<u64>(y) * CHUNK_N + static_cast<u64>(z) * CHUNK_N * CHUNK_N];

        u64 xi = static_cast<u64>(p.x) % CHUNK_SIZE;
        u64 yi = static_cast<u64>(p.y) % CHUNK_SIZE;
        u64 zi = static_cast<u64>(p.z) % CHUNK_SIZE;
        u64 i = xi + yi * CHUNK_SIZE + zi * CHUNK_SIZE * CHUNK_SIZE;
        return chunk->voxel_chunk.voxels[i];
    }
};

void RenderableChunk::update_chunk_mesh(daxa::CommandList & cmd_list)
{
    {
        auto face_staging_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = CHUNK_MAX_SIZE,
            .debug_name = "Playground Vertex Staging buffer",
        });
        cmd_list.destroy_buffer_deferred(face_staging_buffer);

        Vertex * buffer_ptr = device.map_memory_as<Vertex>(face_staging_buffer);
        face_n = 0;
        for (i32 zi = 0; zi < 32; ++zi)
        {
            for (i32 yi = 0; yi < 32; ++yi)
            {
                for (i32 xi = 0; xi < 32; ++xi)
                {
                    glm::ivec3 voxel_pos = glm::ivec3{xi, yi, zi} + glm::ivec3(voxel_chunk.pos);
                    if (renderable_world->get_voxel(voxel_pos).is_occluding())
                    {
                        if (!renderable_world->get_voxel(voxel_pos + glm::ivec3(+1, 0, 0)).is_occluding_nx())
                            *buffer_ptr = Vertex(xi, yi, zi, 0), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + glm::ivec3(-1, 0, 0)).is_occluding_px())
                            *buffer_ptr = Vertex(xi, yi, zi, 1), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + glm::ivec3(0, +1, 0)).is_occluding_ny())
                            *buffer_ptr = Vertex(xi, yi, zi, 2), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + glm::ivec3(0, -1, 0)).is_occluding_py())
                            *buffer_ptr = Vertex(xi, yi, zi, 3), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + glm::ivec3(0, 0, +1)).is_occluding_nz())
                            *buffer_ptr = Vertex(xi, yi, zi, 4), ++buffer_ptr, ++face_n;
                        if (!renderable_world->get_voxel(voxel_pos + glm::ivec3(0, 0, -1)).is_occluding_pz())
                            *buffer_ptr = Vertex(xi, yi, zi, 5), ++buffer_ptr, ++face_n;
                    }
                }
            }
        }
        device.unmap_memory(face_staging_buffer);

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = face_staging_buffer,
            .dst_buffer = face_buffer,
            .size = CHUNK_MAX_SIZE,
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ,
        });
    }

    {
        auto face_staging_buffer = device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = CHUNK_MAX_SIZE,
            .debug_name = "Playground Vertex Staging buffer",
        });
        cmd_list.destroy_buffer_deferred(face_staging_buffer);

        Vertex * buffer_ptr = device.map_memory_as<Vertex>(face_staging_buffer);
        water_face_n = 0;
        for (i32 zi = 0; zi < 32; ++zi)
        {
            for (i32 yi = 0; yi < 32; ++yi)
            {
                for (i32 xi = 0; xi < 32; ++xi)
                {
                    glm::ivec3 voxel_pos = glm::ivec3{xi, yi, zi} + glm::ivec3(voxel_chunk.pos);
                    if (renderable_world->get_voxel(voxel_pos).id == 2)
                    {
                        if (renderable_world->get_voxel(voxel_pos + glm::ivec3(+1, 0, 0)).id == 0)
                            *buffer_ptr = Vertex(xi, yi, zi, 0), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + glm::ivec3(-1, 0, 0)).id == 0)
                            *buffer_ptr = Vertex(xi, yi, zi, 1), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + glm::ivec3(0, +1, 0)).id == 0)
                            *buffer_ptr = Vertex(xi, yi, zi, 2), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + glm::ivec3(0, -1, 0)).id == 0)
                            *buffer_ptr = Vertex(xi, yi, zi, 3), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + glm::ivec3(0, 0, +1)).id == 0)
                            *buffer_ptr = Vertex(xi, yi, zi, 4), ++buffer_ptr, ++water_face_n;
                        if (renderable_world->get_voxel(voxel_pos + glm::ivec3(0, 0, -1)).id == 0)
                            *buffer_ptr = Vertex(xi, yi, zi, 5), ++buffer_ptr, ++water_face_n;
                    }
                }
            }
        }
        device.unmap_memory(face_staging_buffer);

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = face_staging_buffer,
            .dst_buffer = water_face_buffer,
            .size = CHUNK_MAX_SIZE,
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::VERTEX_SHADER_READ,
        });
    }
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
        .present_mode = daxa::PresentMode::DOUBLE_BUFFER_WAIT_FOR_VBLANK,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .debug_name = "Playground Swapchain",
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
        .debug_name = "Playground Compiler",
    });

    // clang-format off
    daxa::RasterPipeline raster_pipeline = pipeline_compiler.create_raster_pipeline({
        .vertex_shader_info = {.source = daxa::ShaderFile{"vert.hlsl"}},
        .fragment_shader_info = {.source = daxa::ShaderFile{"frag.hlsl"}},
        .color_attachments = {{.format = swapchain.get_format(), .blend = {.blend_enable = true, .src_color_blend_factor = daxa::BlendFactor::SRC_ALPHA, .dst_color_blend_factor = daxa::BlendFactor::ONE_MINUS_SRC_ALPHA}}},
        .depth_test = {
            .depth_attachment_format = daxa::Format::D32_SFLOAT,
            .enable_depth_test = true,
            .enable_depth_write = true,
        },
        .raster = {
            .face_culling = daxa::FaceCullFlagBits::BACK_BIT,
        },
        .push_constant_size = sizeof(RasterPush),
        .debug_name = "Playground Pipeline",
    }).value();
    // clang-format on

    daxa::BinarySemaphore binary_semaphore = device.create_binary_semaphore({
        .debug_name = "Playground Present Semaphore",
    });

    static inline constexpr u64 FRAMES_IN_FLIGHT = 1;
    daxa::TimelineSemaphore gpu_framecount_timeline_sema = device.create_timeline_semaphore(daxa::TimelineSemaphoreInfo{
        .initial_value = 0,
        .debug_name = "Playground gpu framecount Timeline Semaphore",
    });
    u64 cpu_framecount = FRAMES_IN_FLIGHT - 1;

    Clock::time_point start = Clock::now(), prev_time = start;

    RenderableVoxelWorld renderable_world{device};
    Player3D player = {
        .rot = {2.0f, 0.0f, 0.0f},
    };
    bool should_resize = false, paused = true;

    App() : AppWindow<App>("Samples: Playground") {}

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
            else
            {
                std::cout << new_pipeline.message() << std::endl;
            }
        }

        if (should_resize)
        {
            do_resize();
        }

        auto swapchain_image = swapchain.acquire_next_image();

        auto cmd_list = device.create_command_list({
            .debug_name = "Playground Command List",
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
                .image_view = swapchain_image.default_view(),
                .load_op = daxa::AttachmentLoadOp::CLEAR,
                .clear_value = std::array<f32, 4>{0.2f, 0.4f, 1.0f, 1.0f},
            }},
            .depth_attachment = {{
                .image_view = depth_image.default_view(),
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
