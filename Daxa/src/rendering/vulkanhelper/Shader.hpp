#pragma once

#include <optional>
#include <string>

#include "../Vulkan.hpp"

namespace daxa {
	namespace vkh {
		std::optional<VkShaderModule> loadShaderModule(std::string filePath, VkDevice device = vkh::mainDevice);

		class ShaderModule {
		public:
			ShaderModule(std::string filePath, VkDevice device = vkh::mainDevice);

			ShaderModule(ShaderModule&& other) noexcept;

			~ShaderModule();

			operator const VkShaderModule&();

			const VkShaderModule& get() const;

			operator bool() const;

			bool valid() const;

		private:
			DeferedDestructionQueue* destructionQ{ nullptr };
			VkDevice device{ VK_NULL_HANDLE };
			VkShaderModule shader{ VK_NULL_HANDLE };
		};

		VkPipelineShaderStageCreateInfo makeShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
	}
}
