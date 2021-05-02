#include "Daxa.hpp"
#include "../Daxa/src/util/Timing.hpp"
#include "../Daxa/src/entity/ECS.hpp"

#include <iostream>

int main(int argc, char* args[])
{
    daxa::initialize();

    std::cout << "sizeof image slot: " << sizeof(daxa::ImageManager::ImageSlot) * (1 << 16) << std::endl;

    struct type1 {
        u32 i;
    };

    struct type2 {
        std::string str;
    };

    daxa::ECSArchetypeStorage<type1> tester{ .typeIndexMapping = std::array{0} };

    std::vector<type1>* arr = reinterpret_cast<std::vector<type1>*>(tester.get(0));

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