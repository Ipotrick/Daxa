#include "Daxa.hpp"
#include "../Daxa/src/util/Timing.hpp"

#include <iostream>

int main(int argc, char* args[])
{
    daxa::initialize();

    std::cout << "sizeof image slot: " << sizeof(daxa::ImageManager::ImageSlot) << std::endl;

    {
        daxa::OwningMutex<daxa::ImageManager> imagesMtx;

        {
            auto handle = imagesMtx.lock()->getHandle("assets/Triangle.png");
        }
        auto handle = imagesMtx.lock()->getHandle("assets/Triangle.png");
        std::vector<decltype(imagesMtx.lock()->getHandle(""))> handles;
        handles.reserve(100'000);
        std::vector<decltype(imagesMtx.lock()->getHandle(""))> handlesCpy;
        handlesCpy.reserve(100'000);
        {
            daxa::LogWatch logwatch("create handles");

            for (i32 i = 0; i < 100'000; i++) {
                handles.push_back(handle);
            }
        }
        {
            daxa::LogWatch logwatch("copy handles");

            handlesCpy.insert(handlesCpy.end(), handles.begin(), handles.end());
        }
        {
            daxa::LogWatch logwatch("destroy handles");
            handles.clear();
        }

        daxa::Application app{ "Test", 512, 256 };

        bool running{ true };
        while (running) {
            daxa::LogWatch("deltatime");

            imagesMtx.lock()->update();

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