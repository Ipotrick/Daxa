#pragma once

#include "../Vulkan.hpp"
#include "CommandPool.hpp"

namespace daxa {
    namespace vkh {
        VkCommandBuffer makeCommandBuffer(VkCommandPool pool, VkDevice device = vkh::mainDevice);

        VkCommandBufferBeginInfo makeCmdBeginInfo(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        class CommandBuffer {
        public:
            CommandBuffer(VkCommandPool pool, VkDevice device = vkh::mainDevice);

            CommandBuffer(CommandBuffer&& other) noexcept;

            ~CommandBuffer();

            operator const VkCommandBuffer&();

            const VkCommandBuffer& get();
        private:
            VkDevice device{ VK_NULL_HANDLE };
            VkCommandPool pool{ VK_NULL_HANDLE };
            VkCommandBuffer buffer{ VK_NULL_HANDLE };
        };
    }
}
