#include "Vulkan.hpp"

#include <iostream>
#include <algorithm>

#include <VkBootstrap.hpp>

namespace daxa {
	namespace vkh {
		VkInstance					instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT	debugMessenger{ VK_NULL_HANDLE };
		VkPhysicalDevice			mainPhysicalDevice{ VK_NULL_HANDLE };
		VkDevice					mainDevice{ VK_NULL_HANDLE };
		VkQueue						mainGraphicsQueue{ VK_NULL_HANDLE };
		u32							mainGraphicsQueueFamiltyIndex{ VK_NULL_HANDLE };
		VkQueue						mainTransferQueue{ VK_NULL_HANDLE };
		u32							mainTransferQueueFamiltyIndex{ VK_NULL_HANDLE };
		VkQueue						mainComputeQueue{ VK_NULL_HANDLE };
		u32							mainComputeQueueFamiltyIndex{ VK_NULL_HANDLE };
		VmaAllocator				allocator;

		void initialise()
		{
			vkb::InstanceBuilder instanceBuilder;
			instanceBuilder
				.set_app_name("Daxa Application")
				.set_engine_name("Daxa")
				.request_validation_layers(true)
				.require_api_version(1, 1, 0)
				.use_default_debug_messenger();
			auto instanceBuildReturn = instanceBuilder.build();
			if (!instanceBuildReturn) {
				std::cerr << "could not create vulkan instance!\n";
			}

			auto vkbInstance = instanceBuildReturn.value();
			instance = vkbInstance.instance;
			debugMessenger = vkbInstance.debug_messenger;

			vkb::PhysicalDeviceSelector selector{ vkbInstance };
			vkb::PhysicalDevice physicalDevice = selector
				.set_minimum_version(1, 1)
				.defer_surface_initialization()
				.require_separate_compute_queue()
				.require_separate_transfer_queue()
				.select()
				.value();

			mainPhysicalDevice = physicalDevice.physical_device;

			vkb::DeviceBuilder deviceBuilder{ physicalDevice };

			vkb::Device vkbDevice = deviceBuilder.build().value();

			mainDevice = vkbDevice.device;

			mainGraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
			mainGraphicsQueueFamiltyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
			mainTransferQueue = vkbDevice.get_queue(vkb::QueueType::transfer).value();
			mainTransferQueueFamiltyIndex = vkbDevice.get_queue_index(vkb::QueueType::transfer).value();
			mainComputeQueue = vkbDevice.get_queue(vkb::QueueType::compute).value();
			mainComputeQueueFamiltyIndex = vkbDevice.get_queue_index(vkb::QueueType::compute).value();

			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = vkh::mainPhysicalDevice;
			allocatorInfo.device = vkh::mainDevice;
			allocatorInfo.instance = vkh::instance;
			vmaCreateAllocator(&allocatorInfo, &allocator);
		}

		void cleanup()
		{
			vmaDestroyAllocator(allocator);
			vkb::destroy_debug_utils_messenger(instance, debugMessenger);
			vkDestroyDevice(mainDevice, nullptr);
			vkDestroyInstance(instance, nullptr);
		}
	}
}
