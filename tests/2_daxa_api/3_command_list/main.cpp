#include <daxa/daxa.hpp>
#include <iostream>

struct App
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = true,
    });
    daxa::Device device = daxa_ctx.create_device({});
};

namespace tests
{
    using namespace daxa::types;

    void simplest(App & app)
    {
        auto cmd_list = app.device.create_command_list({});

        // command lists must be completed before submission!
        cmd_list.complete();

        app.device.submit_commands({
            .command_lists = {cmd_list},
        });
    }

    void copy(App & app)
    {
        auto cmd_list = app.device.create_command_list({.debug_name = "copy command list"});

        std::array<f32, 4> data = {0.0f, 1.0f, 2.0f, 3.0f};

        daxa::BufferId staging_upload_buffer = app.device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
            .size = sizeof(decltype(data)),
            .debug_name = "staging_upload_buffer",
        });

        daxa::BufferId device_local_buffer = app.device.create_buffer({
            .size = sizeof(decltype(data)),
            .debug_name = "device_local_buffer",
        });

        daxa::BufferId staging_readback_buffer = app.device.create_buffer({
            .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .size = sizeof(decltype(data)),
            .debug_name = "staging_readback_buffer",
        });

        daxa::ImageId image_1 = app.device.create_image({
            .format = daxa::Format::R32G32B32A32_SFLOAT,
            .size = {1, 1, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_DST | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .debug_name = "image_1",
        });

        daxa::ImageId image_2 = app.device.create_image({
            .format = daxa::Format::R32G32B32A32_SFLOAT,
            .size = {1, 1, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_DST | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .debug_name = "image_2",
        });

        daxa::TimelineQueryPool timeline_query_pool = app.device.create_timeline_query_pool({
            .query_count = 2,
            .debug_name = "timeline_query",
        });

        auto buffer_ptr = app.device.get_host_address_as<std::array<f32, 4>>(staging_upload_buffer);

        *buffer_ptr = data;

        cmd_list.reset_timestamps({
            .query_pool = timeline_query_pool,
            .start_index = 0,
            .count = timeline_query_pool.info().query_count,
        });

        cmd_list.write_timestamp({
            .query_pool = timeline_query_pool,
            .pipeline_stage = daxa::PipelineStageFlagBits::BOTTOM_OF_PIPE,
            .query_index = 0,
        });

        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::HOST_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = staging_upload_buffer,
            .dst_buffer = device_local_buffer,
            .size = sizeof(decltype(data)),
        });

        // Barrier to make sure device_local_buffer is has no read after write hazard.
        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_id = image_1,
        });

        cmd_list.copy_buffer_to_image({
            .buffer = device_local_buffer,
            .image = image_1,
            .image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_extent = {1, 1, 1},
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
            .after_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .image_id = image_1,
        });

        cmd_list.pipeline_barrier_image_transition({
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .after_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_id = image_2,
        });

        cmd_list.copy_image_to_image({
            .src_image = image_1,
            .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .dst_image = image_2,
            .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .extent = {1, 1, 1},
        });

        cmd_list.pipeline_barrier_image_transition({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
            .after_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .image_id = image_2,
        });

        // Barrier to make sure device_local_buffer is has no write after read hazard.
        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
        });

        cmd_list.copy_image_to_buffer({
            .buffer = device_local_buffer,
            .image = image_2,
            .image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .image_extent = {1, 1, 1},
        });

        // Barrier to make sure device_local_buffer is has no read after write hazard.
        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = device_local_buffer,
            .dst_buffer = staging_readback_buffer,
            .size = sizeof(decltype(data)),
        });

        // Barrier to make sure staging_readback_buffer is has no read after write hazard.
        cmd_list.pipeline_barrier({
            .awaited_pipeline_access = daxa::AccessConsts::TRANSFER_WRITE,
            .waiting_pipeline_access = daxa::AccessConsts::HOST_READ,
        });

        cmd_list.write_timestamp({
            .query_pool = timeline_query_pool,
            .pipeline_stage = daxa::PipelineStageFlagBits::BOTTOM_OF_PIPE,
            .query_index = 1,
        });

        cmd_list.complete();

        app.device.submit_commands({
            .command_lists = {cmd_list},
        });

        app.device.wait_idle();

        auto query_results = timeline_query_pool.get_query_results(0, 2);
        if (query_results[1] && query_results[3])
        {
            std::cout << "gpu execution took " << static_cast<f64>(query_results[2] - query_results[0]) / 1000000.0 << " ms" << std::endl;
        }

        std::array<f32, 4> readback_data = *app.device.get_host_address_as<std::array<f32, 4>>(staging_readback_buffer);

        std::cout << "Original data: " << data[0] << ' ' << data[1] << ' ' << data[2] << ' ' << data[3] << std::endl;
        std::cout << "Readback data: " << readback_data[0] << ' ' << readback_data[1] << ' ' << readback_data[2] << ' ' << readback_data[3] << std::endl;

        for (usize i = 0; i < 4; ++i)
        {
            DAXA_DBG_ASSERT_TRUE_M(data[i] == readback_data[i], "readback data differs from upload data");
        }

        app.device.destroy_buffer(staging_upload_buffer);
        app.device.destroy_buffer(device_local_buffer);
        app.device.destroy_buffer(staging_readback_buffer);
        app.device.destroy_image(image_1);
        app.device.destroy_image(image_2);

        app.device.collect_garbage();
    }

    void deferred_destruction(App & app)
    {
        auto cmd_list = app.device.create_command_list({.debug_name = "deferred_destruction command list"});

        daxa::BufferId buffer = app.device.create_buffer({.size = 4});
        daxa::ImageId image = app.device.create_image({
            .size = {1, 1, 1},
            .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT,
        });
        daxa::ImageViewId image_view = app.device.create_image_view({.image = image});
        daxa::SamplerId sampler = app.device.create_sampler({});

        // The gpu resources are not destroyed here. Their destruction is deferred until the command list completes execution on the gpu.
        cmd_list.destroy_buffer_deferred(buffer);
        cmd_list.destroy_image_deferred(image);
        cmd_list.destroy_image_view_deferred(image_view);
        cmd_list.destroy_sampler_deferred(sampler);

        // The gpu resources are still alive, as long as this command list is not submitted and has not finished execution.
        cmd_list.complete();

        // Even after this call the resources will still be alive, as zombie resources are not checked to be dead in submit calls.
        app.device.submit_commands({
            .command_lists = {cmd_list},
        });

        app.device.wait_idle();

        // Here the gpu resources will be destroyed.
        // Collect_garbage loops over all zombie resources and destroys them when they are no longer used on the gpu/ their associated command list finished executing.
        app.device.collect_garbage();
    }
} // namespace tests

int main()
{
    App app = {};
    tests::simplest(app);
    tests::copy(app);
    tests::deferred_destruction(app);
}
