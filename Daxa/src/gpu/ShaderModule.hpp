#pragma once

#include "../DaxaCore.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <string>
#include <filesystem>

#include <vk_mem_alloc.h>

namespace daxa {
	namespace gpu {

		std::optional<std::string> tryLoadGLSLShaderFromFile(std::filesystem::path const& path);

		std::optional<std::vector<u32>> tryGenSPIRVFromGLSL(std::string const& src, VkShaderStageFlagBits shaderStage);

		class ShaderModule {
		public:
			ShaderModule()						 					= default;
			ShaderModule(ShaderModule&& other) noexcept				= delete;
			ShaderModule& operator=(ShaderModule&& other) noexcept	= delete;
			~ShaderModule();

			std::vector<u32> const& getSPIRV() const { return spirv; }
			VkShaderStageFlagBits getVkShaderStage() const { return shaderStage; }
			std::string const& getVkEntryPoint() const { return entryPoint; }
			VkShaderModule getVkShaderModule() const { return shaderModule; }
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
			ShaderModuleHandle(std::shared_ptr<ShaderModule> shaderModule) 
				: shaderModule{ std::move(shaderModule) }
			{}
			ShaderModuleHandle() = default;
			
			static std::optional<ShaderModuleHandle> tryCreateDAXAShaderModule(VkDevice device, std::filesystem::path const& path, std::string const& entryPoint, VkShaderStageFlagBits shaderStage);
			static std::optional<ShaderModuleHandle> tryCreateDAXAShaderModule(VkDevice device, std::string const& glsl, std::string const& entryPoint, VkShaderStageFlagBits shaderStage);

			ShaderModule const& operator*() const { return *shaderModule; }
			ShaderModule& operator*() { return *shaderModule; }
			ShaderModule const* operator->() const { return shaderModule.get(); }
			ShaderModule* operator->() { return shaderModule.get(); }

			size_t getRefCount() const { return shaderModule.use_count(); }

			operator bool() const { return shaderModule.operator bool(); }
			bool operator!() const { return !shaderModule; }
			bool valid() const { return *this; }
		private:
			std::shared_ptr<ShaderModule> shaderModule = {};
		};
	}
}