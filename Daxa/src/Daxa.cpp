#include "../include/Daxa.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "rendering/Vulkan.hpp"

namespace daxa {
	void initialize()
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			printf("error initializing SDL: %s\n", SDL_GetError());
			exit(-1);
		}
		VulkanContext::initialise();
		glslang::InitializeProcess();
		Jobs::initialize();
	}

	void cleanup()
	{
		Jobs::cleanup();
		glslang::FinalizeProcess();
		VulkanContext::cleanup();
		SDL_Quit();
	}
}
