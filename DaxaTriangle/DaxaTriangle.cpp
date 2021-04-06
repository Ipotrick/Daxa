#include "Daxa.hpp"

#include <iostream>

int main(int argc, char* args[])
{
    daxa::initialize();

    {
        daxa::OwningMutex<daxa::ImageManager> imagesMtx;

        {
            auto handle = imagesMtx.lock()->getHandle("assets/Triangle.png");
        }

        daxa::Application app{ "Test", 512, 256 };

        bool running{ true };
        while (running) {
            imagesMtx.lock()->update();

            {
                auto handle2 = imagesMtx.lock()->getHandle("assets/Triangle.png");
            }

            daxa::Jobs::orphan(daxa::Jobs::schedule(daxa::ImageManager::CreateJob(&imagesMtx)));
            daxa::Jobs::orphan(daxa::Jobs::schedule(daxa::ImageManager::DestroyJob(&imagesMtx)));

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