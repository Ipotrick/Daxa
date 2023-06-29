#pragma once

#include "common.hpp"
#include "mipmapping.hpp"
#include "shaders/shader_integration.inl"
#include "persistent_resources.hpp"
#include "transient_overlap.hpp"

namespace tests
{
    using namespace daxa::task_resource_uses;

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
            .uses = {},
            .task = [&](daxa::TaskInterface const &)
            {
                std::cout << "Hello, ";
            },
            .name = APPNAME_PREFIX("task 1 (execution)"),
        });
        task_list.add_task({
            .uses = {},
            .task = [&](daxa::TaskInterface const &)
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
        // Need to scope the task lists lifetime.
        // Task list MUST die before we call wait_idle and collect_garbage.
        auto task_list = daxa::TaskList({
            .device = app.device,
            .record_debug_information = true,
            .name = APPNAME_PREFIX("create-write-read image"),
        });
        // CREATE IMAGE
        auto task_image = task_list.create_transient_image(daxa::TaskTransientImageInfo{.size = {1, 1, 1}, .name = "task list tested image"});
        // WRITE IMAGE 1
        task_list.add_task({
            .uses = {daxa::TaskImageUse<daxa::TaskImageAccess::COMPUTE_SHADER_WRITE>{task_image}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("write image 1"),
        });
        // READ_IMAGE 1
        task_list.add_task({
            .uses = {daxa::TaskImageUse<daxa::TaskImageAccess::COMPUTE_SHADER_READ>{task_image}},
            .task = [](daxa::TaskInterface const &) {},
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
            .uses = {ImageComputeShaderWrite<>{task_image.subslice({.base_array_layer = 0, .layer_count = 1})}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("write image array layer 1"),
        });
        // READ_IMAGE 1
        task_list.add_task({
            .uses = {ImageComputeShaderRead<>{task_image.subslice({.base_array_layer = 1, .layer_count = 1})}},
            .task = [](daxa::TaskInterface const &) {},
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
            .uses = {daxa::TaskBufferUse<daxa::TaskBufferAccess::HOST_TRANSFER_WRITE>{task_buffer}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("host transfer buffer"),
        });

        task_list.add_task({
            .uses = {daxa::TaskBufferUse<daxa::TaskBufferAccess::COMPUTE_SHADER_READ>{task_buffer}},
            .task = [](daxa::TaskInterface const &) {},
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
            .uses = {ImageComputeShaderRead<>{task_image.handle().subslice({.base_array_layer = 1, .layer_count = 1})}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("read array layer 2"),
        });
        task_list.add_task({
            .uses = {ImageComputeShaderWrite<>{task_image.handle().subslice({.base_array_layer = 0, .layer_count = 1})}},
            .task = [](daxa::TaskInterface const &) {},
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
            .uses = {ImageComputeShaderRead<>{task_image.handle().subslice({.base_array_layer = 1, .layer_count = 1})}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("read image layer 2"),
        });
        task_list.add_task({
            .uses = {ImageComputeShaderWrite<>{task_image.handle().subslice({.base_array_layer = 3, .layer_count = 1})}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("write image layer 4"),
        });
        task_list.add_task({
            .uses = {ImageComputeShaderWrite<>{task_image.handle().subslice({.base_array_layer = 0, .layer_count = 4})}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("write image layer 1 - 4"),
        });
        task_list.add_task({
            .uses = {ImageComputeShaderRead<>{task_image.handle().subslice({.base_array_layer = 0, .layer_count = 4})}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("read image layer 1 - 4"),
        });

        task_list.complete({});
        task_list.execute({});
        std::cout << task_list.get_debug_string() << std::endl;
        app.device.destroy_image(image);
    }

    void shader_integration_inl_use()
    {
        // TEST:
        //  1) Create resources
        //  2) Use Compute dispatch to write to image
        //  4) readback and validate
        AppContext app = {};
        auto dummy = app.device.create_image({
            .size = {16, 16, 1},
            .array_layer_count = 1,
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .name = "dummy",
        });
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
            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
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
            .uses = daxa::to_generic_uses(ShaderIntegrationTask::Uses{
                .settings = {task_buffer},
                .image = {task_image},
            }),
            .task = [&](daxa::TaskInterface ti)
            {
                auto cmd = ti.get_command_list();
                cmd.set_uniform_buffer(ti.uses.get_uniform_buffer_info());
                cmd.set_pipeline(*compute_pipeline);
                cmd.dispatch(1, 1, 1);
            },
            .constant_buffer_slot = ShaderIntegrationTask::CONSANT_BUFFER_SLOT,
            .name = "write image in compute",
        });
        task_list.add_task({
            .uses = daxa::to_generic_uses(ShaderIntegrationTask::Uses{
                .settings = {task_buffer},
                .image = {task_image},
            }),
            .task = [&](daxa::TaskInterface ti)
            {
                auto cmd = ti.get_command_list();
                // Optionally, the shader uses can still be accessed with the usual task interface for immediate tasks. 
                [[maybe_unused]] auto img = ti.uses[task_image].image();
                // Get a constant buffer set info, ready to use for the next pipelines constants.
                cmd.set_uniform_buffer(ti.uses.get_uniform_buffer_info());
                cmd.set_pipeline(*compute_pipeline);
                cmd.dispatch(1, 1, 1);
            },
            .constant_buffer_slot = ShaderIntegrationTask::CONSANT_BUFFER_SLOT,
            .name = "write image in compute 2",
        });
        task_list.submit({});

        task_list.complete({});
        task_list.execute({});
        std::cout << task_list.get_debug_string() << std::endl;
        app.device.wait_idle();
        app.device.destroy_image(image);
        app.device.destroy_image(dummy);
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
            .uses = {daxa::TaskImageUse<daxa::TaskImageAccess::SHADER_WRITE>{persistent_task_image}},
            .task = [&](daxa::TaskInterface const &) {},
            .name = "write persistent image",
        });
        task_list.add_task({
            .uses = {
                daxa::TaskBufferUse<daxa::TaskBufferAccess::SHADER_READ>{persistent_task_buffer},
                daxa::TaskImageUse<daxa::TaskImageAccess::SHADER_READ>{persistent_task_image},
            },
            .task = [&](daxa::TaskInterface const &) {},
            .name = "read persistent image, read persistent buffer",
        });
        task_list.add_task({
            .uses = {daxa::TaskBufferUse<daxa::TaskBufferAccess::SHADER_READ>{persistent_task_buffer}},
            .task = [&](daxa::TaskInterface const &) {},
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
} // namespace tests

auto main() -> int
{
    // tests::simplest();
    // tests::execution();
    // tests::write_read_image();
    // tests::write_read_image_layer();
    // tests::create_transfer_read_buffer();
    // tests::initial_layout_access();
    // tests::tracked_slice_barrier_collapsing();
    // tests::correct_read_buffer_task_ordering();
    // tests::sharing_persistent_image();
    // tests::sharing_persistent_buffer();
    // tests::transient_resources();
    tests::shader_integration_inl_use();
    tests::mipmapping();
}
