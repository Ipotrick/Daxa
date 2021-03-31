#pragma once

#include "../Vulkan.hpp"

namespace daxa {
    namespace vk {
        class CommandPool {
        public:
            CommandPool(
                u32 queueFamilyIndex = daxa::vk::mainGraphicsQueueFamiltyIndex,
                VkCommandPoolCreateFlagBits flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                VkDevice device = daxa::vk::mainDevice
            );

            CommandPool(CommandPool&& other) noexcept;

            ~CommandPool();

            operator const VkCommandPool&();

            const VkCommandPool& get();
        private:
            u32 queueFamilyIndex{ 0xFFFFFFFF };
            VkDevice device{ VK_NULL_HANDLE };
            VkCommandPool pool{ VK_NULL_HANDLE };
        };
    }
}
