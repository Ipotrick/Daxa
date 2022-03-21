#include "ShaderModule.hpp"

#include <fstream>
#include <iostream>
#include <streambuf>
#include <thread>

#include <vulkan/vulkan.h>

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include "Instance.hpp"

#include "shaderc/shaderc.hpp"

namespace daxa {
	namespace gpu {

		ShaderModule::~ShaderModule() {
			if (deviceBackend) {
				vkDestroyShaderModule(deviceBackend->device.device, shaderModule, nullptr);
				deviceBackend = {};
			}
		}

		Result<std::string> tryLoadShaderSourceFromFile(std::filesystem::path const& path) {
			auto startTime = std::chrono::steady_clock::now();
			while ((std::chrono::steady_clock::now() - startTime).count() < 100'000'000) {
				std::ifstream ifs(path);
				if (!ifs.good()) {
					std::string err = "could not find or open file: \"";
					err += path.string();
					err += '\"';
					return ResultErr{ err.c_str() };
				}
				std::string str;

				ifs.seekg(0, std::ios::end);   
				str.reserve(ifs.tellg());
				ifs.seekg(0, std::ios::beg);

				str.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

				if (str.size() < 1) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}
						
				return str;
			}

			std::string err = "time out while trying to read file: \"";
			err += path.string();
			err += '\"';
			return ResultErr{ err.c_str() };
		}

		Result<std::vector<u32>> tryGenSPIRVFromShaderc(std::string const& src, VkShaderStageFlagBits shaderStage, ShaderLang lang) {
			auto translateShaderStage = [](VkShaderStageFlagBits stage) -> shaderc_shader_kind {
				switch (stage) {
				case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT: return shaderc_shader_kind::shaderc_vertex_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return shaderc_shader_kind::shaderc_tess_control_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return shaderc_shader_kind::shaderc_tess_evaluation_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT: return shaderc_shader_kind::shaderc_geometry_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT: return shaderc_shader_kind::shaderc_fragment_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT: return shaderc_shader_kind::shaderc_compute_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR: return shaderc_shader_kind::shaderc_raygen_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return shaderc_shader_kind::shaderc_anyhit_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return shaderc_shader_kind::shaderc_closesthit_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR: return shaderc_shader_kind::shaderc_miss_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_INTERSECTION_BIT_KHR: return shaderc_shader_kind::shaderc_intersection_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR: return shaderc_shader_kind::shaderc_callable_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_NV: return shaderc_shader_kind::shaderc_task_shader;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_NV: return shaderc_shader_kind::shaderc_mesh_shader;
				default:
					std::cerr << "error: unknown shader stage!\n";
					std::abort();
				}
			};
			auto stage = translateShaderStage(shaderStage);
			shaderc_source_language langType;
			switch (lang) {
				case ShaderLang::GLSL: langType = shaderc_source_language_glsl; break;
				case ShaderLang::HLSL: langType = shaderc_source_language_hlsl; break;
			}

			shaderc::Compiler compiler;
  			shaderc::CompileOptions options;
			/*
				DO NOT ENABLE OPTIMIZATIONS!
				The driver will optimize it anyways AND
				the optimizations can shrink push constants wich will definetly lead to bugs in reflected pipeline layouts!
			*/
  			// options.SetOptimizationLevel(shaderc_optimization_level_size); 
			options.SetSourceLanguage(langType);
			
			shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(src, stage, "name", options);

			if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
				return daxa::ResultErr{.message = module.GetErrorMessage().c_str()};
			}

			return { std::vector<u32>{ module.begin(), module.end()} };
		}

		Result<VkShaderModule> tryCreateVkShaderModule(std::shared_ptr<DeviceBackend>& deviceBackend, std::vector<u32> const& spirv) {

			VkShaderModuleCreateInfo shaderModuleCI{
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.pNext = nullptr,
				.codeSize = (u32)(spirv.size() * sizeof(u32)),
				.pCode = spirv.data(),
			};
			VkShaderModule shaderModule;
			VkResult res;
			if ((res = vkCreateShaderModule(deviceBackend->device.device, (VkShaderModuleCreateInfo*)&shaderModuleCI, nullptr, &shaderModule)) == VK_SUCCESS) {
				return { shaderModule };
			}
			else {
				return ResultErr{ "could not create shader module from spriv source" };
			}
		}

		Result<VkShaderModule> tryCreateVkShaderModule(std::shared_ptr<DeviceBackend>& deviceBackend, std::filesystem::path const& path, VkShaderStageFlagBits shaderStage, ShaderLang lang) {
			auto src = tryLoadShaderSourceFromFile(path);
			if (src.isErr()) {
				return ResultErr{ src.message() };
			}
			auto spirv = tryGenSPIRVFromShaderc(src.value(), shaderStage, lang);
			if (spirv.isErr()) {
				return ResultErr{ spirv.message() + "; with path: " + path.string() };
			}
			auto shadMod = tryCreateVkShaderModule(deviceBackend, spirv.value());
			if (!shadMod.isErr()) {
				return ResultErr{ shadMod.message() };
			}
			return { shadMod.value() };
		}

		Result<ShaderModuleHandle> ShaderModuleHandle::tryCompileShader(std::shared_ptr<DeviceBackend>& deviceBackend, std::string const& glsl, std::string const& entryPoint, VkShaderStageFlagBits shaderStage, ShaderLang lang) {
			auto spirv = tryGenSPIRVFromShaderc(glsl, shaderStage, lang);
			if (spirv.isErr()) {
				return ResultErr{ spirv.message() };
			}
			auto shaderModule = tryCreateVkShaderModule(deviceBackend, spirv.value());
			if (shaderModule.isErr()) {
				return ResultErr{ shaderModule.message() };
			}
			auto shaderMod = std::make_shared<ShaderModule>();
			shaderMod->entryPoint = entryPoint;
			shaderMod->shaderModule = std::move(shaderModule.value());
			shaderMod->spirv = spirv.value();
			shaderMod->shaderStage = shaderStage;
			shaderMod->deviceBackend = deviceBackend;
			return { ShaderModuleHandle{ std::move(shaderMod) } };
		}
		
		Result<ShaderModuleHandle> ShaderModuleHandle::tryCreateDAXAShaderModule(std::shared_ptr<DeviceBackend>& deviceBackend, ShaderModuleCreateInfo const& ci) {
			std::string sourceCode = {};
			if (ci.pathToSource) {
				auto src = tryLoadShaderSourceFromFile(ci.pathToSource);
				if (src.isErr()) {
					return ResultErr{ src.message() };
				}
				sourceCode = src.value();
			}
			else if (ci.source) {
				sourceCode = ci.source;
			}
			else {
				return ResultErr{"no path given"};
			}

			auto shadMod = tryCompileShader(deviceBackend, sourceCode, ci.entryPoint, ci.stage, ci.shaderLang);
			if (shadMod.isErr()) {
				auto errMess = shadMod.message();
				if (ci.pathToSource) {
					errMess += "; shader from path: ";
					errMess += ci.pathToSource;
				}
				return ResultErr{ errMess };
			}

			if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
				shadMod.value()->debugName = ci.debugName;

				VkDebugUtilsObjectNameInfoEXT imageNameInfo {
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = NULL,
					.objectType = VK_OBJECT_TYPE_SHADER_MODULE,
					.objectHandle = (uint64_t)shadMod.value()->shaderModule,
					.pObjectName = ci.debugName,
				};
				instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &imageNameInfo);
			}

			return { shadMod.value() };
		}
	}
}
