#include "ShaderModule.hpp"

#include <fstream>
#include <iostream>
#include <streambuf>
#include <thread>

namespace daxa {
	namespace gpu {

		ShaderModule::~ShaderModule() {
			if (deviceBackend) {
				vkDestroyShaderModule(deviceBackend->device.device, shaderModule, nullptr);
				deviceBackend = {};
			}
		}

	}
}
