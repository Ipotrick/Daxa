#include <daxa/daxa.hpp>

struct App
{
    daxa::Context daxa_ctx = daxa::create_context({});
    daxa::Device device = daxa_ctx.create_default_device();
};

namespace tests
{
    using namespace daxa::types;

    void binary_semaphore(App & app)
    {
        auto cmd_list = app.device.create_command_list({});

        cmd_list.complete();

        auto binary_semaphore_1 = app.device.create_binary_semaphore({});
        auto binary_semaphore_2 = app.device.create_binary_semaphore({});

        // This semaphore is useful in the future, it can be used to make submits wait on each other, 
        // or to make a present wait for a submit to finish
        app.device.submit_commands({
            .command_lists = {cmd_list},
            .signal_binary_semaphores = {binary_semaphore_1},
        });

        app.device.submit_commands({
            .command_lists = {cmd_list},
            .wait_binary_semaphores = {binary_semaphore_1},
            .signal_binary_semaphores = {binary_semaphore_2}
        });

        // Binary semaphores can be reused ONLY after they have been signaled.
        app.device.submit_commands({
            .command_lists = {cmd_list},
            .wait_binary_semaphores = {binary_semaphore_2},
            .signal_binary_semaphores = {binary_semaphore_1}
        });

        app.device.submit_commands({
            .command_lists = {cmd_list},
            .wait_binary_semaphores = {binary_semaphore_1},
        });
    }

    void memory_barriers(App & app)
    {
        auto cmd_list = app.device.create_command_list({});
        cmd_list.complete();

        // This semaphore is useful in the future, it will be used to
        // notify the swapchain that it should wait for this semaphore
        // before presenting (this is used when the command list interacts
        // with the swapchain, ie. clearing the surface!)
        app.device.submit_commands({
            .command_lists = {cmd_list},
        });
    }
} // namespace tests

int main()
{
    App app = {};
    tests::binary_semaphore(app);
}
