#pragma once

#include <vulkan/vulkan.h>

#include "../dependencies/vk_mem_alloc.hpp"

#include "../DaxaCore.hpp"

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cout <<"Detected Vulkan error: " << err << std::endl; \
			abort();                                                \
		}                                                           \
	} while (0)

namespace daxa{
	namespace vkh {
		extern VkInstance				instance;
		extern VkDebugUtilsMessengerEXT debugMessenger;
		extern VkPhysicalDevice			mainPhysicalDevice;
		extern VkDevice					mainDevice;
		extern VkQueue					mainGraphicsQueue;
		extern u32						mainGraphicsQueueFamiltyIndex;
		extern VkQueue					mainTransferQueue;
		extern u32						mainTransferQueueFamiltyIndex;
		extern VkQueue					mainComputeQueue;
		extern u32						mainComputeQueueFamiltyIndex;
		extern VmaAllocator				allocator;

		void initialise();

		void cleanup();
	}
}
