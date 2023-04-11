#include <daxa/daxa.hpp>

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

    void binary_semaphore(App & app)
    {
        auto cmd_list1 = app.device.create_command_list({});
        cmd_list1.complete();

        auto cmd_list2 = app.device.create_command_list({});
        cmd_list2.complete();

        auto cmd_list3 = app.device.create_command_list({});
        cmd_list3.complete();

        auto cmd_list4 = app.device.create_command_list({});
        cmd_list4.complete();

        auto binary_semaphore_1 = app.device.create_binary_semaphore({});
        auto binary_semaphore_2 = app.device.create_binary_semaphore({});

        // This semaphore is useful in the future, it can be used to make submits wait on each other,
        // or to make a present wait for a submit to finish
        app.device.submit_commands({
            .command_lists = {cmd_list1},
            .signal_binary_semaphores = {binary_semaphore_1},
        });

        app.device.submit_commands({
            .command_lists = {cmd_list2},
            .wait_binary_semaphores = {binary_semaphore_1},
            .signal_binary_semaphores = {binary_semaphore_2},
        });

        // Binary semaphores can be reused ONLY after they have been signaled.
        app.device.submit_commands({
            .command_lists = {cmd_list3},
            .wait_binary_semaphores = {binary_semaphore_2},
            .signal_binary_semaphores = {binary_semaphore_1},
        });

        app.device.submit_commands({
            .command_lists = {cmd_list4},
            .wait_binary_semaphores = {binary_semaphore_1},
        });

        app.device.wait_idle();
        app.device.collect_garbage();
    }

    void memory_barriers(App & app)
    {
        auto cmd_list = app.device.create_command_list({});
        cmd_list.complete();

        app.device.submit_commands({
            .command_lists = {cmd_list},
        });

        app.device.wait_idle();
        app.device.collect_garbage();
    }
} // namespace tests

auto main() -> int
{
    App app = {};
    tests::binary_semaphore(app);
    // Useless for now
    tests::memory_barriers(app);
}
