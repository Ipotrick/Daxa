#pragma once

#include "CommandPool.hpp"

namespace daxa {
    CommandPool::CommandPool(
        u32 queueFamilyIndex = daxa::vulkan::mainGraphicsQueueFamiltyIndex,
        VkDevice device = daxa::vulkan::mainDevice,
        VkCommandPoolCreateFlagBits flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
    ) :
        queueFamilyIndex{ queueFamilyIndex },
        device{ device }
    {
        VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
        cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.pNext = nullptr;
        cmdPoolCreateInfo.flags = flags;   // we can reset individual command buffers from this pool
        cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

        vkCreateCommandPool(daxa::vulkan::mainDevice, &cmdPoolCreateInfo, nullptr, &pool);
    }

    CommandPool::~CommandPool()
    {
        vkDestroyCommandPool(device, pool, nullptr);
    }

    CommandPool::operator VkCommandPool()
    {
        return pool;
    }
}

#include "CommandBuffer.hpp"

namespace daxa {
    CommandBuffer::CommandBuffer(VkCommandPool pool, VkDevice device)
    {
        VkCommandBufferAllocateInfo cmdAllocInfo = {};
        cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdAllocInfo.pNext = nullptr;
        cmdAllocInfo.commandPool = pool;                        // commands will be made from our _commandPool
        cmdAllocInfo.commandBufferCount = 1;                    // we will allocate 1 command buffer
        cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;   // command level is Primary

        vkAllocateCommandBuffers(device, &cmdAllocInfo, &buffer);
    }

    CommandBuffer::CommandBuffer(const CommandPool& pool) :
        CommandBuffer(pool.pool, pool.device)
    { }

    CommandBuffer::operator VkCommandBuffer()
    {
        return buffer;
    }
}
