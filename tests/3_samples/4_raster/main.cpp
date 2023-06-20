#include <0_common/player.hpp>
#include <0_common/voxels.hpp>
#include "shared.inl"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/math_operators.hpp>
#include <iostream>
#include <daxa/utils/task_list.hpp>

#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_NATIVE_INCLUDE_NONE
using HWND = void *;
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>

#include <set>

auto get_native_handle(GLFWwindow * glfw_window_ptr) -> daxa::NativeWindowHandle
{
#if defined(_WIN32)
    return glfwGetWin32Window(glfw_window_ptr);
#elif defined(__linux__)
    return reinterpret_cast<daxa::NativeWindowHandle>(glfwGetX11Window(glfw_window_ptr));
#elif defined(__APPLE__)
    return glfwGetCocoaWindow(glfw_window_ptr);
#else
    return {};
#endif
}

auto get_native_platform(GLFWwindow * /*unused*/) -> daxa::NativeWindowPlatform
{
#if defined(_WIN32)
    return daxa::NativeWindowPlatform::WIN32_API;
#elif defined(__linux__)
    return daxa::NativeWindowPlatform::XLIB_API;
#elif defined(__APPLE__)
    return daxa::NativeWindowPlatform::COCOA_API;
#else
    return daxa::NativeWindowPlatform::UNKNOWN;
#endif
}

struct AppInfo
{
    daxa::u32 width{}, height{};
    bool swapchain_out_of_date = false;

    bool is_paused = true;
    bool mouse_captured = false;

    Player3D player = {
        .rot = {2.0f, 0.0f, 0.0f},
    };
};

auto main() -> int
{
    auto app_info = AppInfo{.width = 800, .height = 600};

    auto renderable_world = RenderableWorld{};
    renderable_world.generate_chunks();

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto *glfw_window_ptr = glfwCreateWindow(
        static_cast<daxa::i32>(app_info.width),
        static_cast<daxa::i32>(app_info.height),
        "Daxa sample window name", nullptr, nullptr);
    glfwSetWindowUserPointer(glfw_window_ptr, &app_info);
    glfwSetWindowSizeCallback(
        glfw_window_ptr,
        [](GLFWwindow * glfw_window, int width, int height)
        {
            auto & app_info_ref = *reinterpret_cast<AppInfo *>(glfwGetWindowUserPointer(glfw_window));
            app_info_ref.swapchain_out_of_date = true;
            app_info_ref.width = static_cast<daxa::u32>(width);
            app_info_ref.height = static_cast<daxa::u32>(height);
        });
    glfwSetCursorPosCallback(
        glfw_window_ptr,
        [](GLFWwindow * glfw_window, double dx, double dy)
        {
            auto & app_info_ref = *reinterpret_cast<AppInfo *>(glfwGetWindowUserPointer(glfw_window));
            if (!app_info_ref.is_paused)
            {
                f32 const x = static_cast<f32>(dx);
                f32 const y = static_cast<f32>(dy);
                f32 const center_x = static_cast<f32>(app_info_ref.width / 2);
                f32 const center_y = static_cast<f32>(app_info_ref.height / 2);
                auto offset = f32vec2{x - center_x, center_y - y};
                app_info_ref.player.on_mouse_move(offset.x, offset.y);
                glfwSetCursorPos(glfw_window, static_cast<f64>(center_x), static_cast<f64>(center_y));
            }
        });

    glfwSetKeyCallback(
        glfw_window_ptr,
        [](GLFWwindow * glfw_window, i32 key_id, i32, i32 action, i32)
        {
            auto & app_info_ref = *reinterpret_cast<AppInfo *>(glfwGetWindowUserPointer(glfw_window));

            if (key_id == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                app_info_ref.is_paused = !app_info_ref.is_paused;
                auto should_capture = !app_info_ref.is_paused;
                if (app_info_ref.mouse_captured != should_capture)
                {
                    glfwSetCursorPos(glfw_window, static_cast<f64>(app_info_ref.width / 2), static_cast<f64>(app_info_ref.height / 2));
                    app_info_ref.mouse_captured = should_capture;
                }
                glfwSetInputMode(glfw_window, GLFW_CURSOR, should_capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
                glfwSetInputMode(glfw_window, GLFW_RAW_MOUSE_MOTION, static_cast<int>(should_capture));
            }

            if (!app_info_ref.is_paused)
            {
                app_info_ref.player.on_key(key_id, action);
            }
        });

    auto *native_window_handle = get_native_handle(glfw_window_ptr);
    auto native_window_platform = get_native_platform(glfw_window_ptr);

    daxa::Context context = daxa::create_context({});
    daxa::Device device = context.create_device({.name = "my device"});

    daxa::Swapchain swapchain = device.create_swapchain({
        .native_window = native_window_handle,
        .native_window_platform = native_window_platform,
        .present_mode = daxa::PresentMode::MAILBOX,
        .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
        .name = "my swapchain",
    });

    auto pipeline_manager = daxa::PipelineManager({
        .device = device,
        .shader_compile_options = {
            .root_paths = {
                DAXA_SHADER_INCLUDE_DIR,
                DAXA_SAMPLE_PATH,
                "tests/0_common/shaders",
                "tests/0_common/shared",
            },
            .language = daxa::ShaderLanguage::GLSL,
            .enable_debug_info = true,
        },
        .name = "my pipeline manager",
    });
    std::shared_ptr<daxa::RasterPipeline> pipeline;
    {
        auto result = pipeline_manager.add_raster_pipeline({
            .vertex_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"main.glsl"}},
            .fragment_shader_info = daxa::ShaderCompileInfo{.source = daxa::ShaderFile{"main.glsl"}},
            .color_attachments = {{.format = swapchain.get_format()}},
            .depth_test = {
                .depth_attachment_format = daxa::Format::D32_SFLOAT,
                .enable_depth_test = true,
                .enable_depth_write = true,
            },
            .raster = {
                .face_culling = daxa::FaceCullFlagBits::BACK_BIT,
            },
            .push_constant_size = sizeof(DrawPush),
            .name = "my pipeline",
        });
        if (result.is_err())
        {
            std::cerr << result.message() << std::endl;
            return -1;
        }
        pipeline = result.value();
    }

    auto perframe_input_buffer_id = device.create_buffer({
        .size = sizeof(PerframeInput),
        .name = "perframe_input_buffer_id",
    });

    auto depth_image = device.create_image({
        .format = daxa::Format::D32_SFLOAT,
        .size = {app_info.width, app_info.height, 1},
        .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT | daxa::ImageUsageFlagBits::SHADER_READ_ONLY,
        .name = "depth_image",
    });

    constexpr auto MIP_COUNT = 4u;

    auto atlas_texture_array = device.create_image({
        .format = daxa::Format::R8G8B8A8_SRGB,
        .size = {16, 16, 1},
        .mip_level_count = MIP_COUNT,
        .array_layer_count = static_cast<u32>(texture_names.size()),
        .usage = daxa::ImageUsageFlagBits::SHADER_READ_ONLY | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
        .name = "atlas_texture_array",
    });

    auto atlas_sampler = device.create_sampler({
        .magnification_filter = daxa::Filter::NEAREST,
        .minification_filter = daxa::Filter::LINEAR,
        .min_lod = 0,
        .max_lod = static_cast<f32>(MIP_COUNT - 1),
        .name = "atlas_sampler",
    });

    enum class TaskCondition
    {
        VERTICES_UPLOAD,
        TEXTURES_UPLOAD,
        COUNT,
    };
    std::array<bool, static_cast<daxa::usize>(TaskCondition::COUNT)> task_condition_states{};
    auto loop_task_list = daxa::TaskList({
        .device = device,
        .swapchain = swapchain,
        .permutation_condition_count = static_cast<daxa::usize>(TaskCondition::COUNT),
        .name = "my task list",
    });

    daxa::TaskImage task_swapchain_image{{.swapchain_image = true,.name = "swapchain image"}};
    daxa::TaskImage task_depth_image{{.initial_images={.images=std::array{depth_image}}, .name = "depth image"}};
    daxa::TaskImage task_atlas_texture_array{{.initial_images={.images=std::array{atlas_texture_array}}, .name = "atlas_texture_array"}};
    daxa::TaskBuffer task_buffer_per_frame{{.initial_buffers={.buffers=std::array{perframe_input_buffer_id}}, .name="task_buffer_per_frame"}};
    daxa::TaskBuffer task_buffer_renderable_chunks{{.name="task_buffer_renderable_chunks"}};

    auto chunk_update_queue = std::set<usize>{};
    auto chunk_update_queue_mtx = std::mutex{};

    loop_task_list.conditional({
        .condition_index = static_cast<daxa::u32>(TaskCondition::VERTICES_UPLOAD),
        .when_true = [&]()
        {
            loop_task_list.add_task({
                .uses = {{renderable_chunks_task_buffer_id, daxa::TaskBufferAccess::TRANSFER_WRITE}},
                .task = [&](daxa::TaskInterface task_runtime)
                {
                    auto cmd_list = task_runtime.get_command_list();
                    auto threads = std::vector<std::unique_ptr<std::thread>>{};
                    constexpr auto THREAD_N = 1;
                    threads.resize(THREAD_N);

                    for (u32 i = 0; i < THREAD_N; ++i)
                    {
                        threads[i] = std::make_unique<std::thread>(
                            [&]()
                            {
                                auto staging_buffer_id = device.create_buffer({
                                    .size = sizeof(VoxelFace) * 6 * CHUNK_VOXEL_N,
                                    .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                                    .name = "my staging buffer",
                                });
                                cmd_list.destroy_buffer_deferred(staging_buffer_id);
                                auto * buffer_ptr = device.get_host_address_as<VoxelFace>(staging_buffer_id);
                                usize chunk_index = 0;
                                {
                                    auto lock_guard = std::lock_guard{chunk_update_queue_mtx};
                                    auto chunk_iter = std::min_element(
                                        chunk_update_queue.begin(), chunk_update_queue.end(),
                                        [&](usize a, usize b)
                                        {
                                            using namespace daxa::math_operators;
                                            auto & rca = renderable_world.renderable_chunks[a];
                                            auto & rcb = renderable_world.renderable_chunks[b];
                                            auto const del_a = rca.chunk->pos * -1.0f - app_info.player.pos;
                                            auto const del_b = rcb.chunk->pos * -1.0f - app_info.player.pos;
                                            auto dist2_a = dot(del_a, del_a);
                                            auto dist2_b = dot(del_b, del_b);
                                            return dist2_a < dist2_b;
                                        });
                                    chunk_index = *chunk_iter;
                                    chunk_update_queue.erase(chunk_index);
                                }
                                auto & renderable_chunk = renderable_world.renderable_chunks[chunk_index];
                                if (!renderable_chunk.chunk->is_generated)
                                {
                                    renderable_chunk.chunk->generate();
                                    renderable_world.chunk_update(chunk_index);
                                }
                                if (!renderable_chunk.face_buffer.is_empty())
                                {
                                    device.destroy_buffer(renderable_chunk.face_buffer);
                                    renderable_chunk.face_buffer = {};
                                }
                                renderable_chunk.construct_mesh(
                                    buffer_ptr,
                                    [&renderable_world](i32vec3 p)
                                    {
                                        return renderable_world.get_voxel(p);
                                    });
                                if (renderable_chunk.face_n == 0)
                                {
                                    return;
                                }
                                renderable_chunk.face_buffer = device.create_buffer({
                                    .size = static_cast<u32>(sizeof(VoxelFace)) * renderable_chunk.face_n,
                                    .name = "chunk_buffer_id",
                                });
                                cmd_list.copy_buffer_to_buffer({
                                    .src_buffer = staging_buffer_id,
                                    .dst_buffer = renderable_chunk.face_buffer,
                                    .size = static_cast<u32>(sizeof(VoxelFace)) * renderable_chunk.face_n,
                                });
                            });
                    }

                    for (u32 i = 0; i < THREAD_N; ++i)
                    {
                        threads[i]->join();
                    }
                },
                .name = "my upload task",
            });
        },
    });

    loop_task_list.conditional({
        .condition_index = static_cast<daxa::u32>(TaskCondition::TEXTURES_UPLOAD),
        .when_true = [&]()
        {
            loop_task_list.add_task({
                .used_images = {
                    {task_atlas_texture_array, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{.base_array_layer = 0, .layer_count = static_cast<u32>(texture_names.size())}},
                },
                .task = [&](daxa::TaskInterface runtime)
                {
                    auto cmd_list = runtime.get_command_list();
                    load_textures_commands(device, cmd_list, runtime.get_images(task_atlas_texture_array)[0]);
                },
                .name = "Upload Textures",
            });

            i32 mip_size = 16;
            for (u32 i = 0; i < MIP_COUNT - 1; ++i)
            {
                i32 next_mip_size = std::max<i32>(1, mip_size / 2);
                loop_task_list.add_task({
                    .used_images = {
                        {task_atlas_texture_array, daxa::TaskImageAccess::TRANSFER_READ, daxa::ImageMipArraySlice{.base_mip_level = i + 0, .base_array_layer = 0, .layer_count = static_cast<u32>(texture_names.size())}},
                        {task_atlas_texture_array, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{.base_mip_level = i + 1, .base_array_layer = 0, .layer_count = static_cast<u32>(texture_names.size())}},
                    },
                    .task = [&, i, mip_size, next_mip_size](daxa::TaskInterface const & runtime)
                    {
                        auto cmd_list = runtime.get_command_list();
                        auto image_id = runtime.get_images(task_atlas_texture_array)[0];
                        cmd_list.blit_image_to_image({
                            .src_image = image_id,
                            .dst_image = image_id,
                            .src_slice = {
                                .mip_level = i,
                                .base_array_layer = 0,
                                .layer_count = static_cast<u32>(texture_names.size()),
                            },
                            .src_offsets = {{{0, 0, 0}, {mip_size, mip_size, 1}}},
                            .dst_slice = {
                                .mip_level = i + 1,
                                .base_array_layer = 0,
                                .layer_count = static_cast<u32>(texture_names.size()),
                            },
                            .dst_offsets = {{{0, 0, 0}, {next_mip_size, next_mip_size, 1}}},
                            .filter = daxa::Filter::LINEAR,
                        });
                        if (i == MIP_COUNT - 2)
                        {
                            task_condition_states[static_cast<daxa::usize>(TaskCondition::TEXTURES_UPLOAD)] = false;
                        }
                    },
                    .name = "Generate Texture Mips " + std::to_string(i),
                });
                mip_size = next_mip_size;
            }
        },
    });

    loop_task_list.add_task({
        .used_buffers = {
            {renderable_chunks_task_buffer_id, daxa::TaskBufferAccess::VERTEX_SHADER_READ},
        },
        .task = [&](daxa::TaskInterface task_runtime)
        {
            auto cmd_list = task_runtime.get_command_list();
            auto staging_input_buffer = device.create_buffer({
                .size = sizeof(PerframeInput),
                .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                .name = "staging_input_buffer",
            });
            cmd_list.destroy_buffer_deferred(staging_input_buffer);
            auto * buffer_ptr = device.get_host_address_as<PerframeInput>(staging_input_buffer);
            auto const vp_mat = app_info.player.camera.get_vp();
            buffer_ptr->vp_mat = daxa::math_operators::mat_from_span<f32 const, 4, 4>(std::span<f32 const, 16>{glm::value_ptr(vp_mat), 16});
            cmd_list.copy_buffer_to_buffer({
                .src_buffer = staging_input_buffer,
                .dst_buffer = task_runtime.get_buffers(perframe_input_task_buffer_id)[0],
                .size = sizeof(PerframeInput),
            });
        },
        .name = "my draw task",
    });

    loop_task_list.add_task({
        .used_buffers = {
            {renderable_chunks_task_buffer_id, daxa::TaskBufferAccess::VERTEX_SHADER_READ},
        },
        .used_images = {
            {task_swapchain_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
            {task_depth_image, daxa::TaskImageAccess::DEPTH_ATTACHMENT, daxa::ImageMipArraySlice{.image_aspect = daxa::ImageAspectFlagBits::DEPTH}},
        },
        .task = [&](daxa::TaskInterface task_runtime)
        {
            auto cmd_list = task_runtime.get_command_list();
            auto swapchain_image = task_runtime.get_images(task_swapchain_image)[0];
            auto depth_image = task_runtime.get_images(task_depth_image)[0];
            auto perframe_input_ptr = device.get_device_address(task_runtime.get_buffers(perframe_input_task_buffer_id)[0]);
            cmd_list.begin_renderpass({
                .color_attachments = {
                    {
                        .image_view = swapchain_image.default_view(),
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = std::array<daxa::f32, 4>{0.1f, 0.0f, 0.5f, 1.0f},
                    },
                },
                .depth_attachment = daxa::RenderAttachmentInfo{
                    .image_view = depth_image.default_view(),
                    .load_op = daxa::AttachmentLoadOp::CLEAR,
                    .clear_value = daxa::DepthValue{1.0f, 0u},
                },
                .render_area = {.x = 0, .y = 0, .width = app_info.width, .height = app_info.height},
            });
            cmd_list.set_pipeline(*pipeline);
            for (auto & renderable_chunk : renderable_world.renderable_chunks)
            {
                if (renderable_chunk.face_n == 0)
                {
                    continue;
                }
                cmd_list.push_constant(DrawPush{
                    .packed_faces_ptr = device.get_device_address(renderable_chunk.face_buffer),
                    .perframe_input_ptr = perframe_input_ptr,
                    .atlas_texture = { task_runtime.get_images(task_atlas_texture_array)[0].default_view() },
                    .atlas_sampler = atlas_sampler,
                    .chunk_pos = renderable_chunk.chunk->pos,
                });
                cmd_list.draw({.vertex_count = renderable_chunk.face_n * 6});
            }
            cmd_list.end_renderpass();
        },
        .name = "my draw task",
    });

    loop_task_list.submit({});
    loop_task_list.present({});
    loop_task_list.complete({});
    task_condition_states[static_cast<daxa::usize>(TaskCondition::TEXTURES_UPLOAD)] = true;

    using Clock = std::chrono::high_resolution_clock;
    auto start = Clock::now();
    auto prev_time = start;

    while (true)
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr) != 0)
        {
            break;
        }

        if (app_info.swapchain_out_of_date)
        {
            swapchain.resize();
            app_info.swapchain_out_of_date = false;
            loop_task_list.remove_runtime_image(task_depth_image, depth_image);

            auto depth_image_info = device.info_image(depth_image);
            device.destroy_image(depth_image);
            depth_image_info.size = {app_info.width, app_info.height, 1};
            depth_image = device.create_image(depth_image_info);

            loop_task_list.add_runtime_image(task_depth_image, depth_image);
        }

        loop_task_list.remove_runtime_image(task_swapchain_image, swapchain_image);
        swapchain_image = swapchain.acquire_next_image();
        loop_task_list.add_runtime_image(task_swapchain_image, swapchain_image);
        if (swapchain_image.is_empty())
        {
            continue;
        }

        auto now = Clock::now();
        auto elapsed = now - prev_time;
        prev_time = now;
        auto dt = std::chrono::duration<f32>(elapsed).count();

        app_info.player.update(dt);
        app_info.player.camera.resize(static_cast<i32>(app_info.width), static_cast<i32>(app_info.height));
        app_info.player.camera.set_rot(app_info.player.rot.x, app_info.player.rot.y);
        app_info.player.camera.set_pos(app_info.player.pos);

        for (usize i = 0; i < renderable_world.renderable_chunks.size(); ++i)
        {
            auto & renderable_chunk = renderable_world.renderable_chunks[i];
            if (renderable_chunk.mesh_invalid)
            {
                chunk_update_queue.insert(i);
            }
        }

        task_condition_states[static_cast<daxa::usize>(TaskCondition::VERTICES_UPLOAD)] = !chunk_update_queue.empty();
        loop_task_list.execute({.permutation_condition_values = task_condition_states});
    }

    device.wait_idle();
    device.collect_garbage();
    for (auto & renderable_chunk : renderable_world.renderable_chunks)
    {
        if (!renderable_chunk.face_buffer.is_empty())
        {
            device.destroy_buffer(renderable_chunk.face_buffer);
        }
    }
    device.destroy_buffer(perframe_input_buffer_id);
    device.destroy_image(depth_image);
    device.destroy_image(atlas_texture_array);
    device.destroy_sampler(atlas_sampler);
}
