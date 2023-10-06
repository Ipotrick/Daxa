#pragma once

#include <daxa/c/instance.h>
#include <atomic>

struct daxa_ImplInstance
{
    std::atomic_uint64_t ref_count = {};
    daxa_InstanceInfo info = {};
    VkInstance vk_instance = {};
};
