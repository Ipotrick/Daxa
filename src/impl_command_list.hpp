#pragma once

#include <daxa/command_list.hpp>

#include "impl_core.hpp"
#include "impl_semaphore.hpp"

namespace daxa
{
    struct ImplDevice;

    struct ImplCommandList
    {
        using InfoT = CommandListInfo;

        std::weak_ptr<ImplDevice> impl_device = {};
        CommandListInfo info = {};
        VkCommandBuffer vk_cmd_buffer_handle = {};
        VkCommandPool vk_cmd_pool_handle = {};
        bool recording_complete = true;

        std::vector<std::shared_ptr<ImplBinarySemaphore>> used_binary_semaphores = {};

        ImplCommandList(std::weak_ptr<ImplDevice> device_impl);
        ~ImplCommandList();

        void initialize(CommandListInfo const & a_info);
        void reset();
    };
} // namespace daxa
