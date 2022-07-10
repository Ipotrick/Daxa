#pragma once

#include <vulkan/vulkan.h>

#include <daxa/daxa.hpp>
#include <daxa/device.hpp>
#include "impl_context.hpp"

namespace daxa
{
    struct ImplDevice
    {
        VkDevice vk_device_handle;
        std::shared_ptr<ImplContext> ctx;
        
        ImplDevice();
        ~ImplDevice();
    };
}
