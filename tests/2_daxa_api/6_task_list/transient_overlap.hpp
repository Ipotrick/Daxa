#pragma once

#include "common.hpp"
#include "shaders/transient.inl"

namespace tests
{
    auto div_round_up(u32 value, u32 div) -> u32
    {
        return (value + div - 1) / div;
    }

    void set_initial_buffer_data(
        daxa::TaskInterface<> & tri,
        daxa::CommandList & cmd,
        daxa::TaskBufferId buffer,
        u32 size,
        u32 value)
    {
        auto staging = tri.get_allocator().allocate(size * sizeof(u32)).value();
        for (u32 x = 0; x < size; ++x)
        {
            reinterpret_cast<u32 *>(staging.host_address)[x] = value;
        }
        cmd.copy_buffer_to_buffer({
            .src_buffer = tri.get_allocator().get_buffer(),
            .src_offset = staging.buffer_offset,
            .dst_buffer = tri.get_buffers(buffer)[0],
            .size = size * sizeof(u32),
        });
    }

    void validate_buffer_data(
        daxa::TaskInterface<> & tri,
        daxa::CommandList & cmd,
        daxa::TaskBufferId buffer,
        u32 size,
        u32 value,
        daxa::ComputePipeline & pipeline)
    {
        cmd.set_pipeline(pipeline);
        cmd.push_constant(TestBufferPush{
            .test_buffer = tri.get_device().get_device_address(tri.get_buffers(buffer)[0]),
            .size = size,
            .value = value,
        });
        cmd.dispatch(div_round_up(size, 128));
    }

    void set_initial_image_data(
        daxa::TaskInterface<> & tri,
        daxa::CommandList & cmd,
        daxa::TaskImageId image,
        auto size,
        f32 value)
    {
        u32 const image_size = sizeof(f32) * size.x * size.y * size.z;
        auto staging = tri.get_allocator().allocate(image_size).value();
        for (u32 x = 0; x < size.x; ++x)
            for (u32 y = 0; y < size.y; ++y)
                for (u32 z = 0; z < size.z; ++z)
                {
                    usize index = (x + y * size.x + z * size.x * size.y);
                    reinterpret_cast<f32 *>(staging.host_address)[index] = value;
                }
        cmd.copy_buffer_to_image({
            .buffer = tri.get_allocator().get_buffer(),
            .buffer_offset = staging.buffer_offset,
            .image = tri.get_images(image)[0],
            .image_extent = {size.x, size.y, size.z},
        });
    }

    void validate_image_data(
        daxa::TaskInterface<> & tri,
        daxa::CommandList & cmd,
        daxa::TaskImageId image,
        auto size,
        f32 value,
        daxa::ComputePipeline & pipeline)
    {
        cmd.set_pipeline(pipeline);
        cmd.push_constant(TestImagePush{
            .test_image = {tri.get_image_views(image)[0]},
            .size = size,
            .value = value,
        });
        cmd.dispatch(
            div_round_up(size.x, 4),
            div_round_up(size.y, 4),
            div_round_up(size.z, 4));
    }

    void transient_resources()
    {
        // TEST:
        // 1. Create transient resources.
        // 2. Initialize them.
        // 3. Test if their content stays intact over multiple aliased operations.

        daxa::Context daxa_ctx = daxa::create_context({
            .enable_validation = false,
        });
        daxa::Device device = daxa_ctx.create_device({
            .name = "device",
        });

        daxa::PipelineManager pipeline_manager = daxa::PipelineManager{{
            .device = device,
            .shader_compile_options = {
                .root_paths = {
                    DAXA_SHADER_INCLUDE_DIR,
                    "tests/2_daxa_api/6_task_list/shaders",
                },
                .language = daxa::ShaderLanguage::GLSL,
            },
            .name = "pipeline manager",
        }};

        daxa::ComputePipelineCompileInfo const test_image_pipeline_info = {
            .shader_info = {
                .source = daxa::ShaderFile{"transient.glsl"},
                .compile_options = {
                    .defines = std::vector{daxa::ShaderDefine{"TEST_IMAGE", "1"}}},
            },
            .push_constant_size = sizeof(TestImagePush),
            .name = "test image",
        };
        auto test_image_pipeline = pipeline_manager.add_compute_pipeline(test_image_pipeline_info).value();

        daxa::ComputePipelineCompileInfo const test_buffer_pipeline_info = {
            .shader_info = {
                .source = daxa::ShaderFile{"transient.glsl"},
            },
            .push_constant_size = sizeof(TestImagePush),
            .name = "test buffer",
        };
        auto test_buffer_pipeline = pipeline_manager.add_compute_pipeline(test_buffer_pipeline_info).value();

        {
            auto task_list = daxa::TaskList({
                .device = device,
                .reorder_tasks = false, // Disable reordering for testing purposes.
                .record_debug_information = true,
                .name = "task_list",
            });

            // Declare transient resources.
            auto long_life_buffer =
                task_list.create_transient_buffer({
                    .size = LONG_LIFE_BUFFER_SIZE * sizeof(daxa::u32),
                    .name = "long life buffer",
                });

            auto medium_life_image =
                task_list.create_transient_image({
                    .dimensions = 3,
                    .format = daxa::Format::R32_SFLOAT,
                    .size = {MEDIUM_LIFE_IMAGE_SIZE.x, MEDIUM_LIFE_IMAGE_SIZE.y, MEDIUM_LIFE_IMAGE_SIZE.z},
                    .name = "medium life image",
                });

            auto long_life_image =
                task_list.create_transient_image({
                    .dimensions = 3,
                    .format = daxa::Format::R32_SFLOAT,
                    .size = {LONG_LIFE_IMAGE_SIZE.x, LONG_LIFE_IMAGE_SIZE.y, LONG_LIFE_IMAGE_SIZE.z},
                    .name = "long life image",
                });

            auto short_life_image =
                task_list.create_transient_image({
                    .dimensions = 3,
                    .format = daxa::Format::R32_SFLOAT,
                    .size = {SHORT_LIFE_IMAGE_SIZE.x, SHORT_LIFE_IMAGE_SIZE.y, SHORT_LIFE_IMAGE_SIZE.z},
                    .name = "short life large image",
                });

            auto short_life_buffer =
                task_list.create_transient_buffer({
                    .size = SHORT_LIFE_BUFFER_SIZE * sizeof(daxa::u32),
                    .name = "tiny size short life buffer",
                });

            using TBA = daxa::TaskBufferAccess;
            using TIA = daxa::TaskImageAccess;
            using TBU = daxa::TaskBufferUseInit;
            using TIU = daxa::TaskImageUseInit;

            // Record tasks.
            task_list.add_task({
                .used_buffers = {
                    TBU{.id = long_life_buffer, .access = TBA::TRANSFER_WRITE},
                },
                .task = [=](daxa::TaskInterface<> tri)
                {
                    auto cmd = tri.get_command_list();
                    set_initial_buffer_data(tri, cmd, long_life_buffer, LONG_LIFE_BUFFER_SIZE, LONG_LIFE_BUFFER_VALUE);
                },
                .name = "populate long life buffer",
            });

            task_list.add_task({
                .used_images = {
                    TIU{.id = medium_life_image, .access = TIA::TRANSFER_WRITE},
                },
                .task = [=](daxa::TaskInterface<> tri)
                {
                    auto cmd = tri.get_command_list();
                    set_initial_image_data(tri, cmd, medium_life_image, MEDIUM_LIFE_IMAGE_SIZE, MEDIUM_LIFE_IMAGE_VALUE);
                },
                .name = "populate medium life image",
            });

            task_list.add_task({
                .used_buffers = {TBU{.id = long_life_buffer, .access = TBA::COMPUTE_SHADER_READ_ONLY}},
                .used_images = {
                    TIU{.id = medium_life_image, .access = TIA::COMPUTE_SHADER_READ_ONLY, .view_type = daxa::ImageViewType::REGULAR_3D},
                    TIU{.id = long_life_image, .access = TIA::TRANSFER_WRITE},
                },
                .task = [=](daxa::TaskInterface<> tri)
                {
                    auto cmd = tri.get_command_list();
                    set_initial_image_data(tri, cmd, long_life_image, LONG_LIFE_IMAGE_SIZE, LONG_LIFE_IMAGE_VALUE);
                    validate_image_data(tri, cmd, long_life_image, LONG_LIFE_IMAGE_SIZE, LONG_LIFE_IMAGE_VALUE, *test_image_pipeline);
                    validate_buffer_data(tri, cmd, long_life_buffer, LONG_LIFE_BUFFER_SIZE, LONG_LIFE_BUFFER_VALUE, *test_buffer_pipeline);
                },
                .name = "validate long life buffer, validate medium life image, populate long life image",
            });

            task_list.add_task({
                .used_images = {TIU{.id = short_life_image, .access = TIA::COMPUTE_SHADER_READ_WRITE}},
                .task = [=](daxa::TaskInterface<>) {},
                .name = "dummy use short life image",
            });

            task_list.add_task({
                .used_buffers = {TBU{.id = short_life_buffer, .access = TBA::COMPUTE_SHADER_READ_WRITE}},
                .used_images = {TIU{.id = long_life_image, .access = TIA::COMPUTE_SHADER_READ_ONLY, .view_type = daxa::ImageViewType::REGULAR_3D}},
                .task = [=](daxa::TaskInterface<> tri)
                {
                    auto cmd = tri.get_command_list();
                    validate_image_data(tri, cmd, long_life_image, LONG_LIFE_IMAGE_SIZE, LONG_LIFE_IMAGE_VALUE, *test_image_pipeline);
                },
                .name = "validate long life image, dummy access short life buffer",
            });

            task_list.submit({});
            task_list.complete({});
            task_list.execute({});

            std::cout << task_list.get_debug_string() << std::endl;
        }
        device.wait_idle();
        device.collect_garbage();
    }
} // namespace tests
