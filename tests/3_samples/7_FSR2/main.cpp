#include <0_common/player.hpp>
#include <0_common/voxels.hpp>
#include "shared.inl"

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/math_operators.hpp>
#include <iostream>
#include <daxa/utils/task_list.hpp>
#include <daxa/utils/fsr2.hpp>

#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
using HWND = void *;
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

#include <set>

auto get_native_handle(GLFWwindow * glfw_window_ptr) -> daxa::NativeWindowHandle
{
#if defined(_WIN32)
    return glfwGetWin32Window(glfw_window_ptr);
#elif defined(__linux__)
    return reinterpret_cast<daxa::NativeWindowHandle>(glfwGetX11Window(glfw_window_ptr));
#elif defined(__APPLE__)
    return glfw_window_ptr;
    // return glfwGetCocoaWindow(glfw_window_ptr);
#endif
}

auto get_native_platform(GLFWwindow * /*unused*/) -> daxa::NativeWindowPlatform
{
#if defined(_WIN32)
    return daxa::NativeWindowPlatform::WIN32_API;
#elif defined(__linux__)
    return daxa::NativeWindowPlatform::XLIB_API;
#endif
}

struct AppInfo
{
    daxa::u32 width{}, height{};
    bool swapchain_out_of_date = false;

    bool is_paused = true;
    bool mouse_captured = false;
    f32 dt = 0.0f;

    f32mat4x4 view_mat{};

    Player3D player = {
        .pos = {-50.0f, -50.0f, -50.0f},
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

    auto upscale_context = daxa::Fsr2Context({.device = device});

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
            .color_attachments = {
                {.format = daxa::Format::R16G16B16A16_SFLOAT},
                {.format = daxa::Format::R16G16_SFLOAT},
            },
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

    constexpr auto RENDER_SCL = 2u;
    auto render_size = u32vec2{app_info.width / RENDER_SCL, app_info.height / RENDER_SCL};

    auto color_image = device.create_image({
        .format = daxa::Format::R16G16B16A16_SFLOAT,
        .aspect = daxa::ImageAspectFlagBits::COLOR,
        .size = {render_size.x, render_size.y, 1},
        .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::SHADER_READ_ONLY | daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .name = "color_image",
    });
    auto display_image = device.create_image({
        .format = daxa::Format::R16G16B16A16_SFLOAT,
        .aspect = daxa::ImageAspectFlagBits::COLOR,
        .size = {app_info.width, app_info.height, 1},
        .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::SHADER_READ_ONLY | daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
        .name = "display_image",
    });
    auto motion_vectors_image = device.create_image({
        .format = daxa::Format::R16G16_SFLOAT,
        .aspect = daxa::ImageAspectFlagBits::COLOR,
        .size = {render_size.x, render_size.y, 1},
        .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::SHADER_READ_ONLY | daxa::ImageUsageFlagBits::SHADER_READ_WRITE,
        .name = "motion_vectors_image",
    });
    auto depth_image = device.create_image({
        .format = daxa::Format::D32_SFLOAT,
        .aspect = daxa::ImageAspectFlagBits::DEPTH,
        .size = {render_size.x, render_size.y, 1},
        .usage = daxa::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT | daxa::ImageUsageFlagBits::SHADER_READ_ONLY,
        .name = "depth_image",
    });

    constexpr auto MIP_COUNT = 2u;

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
        .max_lod = 0, // static_cast<f32>(MIP_COUNT - 1),
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

    auto swapchain_image = daxa::ImageId{};
    auto task_swapchain_image = loop_task_list.create_transient_task_image({.swapchain_image = true, .name = "swapchain image"});
    loop_task_list.add_runtime_image(task_swapchain_image, swapchain_image);

    auto task_color_image = loop_task_list.create_transient_task_image({.name = "color_image"});
    auto task_display_image = loop_task_list.create_transient_task_image({.name = "display_image"});
    auto task_motion_vectors_image = loop_task_list.create_transient_task_image({.name = "motion_vectors_image"});
    auto task_depth_image = loop_task_list.create_transient_task_image({.name = "depth_image"});
    loop_task_list.add_runtime_image(task_color_image, color_image);
    loop_task_list.add_runtime_image(task_display_image, display_image);
    loop_task_list.add_runtime_image(task_motion_vectors_image, motion_vectors_image);
    loop_task_list.add_runtime_image(task_depth_image, depth_image);

    auto task_atlas_texture_array = loop_task_list.create_transient_task_image({.name = "task_atlas_texture_array"});
    loop_task_list.add_runtime_image(task_atlas_texture_array, atlas_texture_array);

    auto perframe_input_task_buffer_id = loop_task_list.create_transient_task_buffer({ .name = "perframe_input"});
    loop_task_list.add_runtime_buffer(perframe_input_task_buffer_id, perframe_input_buffer_id);
    auto renderable_chunks_task_buffer_id = loop_task_list.create_transient_task_buffer({ .name = "renderable_chunks"});

    auto chunk_update_queue = std::set<usize>{};
    auto chunk_update_queue_mtx = std::mutex{};

    loop_task_list.conditional({
        .condition_index = static_cast<daxa::u32>(TaskCondition::VERTICES_UPLOAD),
        .when_true = [&]()
        {
            loop_task_list.add_task({
                .used_buffers = {{renderable_chunks_task_buffer_id, daxa::TaskBufferAccess::TRANSFER_WRITE}},
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
                                if (chunk_update_queue.empty())
                                {
                                    return;
                                }

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
                i32 next_mip_size = std::max<i32>(1, static_cast<u32>(mip_size) / RENDER_SCL);
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

    auto jitter = f32vec2{0.0f, 0.0f};
    usize cpu_framecount = 0;

    upscale_context.resize({
        .render_size_x = render_size.x,
        .render_size_y = render_size.y,
        .display_size_x = app_info.width,
        .display_size_y = app_info.height,
    });

    loop_task_list.add_task({
        .used_buffers = {
            {renderable_chunks_task_buffer_id, daxa::TaskBufferAccess::VERTEX_SHADER_READ},
        },
        .task = [&](daxa::TaskInterface task_runtime)
        {
            using namespace daxa::math_operators;
            auto cmd_list = task_runtime.get_command_list();
            auto staging_input_buffer = device.create_buffer({
                .size = sizeof(PerframeInput),
                .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                .name = "staging_input_buffer",
            });
            cmd_list.destroy_buffer_deferred(staging_input_buffer);

            auto prev_jitter = jitter;
            jitter = upscale_context.get_jitter(cpu_framecount);
            auto jitter_vec = f32vec2{
                jitter.x * 2.0f / static_cast<f32>(render_size.x),
                jitter.y * 2.0f / static_cast<f32>(render_size.y),
            };

            auto * buffer_ptr = device.get_host_address_as<PerframeInput>(staging_input_buffer);

            buffer_ptr->prev_view_mat = app_info.view_mat;
            auto view_mat = app_info.player.camera.get_vp();
            view_mat = glm::translate(glm::identity<glm::mat4>(), glm::vec3(jitter_vec.x, jitter_vec.y, 0.0f)) * view_mat;
            app_info.view_mat = mat_from_span<f32, 4, 4>(std::span<f32, 4 * 4>{glm::value_ptr(view_mat), 4 * 4});
            buffer_ptr->view_mat = app_info.view_mat;
            buffer_ptr->jitter = (jitter - prev_jitter) * f32vec2{2.0f / static_cast<f32>(render_size.x), 2.0f / static_cast<f32>(render_size.y)};

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
            {task_color_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
            {task_motion_vectors_image, daxa::TaskImageAccess::COLOR_ATTACHMENT, daxa::ImageMipArraySlice{}},
            {task_depth_image, daxa::TaskImageAccess::DEPTH_ATTACHMENT, daxa::ImageMipArraySlice{.image_aspect = daxa::ImageAspectFlagBits::DEPTH}},
        },
        .task = [&](daxa::TaskInterface task_runtime)
        {
            auto cmd_list = task_runtime.get_command_list();
            auto color_image = task_runtime.get_images(task_color_image)[0];
            auto depth_image = task_runtime.get_images(task_depth_image)[0];
            auto motion_vectors_image = task_runtime.get_images(task_motion_vectors_image)[0];
            auto perframe_input_ptr = device.get_device_address(task_runtime.get_buffers(perframe_input_task_buffer_id)[0]);
            cmd_list.begin_renderpass({
                .color_attachments = {
                    {
                        .image_view = color_image.default_view(),
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = std::array<daxa::f32, 4>{0.1f, 0.0f, 0.5f, 1.0f},
                    },
                    {
                        .image_view = motion_vectors_image.default_view(),
                        .load_op = daxa::AttachmentLoadOp::CLEAR,
                        .clear_value = std::array<f32, 4>{0.0f, 0.0f, 0.0f, 0.0f},
                    },
                },
                .depth_attachment = daxa::RenderAttachmentInfo{
                    .image_view = depth_image.default_view(),
                    .load_op = daxa::AttachmentLoadOp::CLEAR,
                    .clear_value = daxa::DepthValue{1.0f, 0u},
                },
                .render_area = {.x = 0, .y = 0, .width = render_size.x, .height = render_size.y},
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

    loop_task_list.add_task({
        .used_images = {
            {task_color_image, daxa::TaskImageAccess::SHADER_READ, daxa::ImageMipArraySlice{}},
            {task_motion_vectors_image, daxa::TaskImageAccess::SHADER_READ, daxa::ImageMipArraySlice{}},
            {task_depth_image, daxa::TaskImageAccess::SHADER_READ, daxa::ImageMipArraySlice{.image_aspect = daxa::ImageAspectFlagBits::DEPTH}},
            {task_display_image, daxa::TaskImageAccess::SHADER_WRITE, daxa::ImageMipArraySlice{}},
        },
        .task = [&](daxa::TaskInterface runtime)
        {
            auto cmd_list = runtime.get_command_list();
            upscale_context.upscale(
                cmd_list,
                {
                    .color = color_image,
                    .depth = depth_image,
                    .motion_vectors = motion_vectors_image,
                    .output = display_image,
                    .should_reset = false,
                    .delta_time = app_info.dt,
                    .jitter = jitter,
                    .should_sharpen = false,
                    .sharpening = 0.0f,
                    .camera_info = {
                        .near_plane = app_info.player.camera.near_clip,
                        .far_plane = app_info.player.camera.far_clip,
                        .vertical_fov = glm::radians(app_info.player.camera.fov),
                    },
                });
        },
        .name = "Upscale Task",
    });
    loop_task_list.add_task({
        .used_images = {
            {task_display_image, daxa::TaskImageAccess::TRANSFER_READ, daxa::ImageMipArraySlice{}},
            {task_swapchain_image, daxa::TaskImageAccess::TRANSFER_WRITE, daxa::ImageMipArraySlice{}},
        },
        .task = [&](daxa::TaskInterface runtime)
        {
            auto size_x = app_info.width;
            auto size_y = app_info.height;
            auto cmd_list = runtime.get_command_list();
            cmd_list.blit_image_to_image({
                .src_image = runtime.get_images(task_display_image)[0],
                .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                .dst_image = runtime.get_images(task_swapchain_image)[0],
                .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .src_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                .src_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                .dst_slice = {.image_aspect = daxa::ImageAspectFlagBits::COLOR},
                .dst_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
            });
        },
        .name = "Blit Task (display to swapchain)",
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
            render_size = {app_info.width / RENDER_SCL, app_info.height / RENDER_SCL};
            upscale_context.resize({
                .render_size_x = render_size.x,
                .render_size_y = render_size.y,
                .display_size_x = app_info.width,
                .display_size_y = app_info.height,
            });
            swapchain.resize();
            app_info.swapchain_out_of_date = false;
            loop_task_list.remove_runtime_image(task_color_image, color_image);
            loop_task_list.remove_runtime_image(task_display_image, display_image);
            loop_task_list.remove_runtime_image(task_motion_vectors_image, motion_vectors_image);
            loop_task_list.remove_runtime_image(task_depth_image, depth_image);
            auto color_image_info = device.info_image(color_image);
            auto display_image_info = device.info_image(display_image);
            auto motion_vectors_image_info = device.info_image(motion_vectors_image);
            auto depth_image_info = device.info_image(depth_image);
            color_image_info.size = {render_size.x, render_size.y, 1};
            display_image_info.size = {app_info.width, app_info.height, 1};
            motion_vectors_image_info.size = {render_size.x, render_size.y, 1};
            depth_image_info.size = {render_size.x, render_size.y, 1};
            device.destroy_image(color_image);
            device.destroy_image(display_image);
            device.destroy_image(motion_vectors_image);
            device.destroy_image(depth_image);
            color_image = device.create_image(color_image_info);
            display_image = device.create_image(display_image_info);
            motion_vectors_image = device.create_image(motion_vectors_image_info);
            depth_image = device.create_image(depth_image_info);
            loop_task_list.add_runtime_image(task_color_image, color_image);
            loop_task_list.add_runtime_image(task_display_image, display_image);
            loop_task_list.add_runtime_image(task_motion_vectors_image, motion_vectors_image);
            loop_task_list.add_runtime_image(task_depth_image, depth_image);
        }

        pipeline_manager.reload_all();

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
        app_info.dt = std::chrono::duration<f32>(elapsed).count();

        app_info.player.update(app_info.dt);
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
        cpu_framecount++;
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
    device.destroy_image(color_image);
    device.destroy_image(display_image);
    device.destroy_image(motion_vectors_image);
    device.destroy_image(depth_image);
    device.destroy_image(atlas_texture_array);
    device.destroy_sampler(atlas_sampler);
}
