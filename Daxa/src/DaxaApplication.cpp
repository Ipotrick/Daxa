#include "DaxaApplication.hpp"

#include <VkBootstrap.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>

namespace daxa {
	Application::Application(std::string name, u32 width, u32 height)
	{
		windowMutex = std::make_unique<OwningMutex<Window>>(
			name, 
			std::array<u32, 2>{ width, height }, 
			vulkan::instance, 
			vulkan::mainDevice,
			vulkan::mainPhysicalDevice
			);

		init_default_renderpass();

		init_framebuffers();
	}

	Application::~Application()
	{
		cleanup();
	}
}