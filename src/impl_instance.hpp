#pragma once

#include "impl_core.hpp"

#include <daxa/c/instance.h>

struct daxa_ImplInstance final : daxa_ImplHandle
{
    daxa_InstanceInfo info = {};
    VkInstance vk_instance = {};
};
