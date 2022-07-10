#pragma once

#include <vulkan/vulkan.h>

#include <daxa/daxa.hpp>
#include <daxa/device.hpp>
#include "impl_context.hpp"

namespace daxa
{
    struct ImplDevice
    {
        VkDevice vk_device_handle = {};
        VkQueue vk_main_queue_handle = {};
        std::shared_ptr<ImplContext> ctx = {};
        DeviceInfo info = {};

        ImplDevice(std::shared_ptr<ImplContext> impl_ctx, VkPhysicalDevice physical_device);
        ~ImplDevice();
    };
} // namespace daxa
