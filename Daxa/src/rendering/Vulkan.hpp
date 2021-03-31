#pragma once

#include <vulkan/vulkan.h>

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
	namespace vk {
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

		void initialise();

		void cleanup();
	}
}
