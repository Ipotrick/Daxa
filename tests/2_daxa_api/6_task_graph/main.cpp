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
        auto d = app.device;
        struct S { daxa::Device d; } s = { d };
        auto task_graph = daxa::TaskGraph({
            .device = app.device,
            .name = APPNAME_PREFIX("task_graph (simplest)"),
        });
    }

    void execution()
    {
        AppContext const app = {};
        auto task_graph = daxa::TaskGraph({
            .device = app.device,
            .name = APPNAME_PREFIX("task_graph (execution)"),
        });

        // This is pointless, but done to show how the task graph executes
        task_graph.add_task({
            .uses = {},
            .task = [&](daxa::TaskInterface const &)
            {
                std::cout << "Hello, ";
            },
            .name = APPNAME_PREFIX("task 1 (execution)"),
        });
        task_graph.add_task({
            .uses = {},
            .task = [&](daxa::TaskInterface const &)
            {
                std::cout << "World!" << std::endl;
            },
            .name = APPNAME_PREFIX("task 2 (execution)"),
        });

        task_graph.complete({});

        task_graph.execute({});
    }

    void write_read_image()
    {
        // TEST:
        //    1) CREATE image
        //    2) WRITE image
        //    3) READ image
        AppContext app = {};
        // Need to scope the task graphs lifetime.
        // Task graph MUST die before we call wait_idle and collect_garbage.
        auto task_graph = daxa::TaskGraph({
            .device = app.device,
            .record_debug_information = true,
            .name = APPNAME_PREFIX("create-write-read image"),
        });
        // CREATE IMAGE
        auto task_image = task_graph.create_transient_image(daxa::TaskTransientImageInfo{.size = {1, 1, 1}, .name = "task graph tested image"});
        // WRITE IMAGE 1
        task_graph.add_task({
            .uses = {daxa::TaskImageUse<daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY>{task_image}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("write image 1"),
        });
        // READ_IMAGE 1
        task_graph.add_task({
            .uses = {daxa::TaskImageUse<daxa::TaskImageAccess::COMPUTE_SHADER_SAMPLED>{task_image}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("read image 1"),
        });

        task_graph.complete({});
        task_graph.execute({});
        std::cout << task_graph.get_debug_string() << std::endl;
    }

    void write_read_image_layer()
    {
        // TEST:
        //    1) CREATE image
        //    2) WRITE into array layer 1 of the image
        //    3) READ from array layer 2 of the image
        AppContext app = {};
        auto task_graph = daxa::TaskGraph({
            .device = app.device,
            .record_debug_information = true,
            .name = APPNAME_PREFIX("create-write-read array layer"),
        });
        // CREATE IMAGE
        auto task_image = task_graph.create_transient_image({
            .size = {1, 1, 1},
            .array_layer_count = 2,
            .name = "task graph tested image",
        });
        task_graph.add_task({
            .uses = {ImageComputeShaderStorageWriteOnly<>{task_image.view({.base_array_layer = 0, .layer_count = 1})}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("write image array layer 1"),
        });
        // READ_IMAGE 1
        task_graph.add_task({
            .uses = {ImageComputeShaderSampled<>{task_image.view({.base_array_layer = 1, .layer_count = 1})}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("read image array layer 1"),
        });
        task_graph.complete({});
        task_graph.execute({});
        std::cout << task_graph.get_debug_string() << std::endl;
    }

    void create_transfer_read_buffer()
    {
        // TEST:
        //    1) CREATE buffer
        //    2) TRANSFER into the buffer
        //    3) READ from the buffer
        AppContext app = {};
        auto task_graph = daxa::TaskGraph({
            .device = app.device,
            .record_debug_information = true,
            .name = APPNAME_PREFIX("create-transfer-read buffer"),
        });

        auto task_buffer = task_graph.create_transient_buffer({
            .size = sizeof(u32),
            .name = "task graph tested buffer",
        });

        task_graph.add_task({
            .uses = {daxa::TaskBufferUse<daxa::TaskBufferAccess::HOST_TRANSFER_WRITE>{task_buffer}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("host transfer buffer"),
        });

        task_graph.add_task({
            .uses = {daxa::TaskBufferUse<daxa::TaskBufferAccess::COMPUTE_SHADER_READ>{task_buffer}},
            .task = [](daxa::TaskInterface const &) {},
            .name = APPNAME_PREFIX("read buffer"),
        });

        task_graph.complete({});
        task_graph.execute({});
        std::cout << task_graph.get_debug_string() << std::endl;
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
            .usage = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
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
            .name = "task graph tested image",
        });

        // TG MUST die before image, as it holds image views to the image that must die before the image.
        {
            auto task_graph = daxa::TaskGraph({
                .device = app.device,
                .record_debug_information = true,
                .name = APPNAME_PREFIX("initial layout image"),
            });
            // CREATE IMAGE
            task_graph.use_persistent_image(task_image);
            task_graph.add_task({
                .uses = {ImageComputeShaderSampled<>{task_image.view().view({.base_array_layer = 1, .layer_count = 1})}},
                .task = [](daxa::TaskInterface const &) {},
                .name = APPNAME_PREFIX("read array layer 2"),
            });
            task_graph.add_task({
                .uses = {ImageComputeShaderStorageWriteOnly<>{task_image.view().view({.base_array_layer = 0, .layer_count = 1})}},
                .task = [](daxa::TaskInterface const &) {},
                .name = APPNAME_PREFIX("write array layer 1"),
            });
            task_graph.complete({});
            task_graph.execute({});
            std::cout << task_graph.get_debug_string() << std::endl;
        }
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
            .usage = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
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
            .name = "task graph tested image",
        });

        // TG MUST die before image, as it holds image views to the image that must die before the image.
        {
            auto task_graph = daxa::TaskGraph({
                .device = app.device,
                .record_debug_information = true,
                .name = APPNAME_PREFIX("tracked slice barrier collapsing"),
            });

            task_image.set_images({.images = {&image, 1}, .latest_slice_states = {init_access.begin(), init_access.end()}});

            task_graph.use_persistent_image(task_image);

            task_graph.add_task({
                .uses = {ImageComputeShaderSampled<>{task_image.view().view({.base_array_layer = 1, .layer_count = 1})}},
                .task = [](daxa::TaskInterface const &) {},
                .name = APPNAME_PREFIX("read image layer 2"),
            });
            task_graph.add_task({
                .uses = {ImageComputeShaderStorageWriteOnly<>{task_image.view().view({.base_array_layer = 3, .layer_count = 1})}},
                .task = [](daxa::TaskInterface const &) {},
                .name = APPNAME_PREFIX("write image layer 4"),
            });
            task_graph.add_task({
                .uses = {ImageComputeShaderStorageWriteOnly<>{task_image.view().view({.base_array_layer = 0, .layer_count = 4})}},
                .task = [](daxa::TaskInterface const &) {},
                .name = APPNAME_PREFIX("write image layer 1 - 4"),
            });
            task_graph.add_task({
                .uses = {ImageComputeShaderSampled<>{task_image.view().view({.base_array_layer = 0, .layer_count = 4})}},
                .task = [](daxa::TaskInterface const &) {},
                .name = APPNAME_PREFIX("read image layer 1 - 4"),
            });

            task_graph.complete({});
            task_graph.execute({});
            std::cout << task_graph.get_debug_string() << std::endl;
        }
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
            .usage = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .name = "dummy",
        });
        auto image = app.device.create_image({
            .size = {16, 16, 1},
            .array_layer_count = 1,
            .usage = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
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
        *app.device.get_host_address_as<float>(buffer).value() = 0.75f;
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
                    "tests/2_daxa_api/6_task_graph/shaders",
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

        auto task_graph = daxa::TaskGraph({
            .device = app.device,
            .record_debug_information = true,
            .name = "shader integration test - task graph",
        });
        task_graph.use_persistent_image(task_image);
        task_graph.use_persistent_buffer(task_buffer);

        task_graph.add_task({
            .uses = daxa::detail::to_generic_uses(ShaderIntegrationTask::Uses{
                .settings = {task_buffer},
                .image = {task_image},
            }),
            .task = [&](daxa::TaskInterface ti)
            {
                auto& cmd = ti.get_recorder();
                cmd.set_uniform_buffer(ti.uses.get_uniform_buffer_info());
                cmd.set_pipeline(*compute_pipeline);
                cmd.dispatch({1, 1, 1});
            },
            .constant_buffer_slot = ShaderIntegrationTask::CONSTANT_BUFFER_SLOT,
            .name = "write image in compute",
        });
        task_graph.add_task({
            .uses = daxa::detail::to_generic_uses(ShaderIntegrationTask::Uses{
                .settings = {task_buffer},
                .image = {task_image},
            }),
            .task = [&](daxa::TaskInterface ti)
            {
                auto& cmd = ti.get_recorder();
                // Optionally, the shader uses can still be accessed with the usual task interface for immediate tasks. 
                [[maybe_unused]] auto img = ti.uses[task_image].image();
                // Get a constant buffer set info, ready to use for the next pipelines constants.
                cmd.set_uniform_buffer(ti.uses.get_uniform_buffer_info());
                cmd.set_pipeline(*compute_pipeline);
                cmd.dispatch({1, 1, 1});
            },
            .constant_buffer_slot = ShaderIntegrationTask::CONSTANT_BUFFER_SLOT,
            .name = "write image in compute 2",
        });
        task_graph.submit({});

        task_graph.complete({});
        task_graph.execute({});
        std::cout << task_graph.get_debug_string() << std::endl;
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
        //  2) Record two task graphs A
        //  3) Task graph A has three tasks inserted in listed order:
        //      Task 1) Writes image
        //      Task 2) Reads image and reads buffer
        //      Task 3) Reads buffer
        //  5) Execute task graph and check the ordering of tasks in batches
        //  Expected result:
        //      NOTE(msakmary): This does something different currently (task 3 is in batch 2)
        //                      - this is due to limitations of what task graph can do without having a proper render-graph
        //                      - will be fixed in the future by adding JIRA
        //      Batch 1:
        //          Task 1
        //          Task 3
        //      Batch 2:
        //          Task 2
        daxa::Instance daxa_ctx = daxa::create_instance({});
        daxa::Device device = daxa_ctx.create_device({
            .name = "device",
        });
        auto image = device.create_image({
            .size = {1, 1, 1},
            .array_layer_count = 1,
            .usage = daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
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

        auto task_graph = daxa::TaskGraph({
            .device = device,
            .record_debug_information = true,
            .name = "task_graph",
        });

        task_graph.use_persistent_image(persistent_task_image);
        task_graph.use_persistent_buffer(persistent_task_buffer);
        task_graph.add_task({
            .uses = {daxa::TaskImageUse<daxa::TaskImageAccess::GRAPHICS_SHADER_STORAGE_WRITE_ONLY>{persistent_task_image}},
            .task = [&](daxa::TaskInterface const &) {},
            .name = "write persistent image",
        });
        task_graph.add_task({
            .uses = {
                daxa::TaskBufferUse<daxa::TaskBufferAccess::GRAPHICS_SHADER_READ>{persistent_task_buffer},
                daxa::TaskImageUse<daxa::TaskImageAccess::GRAPHICS_SHADER_SAMPLED>{persistent_task_image},
            },
            .task = [&](daxa::TaskInterface const &) {},
            .name = "read persistent image, read persistent buffer",
        });
        task_graph.add_task({
            .uses = {daxa::TaskBufferUse<daxa::TaskBufferAccess::GRAPHICS_SHADER_READ>{persistent_task_buffer}},
            .task = [&](daxa::TaskInterface const &) {},
            .name = "read persistent buffer",
        });
        task_graph.submit({});
        task_graph.complete({});

        task_graph.execute({});
        std::cout << task_graph.get_debug_string() << std::endl;

        device.wait_idle();
        device.destroy_image(image);
        device.destroy_buffer(buffer);
        device.collect_garbage();
    }
} // namespace tests

auto main() -> i32
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
    // tests::transient_write_aliasing();
    // tests::transient_resources();
    // tests::shader_integration_inl_use();
    tests::mipmapping();
}
