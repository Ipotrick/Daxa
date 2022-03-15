#pragma once

#include "../DaxaCore.hpp"

#include <variant>
#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <chrono>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Handle.hpp"
#include "DeviceBackend.hpp"

namespace daxa {
	namespace gpu {

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
			friend class CommandList;
			friend class ShaderCache;

			std::shared_ptr<DeviceBackend>	deviceBackend	= {};
			std::vector<u32> 				spirv 			= {};
			VkShaderStageFlagBits 			shaderStage 	= {};
			std::string 					entryPoint 		= {};
			VkShaderModule 					shaderModule 	= {};
			std::string 					debugName 		= {};
		};

		struct ShaderModuleCreateInfo {
			const char* 			source			= {};
			const char*				pathToSource 	= {};
			ShaderLang 				shaderLang 		= ShaderLang::GLSL;
			char const* 			entryPoint 		= "main";
			VkShaderStageFlagBits 	stage 			= {};
			char const* 			debugName 		= {};
		};

		class ShaderModuleHandle : public SharedHandle<ShaderModule>{
		private:
			friend class ShaderCache;
			friend class Device;
			static Result<ShaderModuleHandle> tryCompileShader(std::shared_ptr<DeviceBackend>& device, std::string const& glsl, std::string const& entryPoint, VkShaderStageFlagBits shaderStage, ShaderLang lang);
			static Result<ShaderModuleHandle> tryCreateDAXAShaderModule(std::shared_ptr<DeviceBackend>& device, ShaderModuleCreateInfo const& ci);
		};

		struct ShaderCacheEntry {
			ShaderModuleHandle shaderModule = {};
			std::chrono::time_point<std::chrono::file_clock> lastTouchedTime = {};
		};

		class ShaderCache {
		public:
			ShaderCache() = default;
			void init(std::shared_ptr<DeviceBackend>& device, std::vector<std::filesystem::path> possibleRootPaths);

			Result<ShaderModuleHandle> tryGetShaderModule(std::filesystem::path path, VkShaderStageFlagBits flags, char const* debugName = nullptr);

			Result<std::filesystem::path> findCompletePath(std::filesystem::path sourcePath);
		private:
			std::shared_ptr<DeviceBackend> device = {};
			std::unordered_map<std::string, ShaderCacheEntry> cache = {};
			std::vector<std::filesystem::path> possibleRootPaths = {};
		};
	}
}