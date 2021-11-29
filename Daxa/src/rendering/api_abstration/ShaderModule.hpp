#pragma once
#include <memory>
#include <vector>
#include <string>
#include <filesystem>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

namespace daxa {
	namespace gpu {

		std::optional<std::string> tryLoadGLSLShaderFromFile(std::filesystem::path const& path);

		std::optional<std::vector<u32>> tryGenSPIRVFromGLSL(std::string const& src, vk::ShaderStageFlagBits shaderStage);

		std::optional<vk::UniqueShaderModule> tryCreateVkShaderModule(vk::Device device, std::vector<u32> const& spirv);

		std::optional<vk::UniqueShaderModule> tryCreateVkShaderModule(vk::Device device, std::filesystem::path const& path, vk::ShaderStageFlagBits shaderStage);

		class ShaderModule {
		public:
		private:
			friend class GraphicsPipelineBuilder;
			friend class ShaderModuleHandle;
			friend class Device;
			friend class CommandList;

			std::vector<u32> spirv;
			vk::ShaderStageFlagBits shaderStage;
			std::string entryPoint;
			vk::UniqueShaderModule shaderModule;
		};

		class ShaderModuleHandle {
		public:
			static std::optional<ShaderModuleHandle> tryCreateDAXAShaderModule(vk::Device device, std::filesystem::path const& path, std::string const& entryPoint, vk::ShaderStageFlagBits shaderStage);
			static std::optional<ShaderModuleHandle> tryCreateDAXAShaderModule(vk::Device device, std::string const& glsl, std::string const& entryPoint, vk::ShaderStageFlagBits shaderStage);
			ShaderModule const& operator * () const { return *shaderModule; }
			ShaderModule const* operator -> () const { return &*shaderModule; }
		private:
			friend class Device;

			std::shared_ptr<ShaderModule> shaderModule;
		};
	}
}