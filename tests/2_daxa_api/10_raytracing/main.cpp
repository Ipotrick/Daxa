#include <iostream>

#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_graph.hpp>

#include <0_common/window.hpp>
#include <0_common/shared.hpp>

#include "shaders/shared.inl"

namespace tests
{
    void ray_querry_triangle()
    {
        struct App : AppWindow<App>
        {
            daxa::Instance daxa_ctx = {};
            daxa::Device device = {};
            daxa::Swapchain swapchain = {};
            daxa::TlasId tlas = {};
            daxa::BlasId blas = {};

            App() : AppWindow<App>("ray query test") {}

            ~App()
            {
                if (device.is_valid() && device.is_id_valid(tlas))
                {
                    device.destroy_tlas(tlas);
                }
                if (device.is_valid() && device.is_id_valid(blas))
                {
                    device.destroy_blas(blas);
                }
            }

            void initialize()
            {
                daxa_ctx = daxa::create_instance({});
                device = daxa_ctx.create_device({
                    .selector = [](daxa::DeviceProperties const & prop) -> i32
                    {
                        auto default_value = daxa::default_device_score(prop);
                        return prop.ray_tracing_properties.has_value() ? default_value : -1;
                    },
                    .flags = daxa::DeviceFlagBits::RAY_TRACING,
                });
                swapchain = device.create_swapchain({
                    .native_window = get_native_handle(),
                    .native_window_platform = get_native_platform(),
                    .image_usage = daxa::ImageUsageFlagBits::TRANSFER_DST,
                });

                /// Prepare mesh data:
                auto vertices = std::array{
                    std::array{0.25, 0.75, 0.5},
                    std::array{0.5, 0.25, 0.5},
                    std::array{0.75, 0.75, 0.5},
                };
                auto vertex_buffer = device.create_buffer({
                    .size = sizeof(decltype(vertices)),
                    .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .name = "vertex buffer",
                });
                defer { device.destroy_buffer(vertex_buffer); };
                std::memcpy(device.get_host_address(vertex_buffer).value(), &vertices, sizeof(decltype(vertices)));
                auto indices = std::array{0, 1, 2};
                auto index_buffer = device.create_buffer({
                    .size = sizeof(decltype(indices)),
                    .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .name = "index buffer",
                });
                defer { device.destroy_buffer(index_buffer); };
                std::memcpy(device.get_host_address(index_buffer).value(), &indices, sizeof(decltype(indices)));
                auto transform = daxa_f32mat3x4{
                    {1, 0, 0},
                    {0, 1, 0},
                    {0, 0, 1},
                    {0, 0, 0},
                };
                auto transform_buffer = device.create_buffer({
                    .size = sizeof(daxa_f32mat3x4),
                    .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .name = "transform buffer",
                });
                defer { device.destroy_buffer(transform_buffer); };
                std::memcpy(device.get_host_address(transform_buffer).value(), &transform, sizeof(daxa_f32mat3x4));
                /// Write As description data:
                auto geometries = std::array{
                    daxa::BlasTriangleGeometryInfo{
                        .vertex_format = daxa::Format::R32G32B32_SFLOAT,
                        .vertex_data = {}, // Ignored in get_acceleration_structure_build_sizes.
                        .vertex_stride = sizeof(daxa_f32vec3),
                        .max_vertex = static_cast<u32>(vertices.size() - 1),
                        .index_type = daxa::IndexType::uint32,
                        .index_data = {},     // Ignored in get_acceleration_structure_build_sizes.
                        .transform_data = {}, // Ignored in get_acceleration_structure_build_sizes.
                        .count = 1,
                        .flags = {},
                    }};
                auto blas_build_info = daxa::BlasBuildInfo{
                    .dst_blas = {}, // Ignored in get_acceleration_structure_build_sizes.
                    .geometries = geometries,
                    .scratch_data = {}, // Ignored in get_acceleration_structure_build_sizes.
                };
                /// Query As sizes:
                daxa::AccelerationStructureBuildSizesInfo build_size_info = device.get_blas_build_sizes(blas_build_info);
                /// Create Scratch buffer and As:
                auto blas_scratch_buffer = device.create_buffer({
                    .size = build_size_info.build_scratch_size,
                    .name = "blas scratch buffer",
                });
                defer { device.destroy_buffer(blas_scratch_buffer); };
                this->blas = device.create_blas({
                    .size = build_size_info.acceleration_structure_size,
                    .name = "test blas",
                });
                /// Fill the remaining fields of the build info:
                auto & tri_geom = geometries[0];
                tri_geom.vertex_data = device.get_device_address(vertex_buffer).value();
                tri_geom.index_data = device.get_device_address(index_buffer).value();
                tri_geom.transform_data = device.get_device_address(transform_buffer).value();
                blas_build_info.dst_blas = blas;
                blas_build_info.scratch_data = device.get_device_address(blas_scratch_buffer).value();
                auto blas_instances_buffer = device.create_buffer({
                    .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .size = sizeof(daxa_BlasInstanceData),
                    .name = "blas instances array buffer",
                });
                defer { device.destroy_buffer(blas_instances_buffer); };
                *device.get_host_address_as<daxa_BlasInstanceData>(blas_instances_buffer).value() = daxa_BlasInstanceData{
                    .transform = {
                        {1, 0, 0},
                        {0, 1, 0},
                        {1, 0, 1},
                        {0, 0, 0},
                    },
                    .instance_custom_index = 5, // Set in order to show use later.
                    .mask = 0xFF,
                    .instance_shader_binding_table_record_offset = {}, // Not Used.
                    .flags = {},                                       // Not used.
                    .blas_device_address = device.get_device_address(blas).value(),
                };
                /// Build tlas:
                /// Get build sizes:
                auto blas_instances = std::array{
                    daxa::TlasInstanceInfo{
                        .data = {}, // Ignored for now.
                        .count = 1,
                        .is_data_array_of_pointers = false, // Buffer contains flat array of instances, not an array of pointers to instances.
                        .flags = {},                        // Unused,
                    },
                };
                auto tlas_build_info = daxa::TlasBuildInfo{
                    .dst_tlas = {}, // Ignored in get_acceleration_structure_build_sizes.
                    .instances = blas_instances,
                    .scratch_data = {}, // Ignored in get_acceleration_structure_build_sizes.
                };
                daxa::AccelerationStructureBuildSizesInfo tlas_build_sizes = device.get_tlas_build_sizes(tlas_build_info);
                /// Create Tlas:
                this->tlas = device.create_tlas({
                    .size = tlas_build_sizes.acceleration_structure_size,
                    .name = "tlas",
                });
                /// Create Build Scratch buffer
                auto tlas_scratch_buffer = device.create_buffer({
                    .size = build_size_info.build_scratch_size,
                    .name = "tlas scratch buffer",
                });
                defer { device.destroy_buffer(tlas_scratch_buffer); };
                /// Update build info:
                tlas_build_info.dst_tlas = tlas;
                tlas_build_info.scratch_data = device.get_device_address(tlas_scratch_buffer).value();
                blas_instances[0].data = device.get_device_address(blas_instances_buffer).value();
                /// Record build commands:
                auto exec_cmds = [&]()
                {
                    auto recorder = device.create_command_recorder({});
                    recorder.build_acceleration_structures({
                        .blas_build_infos = std::array{blas_build_info},
                    });
                    recorder.pipeline_barrier({
                        .src_access = daxa::AccessConsts::ACCELERATION_STRUCTURE_BUILD_WRITE,
                        .dst_access = daxa::AccessConsts::ACCELERATION_STRUCTURE_BUILD_READ_WRITE,
                    });
                    recorder.build_acceleration_structures({
                        .tlas_build_infos = std::array{tlas_build_info},
                    });
                    return recorder.complete_current_commands();

                }();
                device.submit_commands({.command_lists = std::array{exec_cmds}});
                device.wait_idle();
            }

            auto update() -> bool
            {
                glfwPollEvents();
                if (glfwWindowShouldClose(glfw_window_ptr) != 0)
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
                auto swapchain_image = swapchain.acquire_next_image();
                if (swapchain_image.is_empty())
                {
                    return;
                }
                auto recorder = device.create_command_recorder({
                    .name = ("recorder (clearcolor)"),
                });

                recorder.pipeline_barrier_image_transition({
                    .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .src_layout = daxa::ImageLayout::UNDEFINED,
                    .dst_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .image_id = swapchain_image,
                });

                recorder.clear_image({
                    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .clear_value = {std::array<f32, 4>{1, 0, 1, 1}},
                    .dst_image = swapchain_image,
                });

                recorder.pipeline_barrier_image_transition({
                    .src_access = daxa::AccessConsts::TRANSFER_WRITE,
                    .src_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .dst_layout = daxa::ImageLayout::PRESENT_SRC,
                    .image_id = swapchain_image,
                });

                auto executalbe_commands = recorder.complete_current_commands();
                recorder.~CommandRecorder();

                device.submit_commands({
                    .command_lists = std::array{executalbe_commands},
                    .wait_binary_semaphores = std::array{swapchain.get_acquire_semaphore()},
                    .signal_binary_semaphores = std::array{swapchain.get_present_semaphore()},
                    .signal_timeline_semaphores = std::array{std::pair{swapchain.get_gpu_timeline_semaphore(), swapchain.get_cpu_timeline_value()}},
                });

                device.present_frame({
                    .wait_binary_semaphores = std::array{swapchain.get_present_semaphore()},
                    .swapchain = swapchain,
                });

                device.collect_garbage();
            }

            void on_mouse_move(f32 /*unused*/, f32 /*unused*/) {}
            void on_mouse_button(i32 /*unused*/, i32 /*unused*/) {}
            void on_key(i32 /*unused*/, i32 /*unused*/) {}

            void on_resize(u32 sx, u32 sy)
            {
                minimized = sx == 0 || sy == 0;
                if (!minimized)
                {
                    swapchain.resize();
                    size_x = swapchain.get_surface_extent().x;
                    size_y = swapchain.get_surface_extent().y;
                    draw();
                }
            }
        };

        App app;
        try
        {
            app.initialize();
        }
        catch (std::runtime_error err)
        {
            std::cout << "No Raytracing capable gpu found. Skipped ray query test." << std::endl;
            return;
        }
        while (true)
        {
            if (app.update())
            {
                break;
            }
        }
    }
} // namespace tests

auto main() -> int
{
    tests::ray_querry_triangle();
    return 0;
}
