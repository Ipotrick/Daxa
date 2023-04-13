#pragma once

#include "common.hpp"
#include "mipmapping.hpp"
#include "persistent_resources.hpp"
#include "transient_overlap.hpp"

#include "shaders/shader_integration.inl"

namespace tests
{
    void simplest()
    {
        AppContext const app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .name = APPNAME_PREFIX("task_list (simplest)"),
        });
    }

    void execution()
    {
        AppContext const app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .name = APPNAME_PREFIX("task_list (execution)"),
        });

        // This is pointless, but done to show how the task list executes
        task_list.add_task({
            .task = [&](daxa::TaskRuntimeInterface const &)
            {
                std::cout << "Hello, ";
            },
            .name = APPNAME_PREFIX("task 1 (execution)"),
        });
        task_list.add_task({
            .task = [&](daxa::TaskRuntimeInterface const &)
            {
                std::cout << "World!" << std::endl;
            },
            .name = APPNAME_PREFIX("task 2 (execution)"),
        });

        task_list.complete({});

        task_list.execute({});
    }

    void write_read_image()
    {
        // TEST:
        //    1) CREATE image
        //    2) WRITE image
        //    3) READ image
        AppContext app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .record_debug_information = true,
            .name = APPNAME_PREFIX("create-write-read image"),
        });
        // CREATE IMAGE
        auto task_image = task_list.create_transient_image({.size = {1, 1, 1}, .name = "task list tested image"});
        // WRITE IMAGE 1
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY,
                     daxa::ImageMipArraySlice{}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("write image 1"),
        });
        // READ_IMAGE 1
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_READ_ONLY,
                     daxa::ImageMipArraySlice{}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("read image 1"),
        });

        task_list.complete({});
        task_list.execute({});
        std::cout << task_list.get_debug_string() << std::endl;
    }

    void write_read_image_layer()
    {
        // TEST:
        //    1) CREATE image
        //    2) WRITE into array layer 1 of the image
        //    3) READ from array layer 2 of the image
        AppContext app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .record_debug_information = true,
            .name = APPNAME_PREFIX("create-write-read array layer"),
        });
        // CREATE IMAGE
        auto task_image = task_list.create_transient_image({
            .size = {1, 1, 1},
            .array_layer_count = 2,
            .name = "task list tested image",
        });
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 0, .layer_count = 1}}},
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("write image array layer 1"),
        });
        // READ_IMAGE 1
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_READ_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 1, .layer_count = 1}}},
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("read image array layer 1"),
        });
        task_list.complete({});
        task_list.execute({});
        std::cout << task_list.get_debug_string() << std::endl;
    }

    void create_transfer_read_buffer()
    {
        // TEST:
        //    1) CREATE buffer
        //    2) TRANSFER into the buffer
        //    3) READ from the buffer
        AppContext app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .record_debug_information = true,
            .name = APPNAME_PREFIX("create-transfer-read buffer"),
        });

        auto task_buffer = task_list.create_transient_buffer({
            .size = sizeof(u32),
            .name = "task list tested buffer",
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer, daxa::TaskBufferAccess::HOST_TRANSFER_WRITE},
            },
            .used_images = {},
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("host transfer buffer"),
        });

        task_list.add_task({
            .used_buffers = {
                {
                    task_buffer,
                    daxa::TaskBufferAccess::COMPUTE_SHADER_READ_ONLY,
                },
            },
            .used_images = {},
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("read buffer"),
        });

        task_list.complete({});
        task_list.execute({});
        std::cout << task_list.get_debug_string() << std::endl;
    }

    void initial_layout_access()
    {
        // TEST:
        //    1) CREATE image - set the task image initial access to write from compute shader
        //    2) READ from a the subimage
        //    3) WRITE into the subimage
        AppContext app = {};
        auto image = app.device.create_image({
            .size = {1, 1, 1},
            .array_layer_count = 2,
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE,
            .name = APPNAME_PREFIX("tested image"),
        });

        std::array init_access = {
            daxa::ImageSliceState{
                .latest_access = daxa::AccessConsts::COMPUTE_SHADER_WRITE,
                .latest_layout = daxa::ImageLayout::GENERAL,
            },
        };

        auto task_image = daxa::TaskImage(daxa::TaskImageInfo{
            .initial_images = {
                .images = {&image, 1},
                .latest_slice_states = {init_access.data(), 1}},
            .swapchain_image = false,
            .name = "task list tested image",
        });

        auto task_list = daxa::TaskList({
            .device = app.device,
            .record_debug_information = true,
            .name = APPNAME_PREFIX("initial layout image"),
        });
        // CREATE IMAGE
        task_list.use_persistent_image(task_image);
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_READ_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 1, .layer_count = 1}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("read array layer 2"),
        });
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 0, .layer_count = 1}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("write array layer 1"),
        });
        task_list.complete({});
        task_list.execute({});
        std::cout << task_list.get_debug_string() << std::endl;
        app.device.destroy_image(image);
    }

    void tracked_slice_barrier_collapsing()
    {
        // TEST:
        //    1) CREATE image - set the task image initial access to write from compute
        //                      shader for one subresouce and read for the other
        //    2) WRITE into the subsubimage
        //    3) READ from a the subsubimage
        //    4) WRITE into the entire image
        //    5) READ the entire image
        //    Expected: There should only be a single barrier between tests 4 and 5.
        AppContext app = {};
        auto image = app.device.create_image({
            .size = {1, 1, 1},
            .array_layer_count = 4,
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE,
            .name = APPNAME_PREFIX("tested image"),
        });

        // CREATE IMAGE
        std::array init_access = {
            daxa::ImageSliceState{
                .latest_access = daxa::AccessConsts::COMPUTE_SHADER_WRITE,
                .latest_layout = daxa::ImageLayout::GENERAL,
                .slice = {.base_array_layer = 0, .layer_count = 2},
            },
            daxa::ImageSliceState{
                .latest_access = daxa::AccessConsts::COMPUTE_SHADER_READ,
                .latest_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
                .slice = {.base_array_layer = 2, .layer_count = 2},
            }};
        auto task_image = daxa::TaskImage({
            .name = "task list tested image",
        });

        auto task_list = daxa::TaskList({
            .device = app.device,
            .record_debug_information = true,
            .name = APPNAME_PREFIX("tracked slice barrier collapsing"),
        });

        task_image.set_images({.images = {&image, 1}, .latest_slice_states = {init_access.begin(), init_access.end()}});

        task_list.use_persistent_image(task_image);

        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_READ_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 1, .layer_count = 1}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("read image layer 2"),
        });
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 3, .layer_count = 1}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("write image layer 4"),
        });
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_WRITE_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 0, .layer_count = 4}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("write image layer 1 - 4"),
        });
        task_list.add_task({
            .used_buffers = {},
            .used_images =
                {
                    {task_image,
                     daxa::TaskImageAccess::COMPUTE_SHADER_READ_ONLY,
                     daxa::ImageMipArraySlice{.base_array_layer = 0, .layer_count = 4}},
                },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("read image layer 1 - 4"),
        });

        task_list.complete({});
        task_list.execute({});
        std::cout << task_list.get_debug_string() << std::endl;
        app.device.wait_idle();
        app.device.destroy_image(image);
        app.device.collect_garbage();
    }

    void shader_integration_inl_use()
    {
        // TEST:
        //  1) Create resources
        //  2) Use Compute dispatch to write to image
        //  4) readback and validate
        AppContext app = {};
        auto image = app.device.create_image({
            .size = {16, 16, 1},
            .array_layer_count = 1,
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .name = "underlying image",
        });
        auto task_image = daxa::TaskImage({
            // In this test, this image name will be "aliased", so the name must not be the same.
            .initial_images = {
                .images = {&image, 1},
            },
            .name = "image",
        });
        auto buffer = app.device.create_buffer({
            .size = 16,
            .allocate_info = daxa::AutoAllocInfo{.flags = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE},
            .name = "underlying buffer",
        });
        *app.device.get_host_address_as<float>(buffer) = 0.75f;
        auto task_buffer = daxa::TaskBuffer({
            .initial_buffers = {
                .buffers = {&buffer, 1},
                .latest_access = daxa::AccessConsts::HOST_WRITE,
            },
            .name = "settings", // This name MUST be identical to the name used in the shader.
        });

        daxa::PipelineManager pipeline_manager = daxa::PipelineManager({
            .device = app.device,
            .shader_compile_options = {
                .root_paths = {
                    DAXA_SHADER_INCLUDE_DIR,
                    "tests/2_daxa_api/6_task_list/shaders",
                },
            },
            .name = "pipeline manager",
        });

        auto compile_result = pipeline_manager.add_compute_pipeline({
            .shader_info = {
                .source = daxa::ShaderFile{"shader_integration.glsl"},
                .compile_options{
                    .enable_debug_info = true,
                },
            },
            .name = "compute_pipeline",
        });
        auto compute_pipeline = compile_result.value();

        auto task_list = daxa::TaskList({
            .device = app.device,
            .record_debug_information = true,
            .name = "shader integration test - task list",
        });
        task_list.use_persistent_image(task_image);
        task_list.use_persistent_buffer(task_buffer);

        task_list.add_task({
            // daxa::ShaderIntegrationTaskListUses is a startup time generated constant global.
            // This global is set from auto generated code in the .inl file.
            .shader_uses = daxa::ShaderIntegrationTaskListUses,
            .shader_uses_image_aliases = {
                daxa::TaskImageAliasInfo{.alias = "shader_integration_image", .aliased_image = task_image},
            },
            .task = [&](daxa::TaskRuntimeInterface const & tri)
            {
                auto cmd = tri.get_command_list();
                cmd.set_pipeline(*compute_pipeline);
                cmd.dispatch(1, 1, 1);
            },
            .name = "write image in compute",
        });
        task_list.submit({});

        task_list.complete({});
        task_list.execute({});
        std::cout << task_list.get_debug_string() << std::endl;
        app.device.wait_idle();
        app.device.destroy_image(image);
        app.device.destroy_buffer(buffer);
        app.device.collect_garbage();
    }

    void correct_read_buffer_task_ordering()
    {
        // TEST:
        //  1) Create persistent image and persistent buffer
        //  2) Record two task lists A
        //  3) Task list A has three tasks inserted in listed order:
        //      Task 1) Writes image
        //      Task 2) Reads image and reads buffer
        //      Task 3) Reads buffer
        //  5) Execute task list and check the ordering of tasks in batches
        //  Expected result:
        //      NOTE(msakmary): This does something different currently (task 3 is in batch 2)
        //                      - this is due to limitations of what task list can do without having a proper render graph
        //                      - will be fixed in the future by adding JIRA
        //      Batch 1:
        //          Task 1
        //          Task 3
        //      Batch 2:
        //          Task 2
        daxa::Context daxa_ctx = daxa::create_context({
            .enable_validation = false,
        });
        daxa::Device device = daxa_ctx.create_device({
            .name = "device",
        });
        auto image = device.create_image({
            .size = {1, 1, 1},
            .array_layer_count = 1,
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE,
            .name = "actual image",
        });

        auto buffer = device.create_buffer({.size = 1,
                                            .name = "actual_buffer"});

        auto persistent_task_image = daxa::TaskImage(daxa::TaskImageInfo{
            .initial_images = {.images = {&image, 1}},
            .swapchain_image = false,
            .name = "image",
        });

        auto persistent_task_buffer = daxa::TaskBuffer(daxa::TaskBufferInfo{
            .initial_buffers = {.buffers = {&buffer, 1}},
            .name = "buffer",
        });

        auto task_list = daxa::TaskList({
            .device = device,
            .record_debug_information = true,
            .name = "task_list",
        });

        task_list.use_persistent_image(persistent_task_image);
        task_list.use_persistent_buffer(persistent_task_buffer);
        task_list.add_task({
            .used_images = {{.id = persistent_task_image, .access = daxa::TaskImageAccess::SHADER_WRITE_ONLY}},
            .task = [&](daxa::TaskRuntimeInterface const &) {},
            .name = "write persistent image",
        });
        task_list.add_task({
            .used_buffers = {{.id = persistent_task_buffer, .access = daxa::TaskBufferAccess::SHADER_READ_ONLY}},
            .used_images = {{.id = persistent_task_image, .access = daxa::TaskImageAccess::SHADER_READ_ONLY}},
            .task = [&](daxa::TaskRuntimeInterface const &) {},
            .name = "read persistent image, read persistent buffer",
        });
        task_list.add_task({
            .used_buffers = {{.id = persistent_task_buffer, .access = daxa::TaskBufferAccess::SHADER_READ_ONLY}},
            .task = [&](daxa::TaskRuntimeInterface const &) {},
            .name = "read persistent buffer",
        });
        task_list.submit({});
        task_list.complete({});

        task_list.execute({});
        std::cout << task_list.get_debug_string() << std::endl;

        device.wait_idle();
        device.destroy_image(image);
        device.destroy_buffer(buffer);
        device.collect_garbage();
    }

    void output_graph()
    {
        /*
        AppContext const app = {};
        auto task_list = daxa::TaskList({
            .device = app.device,
            .name = APPNAME_PREFIX("task_list (output_graph)"),
        });

        auto task_buffer1 = daxa::TaskBuffer({
            .buffers = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .name = APPNAME_PREFIX("task_buffer1"),
        });
        auto task_buffer2 = task_list.create_transient_task_buffer({
            .pre_task_list_slice_states = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .name = APPNAME_PREFIX("task_buffer2"),
        });
        auto task_buffer3 = task_list.create_transient_task_buffer({
            .pre_task_list_slice_states = {daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
            .name = APPNAME_PREFIX("task_buffer3"),
        });

        std::array init_access = {
            daxa::ImageSliceState{
                .latest_access = daxa::Access{daxa::PipelineStageFlagBits::HOST, daxa::AccessTypeFlagBits::WRITE},
                .latest_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL}};
        auto task_image1 = task_list.create_transient_task_image({
            .pre_task_list_slice_states = {init_access.begin(), init_access.end()},
            .name = APPNAME_PREFIX("task_image1"),
        });
        auto task_image2 = task_list.create_transient_task_image({
            .pre_task_list_slice_states = {init_access.begin(), init_access.end()},
            .name = APPNAME_PREFIX("task_image2"),
        });
        auto task_image3 = task_list.create_transient_task_image({
            .pre_task_list_slice_states = {init_access.begin(), init_access.end()},
            .name = APPNAME_PREFIX("task_image3"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer1, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
                {task_buffer2, daxa::TaskBufferAccess::SHADER_READ_ONLY},
            },
            .used_images = {
                {task_image1, daxa::TaskImageAccess::SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
                {task_image2, daxa::TaskImageAccess::SHADER_READ_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("task 1 (output_graph)"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer2, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
            },
            .used_images = {
                {task_image2, daxa::TaskImageAccess::SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("task 2 (output_graph)"),
        });
        task_list.add_task({
            .used_buffers = {
                {task_buffer3, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
            },
            .used_images = {
                {task_image3, daxa::TaskImageAccess::SHADER_WRITE_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("task 3 (output_graph)"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer2, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
                {task_buffer3, daxa::TaskBufferAccess::SHADER_WRITE_ONLY},
            },
            .used_images = {
                {
                    task_image2,
                    daxa::TaskImageAccess::SHADER_WRITE_ONLY,
                    daxa::ImageMipArraySlice{
                        .base_mip_level = 0,
                        .level_count = 3,
                        .base_array_layer = 2,
                        .layer_count = 2,
                    },
                },
                {
                    task_image3,
                    daxa::TaskImageAccess::SHADER_WRITE_ONLY,
                    daxa::ImageMipArraySlice{
                        .base_mip_level = 0,
                        .level_count = 3,
                        .base_array_layer = 2,
                        .layer_count = 2,
                    },
                },
            },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("task 4 (output_graph)"),
        });

        task_list.add_task({
            .used_buffers = {
                {task_buffer3, daxa::TaskBufferAccess::SHADER_READ_ONLY},
            },
            .used_images = {
                {task_image3, daxa::TaskImageAccess::SHADER_READ_ONLY, daxa::ImageMipArraySlice{}},
            },
            .task = [](daxa::TaskRuntimeInterface const &) {},
            .name = APPNAME_PREFIX("task 5 (output_graph)"),
        });

        task_list.complete({});
        task_list.output_graphviz();
        */
    }
}

auto main() -> int
{
    // tests::write_read_image();
    // tests::write_read_image_layer();
    // tests::create_transfer_read_buffer();
    // tests::initial_layout_access();
    // tests::tracked_slice_barrier_collapsing();
    // tests::shader_integration_inl_use();
    tests::mipmapping();
    // tests::correct_read_buffer_task_ordering();
    // tests::sharing_persistent_image();
    // tests::sharing_persistent_buffer();
    // tests::transient_resources();
}
