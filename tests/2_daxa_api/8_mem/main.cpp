#include <daxa/daxa.hpp>
using namespace daxa::types;

#include <daxa/utils/mem.hpp>

#include <iostream>

static inline constexpr usize ITERATION_COUNT = {10000};

auto main() -> int
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = false,
    });
    daxa::Device device = daxa_ctx.create_device({
        .debug_name = "device",
    });
    daxa::TransferMemoryPool tmem{daxa::TransferMemoryPoolInfo{
        .device = device,
        .capacity = 1024,
        .debug_name = "transient memory pool",
    }};
    daxa::TimelineSemaphore gpu_timeline = device.create_timeline_semaphore({
        .debug_name = "timeline semaphpore",
    });
    usize cpu_timeline = 1;

    for (usize iteration = 0; iteration < ITERATION_COUNT; ++iteration)
    {
        gpu_timeline.wait_for_value(cpu_timeline - 1);
        [[maybe_unused]] auto alloc = tmem.allocate(37).value();
        daxa::CommandList cmd = device.create_command_list({});
        cmd.complete();
        device.submit_commands({
            .command_lists{std::move(cmd)},
            .signal_timeline_semaphores = {
                {gpu_timeline, cpu_timeline},
                {tmem.get_timeline_semaphore(), tmem.timeline_value()},
            },
        });
        cpu_timeline += 1;
    }
    device.wait_idle();
    device.collect_garbage();

    std::cout << "Success!" << std::endl;
}
