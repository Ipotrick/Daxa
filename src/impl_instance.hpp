#pragma once

#include <daxa/c/instance.h>

struct daxa_ImplInstance
{
    daxa_InstanceInfo info = {};
    VkInstance vk_instance = {};
};
