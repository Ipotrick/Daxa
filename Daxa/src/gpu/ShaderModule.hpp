#pragma once

#include "../DaxaCore.hpp"

#include <variant>
#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <chrono>
#include <filesystem>
#include <set>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Handle.hpp"

#include "shaderc/shaderc.hpp"

namespace daxa {
	class PipelineCompiler;

	class PipelineHandle;
	class GraphicsPipelineBuilder;
	class BindingSetLayoutCache;

	enum class ShaderLang {
		GLSL,
		HLSL,
	};
	Result<std::vector<u32>> tryGenSPIRVFromSource(std::string const& src, VkShaderStageFlagBits shaderStage, ShaderLang lang);

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
		friend class CommandListBackend;
		friend class ShaderCache;
		friend class daxa::PipelineCompiler;

		std::shared_ptr<DeviceBackend>	deviceBackend	= {};
		std::vector<u32> 				spirv 			= {};
		VkShaderStageFlagBits 			shaderStage 	= {};
		std::string 					entryPoint 		= {};
		VkShaderModule 					shaderModule 	= {};
		std::string 					debugName 		= {};
	};

	struct ShaderModuleCreateInfo {
		std::string 				source			= {};
		std::filesystem::path		pathToSource 	= {};
		ShaderLang 					shaderLang 		= ShaderLang::GLSL;
		char const* 				entryPoint 		= "main";
		VkShaderStageFlagBits 		stage 			= {};
		std::vector<std::string> 	defines			= {};
		char const* 				debugName 		= {};
	};

	class ShaderModuleHandle : public SharedHandle<ShaderModule>{
	private:
		friend class Device;
		friend class daxa::PipelineCompiler;
	};
}