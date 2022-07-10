#include <daxa/daxa.hpp>

int main()
{
    auto daxa_ctx = daxa::create_context({.enable_validation = false});

    // auto device_selector = [](daxa::DeviceInfo const & device_info) -> daxa::types::i32
    // {
    //     if (device_info.device_type != daxa::DeviceType::DISCRETE_GPU)
    //     {
    //         return -1;
    //     }
    //     return device_info.limits.max_memory_allocation_count;
    // };
    // auto device = daxa_ctx.create_device(device_selector).value();

    auto device = daxa_ctx.create_default_device();

}
