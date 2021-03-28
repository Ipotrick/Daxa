#include "Vulkan.hpp"

#include <iostream>
#include <algorithm>

#include <VkBootstrap.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace daxa {
	namespace vulkan {
		VkInstance instance{ VK_NULL_HANDLE };
		VkPhysicalDevice mainPhysicalDevice{ VK_NULL_HANDLE };
		VkDevice mainDevice{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT debugMessenger{ VK_NULL_HANDLE };

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
				.select()
				.value();

			mainPhysicalDevice = physicalDevice.physical_device;

			vkb::DeviceBuilder deviceBuilder{ physicalDevice };

			vkb::Device vkbDevice = deviceBuilder.build().value();

			mainDevice = vkbDevice.device;
		}

		void cleanup()
		{
			std::cout << "cleanup\n";
			vkb::destroy_debug_utils_messenger(instance, debugMessenger);
			vkDestroyDevice(mainDevice, nullptr);
			vkDestroyInstance(instance, nullptr);
		}
	}
}
