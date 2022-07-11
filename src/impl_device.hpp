#pragma once

#include "impl_context.hpp"

#include <daxa/daxa.hpp>
#include <daxa/device.hpp>

namespace daxa
{
    struct ImplDevice
    {
        VkPhysicalDevice vk_physical_device = {};
        VkDevice vk_device_handle = {};
        VolkDeviceTable volk_device_table = {};
        VkQueue vk_main_queue_handle = {};
        u32 main_queue_family_index = {};
        std::shared_ptr<ImplContext> ctx = {};
        DeviceInfo info = {};

        ImplDevice(DeviceInfo const &info, std::shared_ptr<ImplContext> impl_ctx, VkPhysicalDevice physical_device);
        ~ImplDevice();
    };
} // namespace daxa
