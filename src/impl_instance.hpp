#pragma once

#include "impl_core.hpp"

#include <daxa/c/instance.h>

struct daxa_ImplInstance final : ImplHandle
{
    InstanceInfo info = {};
    std::string engine_name = {};
    std::string app_name = {};
    VkInstance vk_instance = {};

    static void zero_ref_callback(ImplHandle * handle);
};