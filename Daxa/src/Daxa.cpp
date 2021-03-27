#include "../include/Daxa.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace daxa {
	void initialize()
	{
		Jobs::initialize();
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			printf("error initializing SDL: %s\n", SDL_GetError());
			exit(-1);
		}
	}

	void cleanup()
	{
		SDL_Quit();
		Jobs::cleanup();
	}
}
