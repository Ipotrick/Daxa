#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <Daxa/gpu/Instance.hpp>
#include <Daxa/rendering/Cameras.hpp>

namespace daxa {

	void initialize() {
		if (glfwInit() != GLFW_TRUE)
		{
			printf("error: could not initialize GLFW3!\n");
			exit(-1);
		}
		instance = std::make_unique<Instance>();
	}

	void cleanup() {
		instance.reset();
	}
}