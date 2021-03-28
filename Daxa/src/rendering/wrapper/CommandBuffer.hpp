#pragma once

#include "../Vulkan.hpp"
#include "CommandPool.hpp"

namespace daxa {


    class CommandBuffer {
    public:
        CommandBuffer(VkCommandPool pool, VkDevice device);

        CommandBuffer(const CommandPool& pool);

        operator VkCommandBuffer();

        VkCommandBuffer buffer{ VK_NULL_HANDLE };
    };
}
