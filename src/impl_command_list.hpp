#pragma once

#include <daxa/swapchain.hpp>

#include "impl_core.hpp"
#include "impl_device.hpp"

namespace daxa
{
    struct ImplCommandList
    {
        std::shared_ptr<ImplDevice> impl_device = {};
        CommandListInfo info = {};
        VkCommandBuffer vk_cmd_buffer_handle = {};
        VkCommandPool vk_cmd_pool_handle = {};
        bool recording_complete = true;

        ImplCommandList(std::shared_ptr<ImplDevice> device_impl, CommandListInfo const & info);
        ~ImplCommandList();

        void init();
    };
} // namespace daxa
