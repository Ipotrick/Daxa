#pragma once

#include <optional>
#include <string>

#include "../Vulkan.hpp"

namespace daxa {
	namespace vk {
		std::optional<VkShaderModule> loadShaderModule(std::string filePath, VkDevice device = vk::mainDevice);

		VkPipelineShaderStageCreateInfo makeShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
	}
}
