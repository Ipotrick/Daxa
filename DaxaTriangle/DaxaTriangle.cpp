#include "Daxa.hpp"
#include "../Daxa/src/util/Timing.hpp"
#include "../Daxa/src/entity/ECS.hpp"

#include <iostream>
#include <atomic>

class MyUser : public daxa::User {
    std::unique_ptr<daxa::Renderer> defaultRenderer;
public:
    virtual void init(daxa::AppState* appstate) override {
        defaultRenderer = std::make_unique<daxa::Renderer>(appstate->window);
        defaultRenderer->init();
    }

    virtual void update(daxa::AppState* appstate) override {
        std::cout << "delta time: " << std::setw(8) << appstate->getDeltaTimeSeconds() * 1000.0f << "ms, fps: " << 1.0f / (appstate->getDeltaTimeSeconds()) << std::endl;
        defaultRenderer->draw(appstate->getDeltaTimeSeconds());
    }

    virtual void deinit(daxa::AppState* appstate) override {
        defaultRenderer->deinit();
    }
};

int main(int argc, char* args[])
{
#if 1
    daxa::initialize();
    {
        daxa::Application app{ 1000, 1000, std::string("Test"), std::make_unique<MyUser>(MyUser{}) };

        app.run();
    }
    daxa::cleanup();

    return 0;
#else
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name ("Example Vulkan Application")
                        .request_validation_layers ()
                        .use_default_debug_messenger ()
                        .build ();
    if (!inst_ret) {
        std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
        return false;
    }
    vkb::Instance vkb_inst = inst_ret.value ();

    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    auto phys_ret = selector.set_surface (/* from user created window*/)
                        .set_minimum_version (1, 1) // require a vulkan 1.1 capable device
                        .require_dedicated_transfer_queue ()
                        .select ();
    if (!phys_ret) {
        std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";
        return false;
    }

    vkb::DeviceBuilder device_builder{ phys_ret.value () };
    // automatically propagate needed data from instance & physical device
    auto dev_ret = device_builder.build ();
    if (!dev_ret) {
        std::cerr << "Failed to create Vulkan device. Error: " << dev_ret.error().message() << "\n";
        return false;
    }
    vkb::Device vkb_device = dev_ret.value ();

    // Get the VkDevice handle used in the rest of a vulkan application
    VkDevice device = vkb_device.device;

    // Get the graphics queue with a helper function
    auto graphics_queue_ret = vkb_device.get_queue (vkb::QueueType::graphics);
    if (!graphics_queue_ret) {
        std::cerr << "Failed to get graphics queue. Error: " << graphics_queue_ret.error().message() << "\n";
        return false;
    }
    VkQueue graphics_queue = graphics_queue_ret.value ();

    // Turned 400-500 lines of boilerplate into less than fifty.
    return true;
#endif
}