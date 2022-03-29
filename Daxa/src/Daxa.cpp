#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "glslang/Public/ShaderLang.h"

#include "gpu/Instance.hpp"
#include "rendering/Cameras.hpp"

namespace daxa {

	void initialize() {
		if (glfwInit() != GLFW_TRUE)
		{
			printf("error: could not initialize GLFW3!\n");
			exit(-1);
		}
		instance = std::make_unique<Instance>();
		glslang::InitializeProcess();
	}

	void cleanup() {
		glslang::FinalizeProcess();
		instance.reset();
	}
}