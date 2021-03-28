#include "Daxa.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <iostream>

int main(int argc, char* args[])
{
    daxa::initialize();

    daxa::Application app{ "Test", 256, 256 };

    daxa::CommandPool cmdPool{};

    daxa::CommandBuffer cmd{ cmdPool };
    
    vkBeginCommandBuffer(cmd, ...);

    vkCmdBeginRenderPass(cmd, ...);

    //rendering commands go here

    vkCmdEndRenderPass(cmd);

    vkEndCommandBuffer(cmd)


    bool running{ true };
    while (running) {
        auto window = app.windowMutex->lock();
        if (window->update(0.01f)) {
            running = false;
            continue;
        }
    }

    using namespace std::chrono_literals;

    daxa::cleanup();

    return 0;
}