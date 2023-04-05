#include <daxa/daxa.hpp>
#include <iostream>

namespace tests
{
    using namespace daxa::types;

    void simplest(daxa::Context & daxa_ctx)
    {
        auto device = daxa_ctx.create_device({});
    }
    void device_selection(daxa::Context & daxa_ctx)
    {
        // To select a device, you look at its properties and return a score.
        // Daxa will choose the device you scored as the highest.
        auto device = daxa_ctx.create_device({
            .selector = [](daxa::DeviceProperties const & device_props)
            {
                i32 score = 0;
                switch (device_props.device_type)
                {
                case daxa::DeviceType::DISCRETE_GPU: score += 10000; break;
                case daxa::DeviceType::VIRTUAL_GPU: score += 1000; break;
                case daxa::DeviceType::INTEGRATED_GPU: score += 100; break;
                default: break;
                }
                return score;
            },
            .name = "My device",
        });

        // once the device is created, you can query its properties, such
        // as its name and much more! These are the same properties we used
        // to discriminate in the GPU selection.
        std::cout << device.properties().device_name << std::endl;
    }
} // namespace tests

auto main() -> int
{
    auto daxa_ctx = daxa::create_context({
        .enable_validation = true,
    });

    tests::simplest(daxa_ctx);
    tests::device_selection(daxa_ctx);
}
