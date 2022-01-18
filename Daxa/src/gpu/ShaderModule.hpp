#pragma once

#include "../DaxaCore.hpp"

#include <variant>
#include <memory>
#include <vector>
#include <string>
#include <filesystem>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Handle.hpp"

namespace daxa {
	namespace gpu {

		Result<std::string> tryLoadGLSLShaderFromFile(std::filesystem::path const& path);

		Result<std::vector<u32>> tryGenSPIRVFromGLSL(std::string const& src, VkShaderStageFlagBits shaderStage);

		class ShaderModule {
		public:
			ShaderModule()						 					= default;
			ShaderModule(ShaderModule&& other) noexcept				= delete;
			ShaderModule& operator=(ShaderModule&& other) noexcept	= delete; 
			ShaderModule(ShaderModule const& other)					= delete;
			ShaderModule& operator=(ShaderModule const& other)		= delete;
			~ShaderModule();

			std::vector<u32> const& getSPIRV() const { return spirv; }
			VkShaderStageFlagBits getVkShaderStage() const { return shaderStage; }
			std::string const& getVkEntryPoint() const { return entryPoint; }
			VkShaderModule getVkShaderModule() const { return shaderModule; }

			std::string const& getDebugName() const { return debugName; }
		private:
			friend class GraphicsPipelineBuilder;
			friend class ShaderModuleHandle;
			friend class Device;
			friend class CommandList;

			VkDevice 				device 			= {};
			std::vector<u32> 		spirv 			= {};
			VkShaderStageFlagBits 	shaderStage 	= {};
			std::string 			entryPoint 		= {};
			VkShaderModule 			shaderModule 	= {};
			std::string 			debugName 		= {};
		};

		struct ShaderModuleCreateInfo {
			const char* 			glslSource		= {};
			const char*				pathToSource 	= {};
			char const* 			entryPoint 		= "main";
			VkShaderStageFlagBits 	stage 			= {};
			char const* 			debugName 		= {};
		};

		class ShaderModuleHandle : public SharedHandle<ShaderModule>{
		private:
			friend class Device;
			static Result<ShaderModuleHandle> tryCreateDAXAShaderModule(VkDevice device, std::filesystem::path const& path, std::string const& entryPoint, VkShaderStageFlagBits shaderStage);
			static Result<ShaderModuleHandle> tryCreateDAXAShaderModule(VkDevice device, std::string const& glsl, std::string const& entryPoint, VkShaderStageFlagBits shaderStage);
			static Result<ShaderModuleHandle> tryCreateDAXAShaderModule(VkDevice device, ShaderModuleCreateInfo const& ci);
		};
	}
}