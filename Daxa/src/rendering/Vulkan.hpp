#pragma once

#include <optional>

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "../DaxaCore.hpp"

namespace daxa{
	namespace vulkan {
		extern VkInstance instance;
		extern VkPhysicalDevice mainPhysicalDevice;
		extern VkDevice mainDevice;
		extern VkDebugUtilsMessengerEXT debugMessenger;

		void initialise();

		void cleanup();
	}
}
