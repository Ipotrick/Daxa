#pragma once

#include <daxa/daxa.hpp>
#include <daxa/device.hpp>

#include <daxa/split_barrier.hpp>

namespace daxa
{
    struct GPUEventZombie
    {
        VkEvent vk_event = {};
    };
}