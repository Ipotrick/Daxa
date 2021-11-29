#include "Daxa.hpp"
#include "../Daxa/src/util/Timing.hpp"
#include "../Daxa/src/entity/ECS.hpp"

#include <iostream>
#include <atomic>

int main(int argc, char* args[])
{
    daxa::initialize();
    {
        daxa::Application app{ 1000, 1000, std::string("Test"), nullptr };

        app.run();
    }
    daxa::cleanup();

    return 0;
}