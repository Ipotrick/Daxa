#include <daxa/daxa.hpp>

struct App
{
    daxa::Context daxa_ctx = daxa::create_context({});
    daxa::Device device = daxa_ctx.create_default_device();
};

namespace tests
{
    using namespace daxa::types;

    void simplest(App & app)
    {
        auto cmd_list = app.device.create_command_list({});

        auto binary_semaphore = app.device.create_binary_semaphore({});

        cmd_list.complete();

        // This semaphore is useful in the future, it will be used to
        // notify the swapchain that it should wait for this semaphore
        // before presenting (this is used when the command list interacts
        // with the swapchain, ie. clearing the surface!)
        app.device.submit_commands({
            .command_lists = {cmd_list},
            .signal_binary_semaphores_on_completion = {binary_semaphore},
        });
    }
} // namespace tests

int main()
{
    App app = {};
    tests::simplest(app);
}
