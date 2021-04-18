#pragma once

#include <vulkan/vulkan.h>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

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
	namespace VulkanContext {
		extern vk::Instance					instance;
		extern vk::DebugUtilsMessengerEXT	debugMessenger;
		extern vk::PhysicalDevice			mainPhysicalDevice;
		extern vk::Device					device;
		extern vk::Queue					mainGraphicsQueue;
		extern u32							mainGraphicsQueueFamiltyIndex;
		extern vk::Queue					mainTransferQueue;
		extern u32							mainTransferQueueFamiltyIndex;
		extern vk::Queue					mainComputeQueue;
		extern u32							mainComputeQueueFamiltyIndex;
		extern VmaAllocator					allocator;

		void initialise();

		void cleanup();
	}
}
