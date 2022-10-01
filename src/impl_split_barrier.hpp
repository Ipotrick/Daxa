#pragma once

#include <daxa/split_barrier.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct SplitBarrierZombie
    {
        VkEvent vk_event = {};
    };
} // namespace daxa
