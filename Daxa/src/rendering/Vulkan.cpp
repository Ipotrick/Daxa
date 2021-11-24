#include "Vulkan.hpp"

#include <iostream>
#include <algorithm>

#include <VkBootstrap.hpp>

namespace daxa {
	namespace VulkanGlobals {
		vk::Instance				instance;
		vk::DebugUtilsMessengerEXT	debugMessenger;
		vk::PhysicalDevice			mainPhysicalDevice;
		vk::Device					device;
		vk::Queue					mainGraphicsQueue;
		u32							mainGraphicsQueueFamiltyIndex;
		vk::Queue					mainTransferQueue;
		u32							mainTransferQueueFamiltyIndex;
		vk::Queue					mainComputeQueue;
		u32							mainComputeQueueFamiltyIndex;
		VmaAllocator				allocator;

		void initialise()
		{
			vkb::InstanceBuilder instanceBuilder;
			instanceBuilder
				.set_app_name("Daxa Application")
				.set_engine_name("Daxa")
				.request_validation_layers(true)
				.require_api_version(1, 2, 0)
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
				.set_minimum_version(1,2)
				.defer_surface_initialization()
				.require_separate_compute_queue()
				.require_separate_transfer_queue()
				.select()
				.value();

			mainPhysicalDevice = physicalDevice.physical_device;

			vk::PhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features{
				.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
				.descriptorBindingPartiallyBound = VK_TRUE,
				.descriptorBindingVariableDescriptorCount = VK_TRUE,
				.runtimeDescriptorArray = VK_TRUE,
			};

			vkb::DeviceBuilder deviceBuilder{ physicalDevice };
			deviceBuilder.add_pNext(&descriptor_indexing_features);
			

			vkb::Device vkbDevice = deviceBuilder.build().value();

			device = vkbDevice.device;

			mainGraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
			mainGraphicsQueueFamiltyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
			mainTransferQueue = vkbDevice.get_queue(vkb::QueueType::transfer).value();
			mainTransferQueueFamiltyIndex = vkbDevice.get_queue_index(vkb::QueueType::transfer).value();
			mainComputeQueue = vkbDevice.get_queue(vkb::QueueType::compute).value();
			mainComputeQueueFamiltyIndex = vkbDevice.get_queue_index(vkb::QueueType::compute).value();

			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = mainPhysicalDevice;
			allocatorInfo.device = device;
			allocatorInfo.instance = instance;
			vmaCreateAllocator(&allocatorInfo, &allocator);
		}

		GPUContext getGlobalContext() {
			return GPUContext{
				.physicalDevice = mainPhysicalDevice,
				.device = device,
				.allocator = allocator,
				.graphicsQ = mainGraphicsQueue,
				.graphicsQFamilyIndex = mainGraphicsQueueFamiltyIndex,
				.transferQ = mainTransferQueue,
				.transferQFamiltyIndex = mainTransferQueueFamiltyIndex,
				.computeQ = mainComputeQueue,
				.computeQFamiltyIndex = mainComputeQueueFamiltyIndex
			};
		}

		void cleanup()
		{
			vmaDestroyAllocator(allocator);
			vkb::destroy_debug_utils_messenger(instance, debugMessenger);
			vkDestroyDevice(device, nullptr);
			vkDestroyInstance(instance, nullptr);
		}
	}
}
