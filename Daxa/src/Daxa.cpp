// Daxa.cpp : Defines the functions for the static library.
//

#include "framework.h"

#include <SDL2/SDL.h>
#include "glslang/Public/ShaderLang.h"

#include "gpu/Device.hpp"

namespace daxa {
	void initialize() {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			printf("error initializing SDL: %s\n", SDL_GetError());
			exit(-1);
		}

		glslang::InitializeProcess();
	}

	void cleanup() {
		glslang::FinalizeProcess();
		SDL_Quit();
	}
}