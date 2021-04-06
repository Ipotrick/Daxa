#pragma once

#include "../Vulkan.hpp"

#include "Pool.hpp"

namespace daxa {
    namespace vkh {
        vk::UniqueCommandPool makeCommandPool(
            u32 queueFamilyIndex = daxa::vkh::mainGraphicsQueueFamiltyIndex,
            vk::CommandPoolCreateFlagBits flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            vk::Device device = daxa::vkh::device);

        class CommandPool {
        public:
            CommandPool(
                u32 queueFamilyIndex = daxa::vkh::mainGraphicsQueueFamiltyIndex,
                vk::CommandPoolCreateFlagBits flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                vk::Device device = daxa::vkh::device
            );

            CommandPool(CommandPool&& other) noexcept;

            operator const vk::UniqueCommandPool&();

            const vk::UniqueCommandPool& get();

            vk::CommandBuffer getBuffer();

            void flush();

        private:
            u32 queueFamilyIndex{ 0xFFFFFFFF };
            vk::Device device;
            vk::UniqueCommandPool pool{};
            Pool<vk::CommandBuffer> bufferPool;
        };
    }
}
