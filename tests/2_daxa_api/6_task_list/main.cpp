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

        // command lists must be completed before submission!
        cmd_list.complete();

        app.device.submit_commands({
            .command_lists = {cmd_list},
        });
    }
} // namespace tests

int main()
{
    App app = {};
    tests::simplest(app);
}
