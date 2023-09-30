#pragma once

#include <daxa/sync.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct SplitBarrierZombie
    {
        VkEvent vk_event = {};
    };
} // namespace daxa
