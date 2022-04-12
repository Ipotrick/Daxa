#include "ShaderModule.hpp"

#include <fstream>
#include <iostream>
#include <streambuf>
#include <thread>

#include "backend/DeviceBackend.hpp"

namespace daxa {
	ShaderModule::~ShaderModule() {
		if (deviceBackend) {
			vkDestroyShaderModule(deviceBackend->device.device, shaderModule, nullptr);
			deviceBackend = {};
		}
	}
}
