#include "Daxa.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <iostream>

int main(int argc, char* args[])
{
    daxa::initialize();

    {
        daxa::Application app{ "Test", 512, 256 };

        bool running{ true };
        while (running) {



            app.draw();


            if (app.windowMutex->lock()->update(0.01f)) {
                running = false;
                continue;
            }
        }
    }

    daxa::cleanup();

    return 0;
}