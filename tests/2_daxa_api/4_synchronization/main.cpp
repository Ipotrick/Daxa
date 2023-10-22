#include <daxa/daxa.hpp>

struct App
{
    daxa::Instance daxa_ctx = daxa::create_instance({});
    daxa::Device device = daxa_ctx.create_device({});
};

namespace tests
{
    using namespace daxa::types;

    void binary_semaphore(App & app)
    {
        auto encoder = app.device.create_command_encoder({});
        auto commands1 = encoder.complete_current_commands();
        auto commands2 = encoder.complete_current_commands();
        auto commands3 = encoder.complete_current_commands();
        auto commands4 = encoder.complete_current_commands();
        encoder.~CommandEncoder();

        auto binary_semaphore_1 = app.device.create_binary_semaphore({});
        auto binary_semaphore_2 = app.device.create_binary_semaphore({});

        // This semaphore is useful in the future, it can be used to make submits wait on each other,
        // or to make a present wait for a submit to finish
        app.device.submit_commands({
            .commands = std::array{commands1},
            .signal_binary_semaphores = std::array{binary_semaphore_1},
        });

        app.device.submit_commands({
            .commands = std::array{commands2},
            .wait_binary_semaphores = std::array{binary_semaphore_1},
            .signal_binary_semaphores = std::array{binary_semaphore_2},
        });

        // Binary semaphores can be reused ONLY after they have been signaled.
        app.device.submit_commands({
            .commands = std::array{commands3},
            .wait_binary_semaphores = std::array{binary_semaphore_2},
            .signal_binary_semaphores = std::array{binary_semaphore_1},
        });

        app.device.submit_commands({
            .commands = std::array{commands4},
            .wait_binary_semaphores = std::array{binary_semaphore_1},
        });
    }

    void memory_barriers(App & app)
    {
        auto encoder = app.device.create_command_encoder({});
        auto commands = encoder.complete_current_commands();

        app.device.submit_commands({
            .commands = std::array{commands},
        });
    }
} // namespace tests

auto main() -> int
{
    App app = {};
    tests::binary_semaphore(app);
    // Useless for now
    tests::memory_barriers(app);
}
