#pragma once

#include "../DaxaCore.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <string>
#include <filesystem>

#include "../dependencies/vk_mem_alloc.hpp"

namespace daxa {
	namespace gpu {

		std::optional<std::string> tryLoadGLSLShaderFromFile(std::filesystem::path const& path);

		std::optional<std::vector<u32>> tryGenSPIRVFromGLSL(std::string const& src, VkShaderStageFlagBits shaderStage);

		class ShaderModule {
		public:
			ShaderModule() = default;
			ShaderModule(ShaderModule&& other) noexcept;
			ShaderModule& operator=(ShaderModule&& other) noexcept;
			~ShaderModule();
		private:
			friend class GraphicsPipelineBuilder;
			friend class ShaderModuleHandle;
			friend class Device;
			friend class CommandList;

			VkDevice device;
			std::vector<u32> spirv;
			VkShaderStageFlagBits shaderStage;
			std::string entryPoint;
			VkShaderModule shaderModule;
		};

		class ShaderModuleHandle {
		public:
			static std::optional<ShaderModuleHandle> tryCreateDAXAShaderModule(VkDevice device, std::filesystem::path const& path, std::string const& entryPoint, VkShaderStageFlagBits shaderStage);
			static std::optional<ShaderModuleHandle> tryCreateDAXAShaderModule(VkDevice device, std::string const& glsl, std::string const& entryPoint, VkShaderStageFlagBits shaderStage);
			ShaderModule const& operator * () const { return *shaderModule; }
			ShaderModule const* operator -> () const { return &*shaderModule; }
		private:
			friend class Device;

			std::shared_ptr<ShaderModule> shaderModule;
		};
	}
}