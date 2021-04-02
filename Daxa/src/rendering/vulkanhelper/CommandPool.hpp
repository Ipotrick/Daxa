#pragma once

#include "../Vulkan.hpp"

#include "Pool.hpp"

namespace daxa {
    namespace vkh {
        class DeferedDestructionQueue;

        VkCommandPool makeCommandPool(
            u32 queueFamilyIndex = daxa::vkh::mainGraphicsQueueFamiltyIndex,
            VkCommandPoolCreateFlagBits flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            VkDevice device = daxa::vkh::mainDevice);

        class CommandPool {
        public:
            CommandPool(
                u32 queueFamilyIndex = daxa::vkh::mainGraphicsQueueFamiltyIndex,
                VkCommandPoolCreateFlagBits flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                VkDevice device = daxa::vkh::mainDevice
            );

            CommandPool(CommandPool&& other) noexcept;

            ~CommandPool();

            operator const VkCommandPool&();

            const VkCommandPool& get();

            VkCommandBuffer getBuffer();

            void flush();

        private:
            u32 queueFamilyIndex{ 0xFFFFFFFF };
            VkDevice device{ VK_NULL_HANDLE };
            VkCommandPool pool{ VK_NULL_HANDLE };
            Pool<VkCommandBuffer> bufferPool;
        };
    }
}
