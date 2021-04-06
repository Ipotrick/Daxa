#pragma once

#include <optional>
#include <string>

#include "../Vulkan.hpp"

namespace daxa {
	namespace vkh {
		std::optional<vk::UniqueShaderModule> loadShaderModule(std::string filePath, vk::Device device = vkh::device);

		vk::PipelineShaderStageCreateInfo makeShaderStageCreateInfo(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule);
	}
}
