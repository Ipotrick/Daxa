#pragma once

#include "../DaxaCore.hpp"

#include <vector>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <VkBootstrap.h>

#include "Graveyard.hpp"

namespace daxa {
    namespace gpu {
        struct DeviceBackend {
			DeviceBackend(vkb::Instance& instance);
			~DeviceBackend();

            vkb::Device device                                                              = {};
			void (*vkCmdBeginRenderingKHR)(VkCommandBuffer, const VkRenderingInfoKHR*) 		= nullptr;
			void (*vkCmdEndRenderingKHR)(VkCommandBuffer) 									= nullptr;
			void (*vkCmdPipelineBarrier2KHR)(VkCommandBuffer, VkDependencyInfoKHR const*) 	= nullptr;
			VmaAllocator allocator 															= {};
			u32 graphicsQFamilyIndex 														= -1;
			u32 transferQFamilyIndex														= -1;
			u32 computeQFamilyIndex															= -1;
			std::vector<u32> allQFamilyIndices 												= {};
            Graveyard graveyard                                                             = {};
        };
    }
}