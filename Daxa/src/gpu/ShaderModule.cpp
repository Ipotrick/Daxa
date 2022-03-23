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

		Result<std::string> tryLoadShaderSourceFromFile(std::filesystem::path const& path, std::set<std::pair<std::filesystem::path, std::chrono::file_clock::time_point>>& observedHotloadFilePaths) {
			auto startTime = std::chrono::steady_clock::now();
			while ((std::chrono::steady_clock::now() - startTime).count() < 100'000'000) {
				std::ifstream ifs(path);
				if (!ifs.good()) {
					std::string err = "could not find or open file: \"";
					err += path.string();
					err += '\"';
					return ResultErr{ err.c_str() };
				}
				observedHotloadFilePaths.insert({path, std::filesystem::last_write_time(path)});
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

		Result<std::vector<u32>> tryGenSPIRVFromShaderc(std::string const& src, VkShaderStageFlagBits shaderStage, ShaderLang lang, shaderc::Compiler& compiler, shaderc::CompileOptions& options, char const* sourceFileName = "inline source") {
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
			options.SetSourceLanguage(langType);
			
			shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(src, stage, sourceFileName, options);

			if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
				return daxa::ResultErr{.message = std::move(module.GetErrorMessage())};
			}

			return { std::vector<u32>{ module.begin(), module.end()} };
		}
		
		Result<ShaderModuleHandle> ShaderModuleHandle::tryCreateDAXAShaderModule(std::shared_ptr<DeviceBackend>& deviceBackend, ShaderModuleCreateInfo const& ci, shaderc::Compiler& compiler, shaderc::CompileOptions& options, std::set<std::pair<std::filesystem::path, std::chrono::file_clock::time_point>>& observedHotloadFilePaths) {
			std::string sourceCode = {};
			if (!ci.pathToSource.empty()) {
				auto src = tryLoadShaderSourceFromFile(ci.pathToSource, observedHotloadFilePaths);
				if (src.isErr()) {
					return ResultErr{ src.message() };
				}
				sourceCode = src.value();
			}
			else if (!ci.source.empty()) {
				sourceCode = ci.source;
			}
			else {
				return ResultErr{"no path given"};
			}

			auto spirv = tryGenSPIRVFromShaderc(sourceCode, ci.stage, ci.shaderLang, compiler, options, (ci.pathToSource.empty() ? "inline source" : ci.pathToSource.string().c_str()));
			if (spirv.isErr()) {
				return ResultErr{ spirv.message() };
			}

			VkShaderModuleCreateInfo shaderModuleCI{
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.pNext = nullptr,
				.codeSize = (u32)(spirv.value().size() * sizeof(u32)),
				.pCode = spirv.value().data(),
			};
			VkShaderModule shaderModule;
			VkResult res;
			if ((res = vkCreateShaderModule(deviceBackend->device.device, (VkShaderModuleCreateInfo*)&shaderModuleCI, nullptr, &shaderModule)) != VK_SUCCESS) {
				std::string errMess{ "could not create shader module from spriv source" };
				if (!ci.pathToSource.empty()) {
					errMess += "; shader from path: ";
					errMess += ci.pathToSource.string();
				}
				return ResultErr{ errMess };
			}

			auto shadMod = std::make_shared<ShaderModule>();
			shadMod->entryPoint = ci.entryPoint;
			shadMod->shaderModule = shaderModule;
			shadMod->spirv = spirv.value();
			shadMod->shaderStage = ci.stage;
			shadMod->deviceBackend = deviceBackend;

			if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
				shadMod->debugName = ci.debugName;

				VkDebugUtilsObjectNameInfoEXT imageNameInfo {
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = NULL,
					.objectType = VK_OBJECT_TYPE_SHADER_MODULE,
					.objectHandle = (uint64_t)shadMod->shaderModule,
					.pObjectName = ci.debugName,
				};
				instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &imageNameInfo);
			}

			return { ShaderModuleHandle{ shadMod } };
		}
	}
}
