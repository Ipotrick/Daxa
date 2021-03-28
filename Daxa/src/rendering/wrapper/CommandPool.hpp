#pragma once

#include "../Vulkan.hpp"

namespace daxa {

    class CommandPool {
    public:
        CommandPool(
            u32 queueFamilyIndex = daxa::vulkan::mainGraphicsQueueFamiltyIndex,
            VkDevice device = daxa::vulkan::mainDevice,
            VkCommandPoolCreateFlagBits flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        );

        ~CommandPool();

        operator VkCommandPool();

        u32 queueFamilyIndex{ 0xFFFFFFFF };
        VkDevice device{ VK_NULL_HANDLE };
        VkCommandPool pool{ VK_NULL_HANDLE };
    };
}
