#include <daxa/daxa.hpp>
#include <iostream>

namespace tests
{
    using namespace daxa::types;

    void simplest(daxa::Instance & instance)
    {
        auto device = instance.create_device({});
    }
    void device_selection(daxa::Instance & instance)
    {
        try
        {
            // To select a device, you look at its properties and return a score.
            // Daxa will choose the device you scored as the highest.
            auto device = instance.create_device({
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
        catch (std::runtime_error error)
        {
            std::cout << "failed test \"device_selection\": " << error.what() << std::endl;
            exit(-1);
        }
    }
    constexpr daxa::BufferInfo test_buffer_info = {
        .size = 64,
        .name = "test buffer",
    };
    constexpr daxa::ImageInfo test_image_info = {
        .size = {64, 64, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_STORAGE,
        .name = "test image",
    };
    void sro_creation(daxa::Instance & instance)
    {
        try
        {
            auto device = instance.create_device({.flags = daxa::DeviceFlags2{.ray_tracing = 1}});
            auto test_buffer = device.create_buffer(test_buffer_info);
            auto test_image = device.create_image(test_image_info);
            auto test_image_view = device.create_image_view({
                .type = daxa::ImageViewType::REGULAR_2D_ARRAY,
                .image = test_image,
                .name = "test image view",
            });
            auto test_sampler = device.create_sampler({
                .name = "test sampler",
            });
            device.destroy_sampler(test_sampler);
            device.destroy_image_view(test_image_view);
            device.destroy_image(test_image);
            device.destroy_buffer(test_buffer);
        }
        catch (std::runtime_error error)
        {
            std::cout << "failed test \"sro_creation\": " << error.what() << std::endl;
            exit(-1);
        }
    }
    void sro_aliased_suballocation(daxa::Instance & instance)
    {
        try
        {
            auto device = instance.create_device({.flags = daxa::DeviceFlags2{.ray_tracing = 1}});
            auto test_buffer_mem_req = device.get_memory_requirements(test_buffer_info);
            auto test_image_mem_req = device.get_memory_requirements(test_image_info);
            auto pessimised_mem_req = daxa::MemoryRequirements{
                .size = std::max(test_buffer_mem_req.size, test_image_mem_req.size),
                .alignment = std::max(test_buffer_mem_req.alignment, test_image_mem_req.alignment),
                .memory_type_bits = test_buffer_mem_req.memory_type_bits & test_image_mem_req.memory_type_bits,
            };
            auto test_memory_block = device.create_memory({
                .requirements = pessimised_mem_req,
            });
            auto memory_block_test_buffer = device.create_buffer_from_memory_block({
                .buffer_info = test_buffer_info,
                .memory_block = test_memory_block,
            });
            auto memory_block_test_image = device.create_image_from_memory_block({
                .image_info = test_image_info,
                .memory_block = test_memory_block,
            });
            device.destroy_image(memory_block_test_image);
            device.destroy_buffer(memory_block_test_buffer);
        }
        catch (std::runtime_error error)
        {
            std::cout << "failed test \"sro_aliased_suballocation\": " << error.what() << std::endl;
            exit(-1);
        }
    }
    void acceleration_structure_creation(daxa::Instance & instance)
    {
        try
        {
            daxa::Device device;
            try{
                device = instance.create_device({
                    .flags = daxa::DeviceFlags2{.ray_tracing = 1},
                    .selector = [](daxa::DeviceProperties const & prop) -> i32
                    {
                        auto default_value = daxa::default_device_score(prop);
                        return prop.ray_tracing_properties.has_value() ? default_value : -1;
                    }
                });
            }
            catch(std::runtime_error error)
            {
                std::cout << "Test skipped. Device does not support raytracing!" << std::endl;
                return;
            }
            std::cout << "Device supports raytracing!" << std::endl;

            auto test_buffer_mem_req = device.get_memory_requirements(test_buffer_info);
            auto test_image_mem_req = device.get_memory_requirements(test_image_info);
            auto pessimised_mem_req = daxa::MemoryRequirements{
                .size = std::max(test_buffer_mem_req.size, test_image_mem_req.size),
                .alignment = std::max(test_buffer_mem_req.alignment, test_image_mem_req.alignment),
                .memory_type_bits = test_buffer_mem_req.memory_type_bits & test_image_mem_req.memory_type_bits,
            };
            auto test_memory_block = device.create_memory({
                .requirements = pessimised_mem_req,
            });
            auto memory_block_test_buffer = device.create_buffer_from_memory_block({
                .buffer_info = test_buffer_info,
                .memory_block = test_memory_block,
            });
            auto memory_block_test_image = device.create_image_from_memory_block({
                .image_info = test_image_info,
                .memory_block = test_memory_block,
            });
            device.destroy_image(memory_block_test_image);
            device.destroy_buffer(memory_block_test_buffer);
        }
        catch (std::runtime_error error)
        {
            std::cout << "failed test \"acceleration_structure_creation\": " << error.what() << std::endl;
            exit(-1);
        }
    }
} // namespace tests

auto main() -> int
{
    auto instance = daxa::create_instance({});
    tests::simplest(instance);
    tests::device_selection(instance);
    tests::sro_creation(instance);
    tests::sro_aliased_suballocation(instance);
    tests::acceleration_structure_creation(instance);
    std::cout << "completed all tests successfully!" << std::endl;
}
