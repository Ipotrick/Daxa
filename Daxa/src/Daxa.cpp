// Daxa.cpp : Defines the functions for the static library.
//

#include "framework.h"

#include <SDL2/SDL.h>
#include "glslang/Public/ShaderLang.h"

#include "gpu/Instance.hpp"

namespace daxa {

	void initialize() {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			printf("error initializing SDL: %s\n", SDL_GetError());
			exit(-1);
		}
		gpu::instance = std::make_unique<gpu::Instance>();
		glslang::InitializeProcess();
	}

	void cleanup() {
		glslang::FinalizeProcess();
		gpu::instance.reset();
		SDL_Quit();
	}
}