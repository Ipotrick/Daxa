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

		constexpr TBuiltInResource DAXA_DEFAULT_SHADER_RESSOURCE_SIZES = TBuiltInResource{
			.maxLights = 32,
			.maxClipPlanes = 6,
			.maxTextureUnits = 32,
			.maxTextureCoords = 32,
			.maxVertexAttribs = 64,
			.maxVertexUniformComponents = 4096,
			.maxVaryingFloats = 64,
			.maxVertexTextureImageUnits = 32,
			.maxCombinedTextureImageUnits = 80,
			.maxTextureImageUnits = 32,
			.maxFragmentUniformComponents = 4096,
			.maxDrawBuffers = 32,
			.maxVertexUniformVectors = 128,
			.maxVaryingVectors = 8,
			.maxFragmentUniformVectors = 16,
			.maxVertexOutputVectors = 16,
			.maxFragmentInputVectors = 15,
			.minProgramTexelOffset = -8,
			.maxProgramTexelOffset = 7,
			.maxClipDistances = 8,
			.maxComputeWorkGroupCountX = 65535,
			.maxComputeWorkGroupCountY = 65535,
			.maxComputeWorkGroupCountZ = 65535,
			.maxComputeWorkGroupSizeX = 1024,
			.maxComputeWorkGroupSizeY = 1024,
			.maxComputeWorkGroupSizeZ = 64,
			.maxComputeUniformComponents = 1024,
			.maxComputeTextureImageUnits = 16,
			.maxComputeImageUniforms = 8,
			.maxComputeAtomicCounters = 8,
			.maxComputeAtomicCounterBuffers = 1,
			.maxVaryingComponents = 60,
			.maxVertexOutputComponents = 64,
			.maxGeometryInputComponents = 64,
			.maxGeometryOutputComponents = 128,
			.maxFragmentInputComponents = 128,
			.maxImageUnits = 8,
			.maxCombinedImageUnitsAndFragmentOutputs = 8,
			.maxCombinedShaderOutputResources = 8,
			.maxImageSamples = 0,
			.maxVertexImageUniforms = 0,
			.maxTessControlImageUniforms = 0,
			.maxTessEvaluationImageUniforms = 0,
			.maxGeometryImageUniforms = 0,
			.maxFragmentImageUniforms = 8,
			.maxCombinedImageUniforms = 8,
			.maxGeometryTextureImageUnits = 16,
			.maxGeometryOutputVertices = 256,
			.maxGeometryTotalOutputComponents = 1024,
			.maxGeometryUniformComponents = 1024,
			.maxGeometryVaryingComponents = 64,
			.maxTessControlInputComponents = 128,
			.maxTessControlOutputComponents = 128,
			.maxTessControlTextureImageUnits = 16,
			.maxTessControlUniformComponents = 1024,
			.maxTessControlTotalOutputComponents = 4096,
			.maxTessEvaluationInputComponents = 128,
			.maxTessEvaluationOutputComponents = 128,
			.maxTessEvaluationTextureImageUnits = 16,
			.maxTessEvaluationUniformComponents = 1024,
			.maxTessPatchComponents = 120,
			.maxPatchVertices = 32,
			.maxTessGenLevel = 64,
			.maxViewports = 16,
			.maxVertexAtomicCounters = 0,
			.maxTessControlAtomicCounters = 0,
			.maxTessEvaluationAtomicCounters = 0,
			.maxGeometryAtomicCounters = 0,
			.maxFragmentAtomicCounters = 8,
			.maxCombinedAtomicCounters = 8,
			.maxAtomicCounterBindings = 1,
			.maxVertexAtomicCounterBuffers = 0,
			.maxTessControlAtomicCounterBuffers = 0,
			.maxTessEvaluationAtomicCounterBuffers = 0,
			.maxGeometryAtomicCounterBuffers = 0,
			.maxFragmentAtomicCounterBuffers = 1,
			.maxCombinedAtomicCounterBuffers = 1,
			.maxAtomicCounterBufferSize = 16384,
			.maxTransformFeedbackBuffers = 4,
			.maxTransformFeedbackInterleavedComponents = 64,
			.maxCullDistances = 8,
			.maxCombinedClipAndCullDistances = 8,
			.maxSamples = 4,
			.maxMeshOutputVerticesNV = 256,
			.maxMeshOutputPrimitivesNV = 512,
			.maxMeshWorkGroupSizeX_NV = 32,
			.maxMeshWorkGroupSizeY_NV = 1,
			.maxMeshWorkGroupSizeZ_NV = 1,
			.maxTaskWorkGroupSizeX_NV = 32,
			.maxTaskWorkGroupSizeY_NV = 1,
			.maxTaskWorkGroupSizeZ_NV = 1,
			.maxMeshViewCountNV = 4,
			.limits = {
				.nonInductiveForLoops = 1,
				.whileLoops = 1,
				.doWhileLoops = 1,
				.generalUniformIndexing = 1,
				.generalAttributeMatrixVectorIndexing = 1,
				.generalVaryingIndexing = 1,
				.generalSamplerIndexing = 1,
				.generalVariableIndexing = 1,
				.generalConstantMatrixVectorIndexing = 1,

			},
		};

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

		void ShaderCache::init(std::shared_ptr<DeviceBackend>& device, std::vector<std::filesystem::path> possibleRootPaths) { 
			this->device = device;
			this->possibleRootPaths = possibleRootPaths;
		}
			
		Result<std::filesystem::path> ShaderCache::findCompletePath(std::filesystem::path sourcePath) {
			std::string err = "could not find path to: \"";
			err += sourcePath.string();
			err += "\"";
			daxa::Result<std::filesystem::path> ret = ResultErr{err};
			for (auto& root: possibleRootPaths) {
				auto potentialPath = root / sourcePath;
				if (std::ifstream(potentialPath).good()) {
					if (ret.isOk()) {
						// we found the file more than once, this is illegal
						std::string err = "the file: \"";
						err += sourcePath.string();
						err += "\" was found multiple times in the given root paths";
						daxa::Result<std::filesystem::path> ret = ResultErr{err};
						break;
					} else {
						ret = Result{ potentialPath };
					}
				}
			}
			return ret;
		}
	}
}
