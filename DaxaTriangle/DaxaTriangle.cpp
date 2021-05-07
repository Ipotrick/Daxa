#include "Daxa.hpp"
#include "../Daxa/src/util/Timing.hpp"
#include "../Daxa/src/entity/ECS.hpp"

#include <iostream>
#include <atomic>

int main(int argc, char* args[])
{
    daxa::initialize();
    {
        daxa::OwningMutex<daxa::ImageManager> imagesMtx;

        daxa::Application app{ "Test", 1000, 1000 };

        bool running{ true };
        daxa::StopWatch timer;
        while (running) {
            timer.stop();
            f32 dt = timer.getSecs();
            timer.clear();
            timer.start();


            imagesMtx.lock()->update();

            app.update(dt);
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