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

        app.device.wait_idle();
        app.device.collect_garbage();
    }

    template <usize SX, usize SY, usize SZ>
    using ImageArray = std::array<std::array<std::array<std::array<f32, 4>, SX>, SY>, SZ>;

    void copy(App & app)
    {
        auto cmd_list = app.device.create_command_list({.name = "copy command list"});

        constexpr u32 SIZE_X = 3;
        constexpr u32 SIZE_Y = 3;
        constexpr u32 SIZE_Z = 3;

        auto get_printable_char_buffer = []<usize SX, usize SY, usize SZ>(ImageArray<SX, SY, SZ> const & in_data) -> std::vector<char>
        {
            std::vector<char> data;
            constexpr auto pixel = std::to_array("\033[48;2;000;000;000m  ");
            constexpr auto line_terminator = std::to_array("\033[0m ");
            constexpr auto newline_terminator = std::to_array("\033[0m\n");
            data.resize(SX * SY * SZ * (pixel.size() - 1) + SY * SZ * (line_terminator.size() - 1) + SZ * (newline_terminator.size() - 1) + 1);
            usize output_index = 0;
            for (usize zi = 0; zi < SZ; ++zi)
            {
                for (usize yi = 0; yi < SY; ++yi)
                {
                    for (usize xi = 0; xi < SX; ++xi)
                    {
                        u8 const r = static_cast<u8>(in_data[zi][yi][xi][0] * 255.0f);
                        u8 const g = static_cast<u8>(in_data[zi][yi][xi][1] * 255.0f);
                        u8 const b = static_cast<u8>(in_data[zi][yi][xi][2] * 255.0f);
                        auto next_pixel = pixel;
                        next_pixel[7 + 0 * 4 + 0] = '0' + (r / 100);
                        next_pixel[7 + 0 * 4 + 1] = '0' + (r % 100) / 10;
                        next_pixel[7 + 0 * 4 + 2] = '0' + (r % 10);
                        next_pixel[7 + 1 * 4 + 0] = '0' + (g / 100);
                        next_pixel[7 + 1 * 4 + 1] = '0' + (g % 100) / 10;
                        next_pixel[7 + 1 * 4 + 2] = '0' + (g % 10);
                        next_pixel[7 + 2 * 4 + 0] = '0' + (b / 100);
                        next_pixel[7 + 2 * 4 + 1] = '0' + (b % 100) / 10;
                        next_pixel[7 + 2 * 4 + 2] = '0' + (b % 10);
                        std::copy(next_pixel.begin(), next_pixel.end() - 1, data.data() + output_index);
                        output_index += pixel.size() - 1;
                    }
                    std::copy(line_terminator.begin(), line_terminator.end() - 1, data.data() + output_index);
                    output_index += line_terminator.size() - 1;
                }
                std::copy(newline_terminator.begin(), newline_terminator.end() - 1, data.data() + output_index);
                output_index += newline_terminator.size() - 1;
            }
            data[data.size() - 1] = '\0';
            return data;
        };

        auto data = ImageArray<SIZE_X, SIZE_Y, SIZE_Z>{};

        for (usize zi = 0; zi < SIZE_Z; ++zi)
        {
            for (usize yi = 0; yi < SIZE_Y; ++yi)
            {
                for (usize xi = 0; xi < SIZE_X; ++xi)
                {
                    data[zi][yi][xi][0] = static_cast<f32>(xi) / static_cast<f32>(SIZE_X - 1);
                    data[zi][yi][xi][1] = static_cast<f32>(yi) / static_cast<f32>(SIZE_Y - 1);
                    data[zi][yi][xi][2] = static_cast<f32>(zi) / static_cast<f32>(SIZE_Z - 1);
                    data[zi][yi][xi][3] = 1.0f;
                }
            }
        }

        daxa::BufferId const staging_upload_buffer = app.device.create_buffer({
            .size = sizeof(decltype(data)),
            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
            .name = "staging_upload_buffer",
        });

        daxa::BufferId const device_local_buffer = app.device.create_buffer({
            .size = sizeof(decltype(data)),
            .name = "device_local_buffer",
        });

        daxa::BufferId const staging_readback_buffer = app.device.create_buffer({
            .size = sizeof(decltype(data)),
            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
            .name = "staging_readback_buffer",
        });

        daxa::ImageId const image_1 = app.device.create_image({
            .dimensions = 2 + static_cast<u32>(SIZE_Z > 1),
            .format = daxa::Format::R32G32B32A32_SFLOAT,
            .size = {SIZE_X, SIZE_Y, SIZE_Z},
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_DST | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .name = "image_1",
        });

        daxa::ImageId const image_2 = app.device.create_image({
            .dimensions = 2 + static_cast<u32>(SIZE_Z > 1),
            .format = daxa::Format::R32G32B32A32_SFLOAT,
            .size = {SIZE_X, SIZE_Y, SIZE_Z},
            .usage = daxa::ImageUsageFlagBits::SHADER_READ_WRITE | daxa::ImageUsageFlagBits::TRANSFER_DST | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .name = "image_2",
        });

        daxa::TimelineQueryPool timeline_query_pool = app.device.create_timeline_query_pool({
            .query_count = 2,
            .name = "timeline_query",
        });

        auto & buffer_ptr = *app.device.get_host_address_as<ImageArray<SIZE_X, SIZE_Y, SIZE_Z>>(staging_upload_buffer);

        buffer_ptr = data;

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
            .src_access = daxa::AccessConsts::HOST_WRITE,
            .dst_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = staging_upload_buffer,
            .dst_buffer = device_local_buffer,
            .size = sizeof(decltype(data)),
        });

        // Barrier to make sure device_local_buffer is has no read after write hazard.
        cmd_list.pipeline_barrier({
            .src_access = daxa::AccessConsts::TRANSFER_WRITE,
            .dst_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.pipeline_barrier_image_transition({
            .src_access = daxa::AccessConsts::TRANSFER_WRITE,
            .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
            .dst_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_id = image_1,
        });

        cmd_list.copy_buffer_to_image({
            .buffer = device_local_buffer,
            .image = image_1,
            .image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_extent = {SIZE_X, SIZE_Y, SIZE_Z},
        });

        cmd_list.pipeline_barrier_image_transition({
            .src_access = daxa::AccessConsts::TRANSFER_WRITE,
            .dst_access = daxa::AccessConsts::TRANSFER_READ,
            .dst_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .image_id = image_1,
        });

        cmd_list.pipeline_barrier_image_transition({
            .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
            .dst_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .image_id = image_2,
        });

        cmd_list.copy_image_to_image({
            .src_image = image_1,
            .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .dst_image = image_2,
            .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
            .extent = {SIZE_X, SIZE_Y, SIZE_Z},
        });

        cmd_list.pipeline_barrier_image_transition({
            .src_access = daxa::AccessConsts::TRANSFER_WRITE,
            .dst_access = daxa::AccessConsts::TRANSFER_READ,
            .dst_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .image_id = image_2,
        });

        // Barrier to make sure device_local_buffer is has no write after read hazard.
        cmd_list.pipeline_barrier({
            .src_access = daxa::AccessConsts::TRANSFER_READ,
            .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
        });

        cmd_list.copy_image_to_buffer({
            .image = image_2,
            .image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
            .image_extent = {SIZE_X, SIZE_Y, SIZE_Z},
            .buffer = device_local_buffer,
        });

        // Barrier to make sure device_local_buffer is has no read after write hazard.
        cmd_list.pipeline_barrier({
            .src_access = daxa::AccessConsts::TRANSFER_WRITE,
            .dst_access = daxa::AccessConsts::TRANSFER_READ,
        });

        cmd_list.copy_buffer_to_buffer({
            .src_buffer = device_local_buffer,
            .dst_buffer = staging_readback_buffer,
            .size = sizeof(decltype(data)),
        });

        // Barrier to make sure staging_readback_buffer is has no read after write hazard.
        cmd_list.pipeline_barrier({
            .src_access = daxa::AccessConsts::TRANSFER_WRITE,
            .dst_access = daxa::AccessConsts::HOST_READ,
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
        if ((query_results[1] != 0u) && (query_results[3] != 0u))
        {
            std::cout << "gpu execution took " << static_cast<f64>(query_results[2] - query_results[0]) / 1000000.0 << " ms" << std::endl;
        }

        auto const & readback_data = *app.device.get_host_address_as<ImageArray<SIZE_X, SIZE_Y, SIZE_Z>>(staging_readback_buffer);

        std::cout << "Original data: " << std::endl;
        {
            auto printable_buffer = get_printable_char_buffer(data);
            std::cout << printable_buffer.data();
        }

        std::cout << "Readback data: " << std::endl;
        {
            auto printable_buffer = get_printable_char_buffer(readback_data);
            std::cout << printable_buffer.data();
        }

        for (usize zi = 0; zi < SIZE_Z; ++zi)
        {
            for (usize yi = 0; yi < SIZE_Y; ++yi)
            {
                for (usize xi = 0; xi < SIZE_X; ++xi)
                {
                    for (usize ci = 0; ci < 4; ++ci)
                    {
                        DAXA_DBG_ASSERT_TRUE_M(data[zi][yi][xi][ci] == readback_data[zi][yi][xi][ci], "readback data differs from upload data");
                    }
                }
            }
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
        auto cmd_list = app.device.create_command_list({.name = "deferred_destruction command list"});

        daxa::BufferId const buffer = app.device.create_buffer({.size = 4});
        daxa::ImageId const image = app.device.create_image({
            .size = {1, 1, 1},
            .usage = daxa::ImageUsageFlagBits::COLOR_ATTACHMENT,
        });
        daxa::ImageViewId const image_view = app.device.create_image_view({.image = image});
        daxa::SamplerId const sampler = app.device.create_sampler({});

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

auto main() -> int
{
    App app = {};
    tests::simplest(app);
    // tests::copy(app);
    // tests::deferred_destruction(app);
}
