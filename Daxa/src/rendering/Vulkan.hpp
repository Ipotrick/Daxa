#pragma once

#include <vulkan/vulkan.h>

#include "../DaxaCore.hpp"

namespace daxa{
	namespace vulkan {
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
