#pragma once

#include "../Vulkan.hpp"
#include "CommandPool.hpp"

namespace daxa {
    namespace vk {
        class CommandBuffer {
        public:
            CommandBuffer(VkCommandPool pool, VkDevice device = vk::mainDevice);

            CommandBuffer(CommandBuffer&& other) noexcept;

            operator const VkCommandBuffer&();

            const VkCommandBuffer& get();
        private:
            VkDevice device{ VK_NULL_HANDLE };
            VkCommandPool pool{ VK_NULL_HANDLE };
            VkCommandBuffer buffer{ VK_NULL_HANDLE };
        };
    }
}
