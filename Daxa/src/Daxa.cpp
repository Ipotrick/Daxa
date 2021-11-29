#include "../include/Daxa.hpp"
#include "glslang/Public/ShaderLang.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace daxa {
	void initialize()
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			printf("error initializing SDL: %s\n", SDL_GetError());
			exit(-1);
		}
		glslang::InitializeProcess();
		Jobs::initialize();
	}

	void cleanup()
	{
		Jobs::cleanup();
		glslang::FinalizeProcess();
		SDL_Quit();
	}
}
