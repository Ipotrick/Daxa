#include "ShaderModule.hpp"

#include <fstream>
#include <iostream>
#include <streambuf>
#include <thread>

#include <vulkan/vulkan.h>

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include "Instance.hpp"

namespace daxa {
	namespace gpu {

		ShaderModule::~ShaderModule() {
			if (deviceBackend->device.device) {
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
			.maxVertexTextureImageUnits = 1 << 16,
			.maxCombinedTextureImageUnits = 1 << 16,
			.maxTextureImageUnits = 1 << 16,
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
			.maxComputeTextureImageUnits = 1 << 16,
			.maxComputeImageUniforms = 1 << 16,
			.maxComputeAtomicCounters = 8,
			.maxComputeAtomicCounterBuffers = 1,
			.maxVaryingComponents = 60,
			.maxVertexOutputComponents = 64,
			.maxGeometryInputComponents = 64,
			.maxGeometryOutputComponents = 128,
			.maxFragmentInputComponents = 128,
			.maxImageUnits = 1 << 16,
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
			.limits{
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

		/**
		 * @brief 	Because VSCode is dumb it sometimes touches files twice, where the first touch just clears the file
		 * 			so we just assume that empty files are the vs code bug and try to get the correct file for 100 milliseconds.
		 * 
		 * @param path 
		 * @return Result<std::string> 
		 */
		Result<std::string> tryLoadGLSLShaderFromFile(std::filesystem::path const& path) {
			auto startTime = std::chrono::steady_clock::now();
			while ((std::chrono::steady_clock::now() - startTime).count() < 100'000'000) {
				std::ifstream ifs(path);
				if (!ifs.good()) {
					return ResultErr{ "could not find or open file" };
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

			return ResultErr{ "timeout while trying to read file" };
		}

		Result<std::vector<u32>> tryGenSPIRVFromGLSL(std::string const& src, VkShaderStageFlagBits shaderStage) {
			auto translateShaderStage = [](VkShaderStageFlagBits stage) -> EShLanguage {
				switch (stage) {
				case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT: return EShLangVertex;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return EShLangTessControl;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return EShLangTessEvaluation;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT: return EShLangGeometry;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT: return EShLangFragment;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT: return EShLangCompute;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR: return EShLangRayGenNV;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return EShLangAnyHitNV;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return EShLangClosestHitNV;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR: return EShLangMissNV;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_INTERSECTION_BIT_KHR: return EShLangIntersectNV;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR: return EShLangCallableNV;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_NV: return EShLangTaskNV;
				case VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_NV: return EShLangMeshNV;
				default:
					std::cerr << "error: glslToSPIRV: UNKNOWN SHADER STAGE!\n";
					std::abort();
				}
			};
			const auto stage = translateShaderStage(shaderStage);
			const char* shaderStrings[] = { src.c_str() };
			glslang::TShader shader(stage);
			shader.setStrings(shaderStrings, 1);
			auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
			TBuiltInResource resource = DAXA_DEFAULT_SHADER_RESSOURCE_SIZES;
			if (!shader.parse(&resource, 100, false, messages)) {
				std::string s;
				s += "failed to compile shader due to:\n";
				s += shader.getInfoLog();
				s += shader.getInfoDebugLog();
				return ResultErr{s};
			}
			glslang::TProgram program;
			program.addShader(&shader);
			if (!program.link(messages)) {
				std::string s;
				s += "failed to compile shader due to:\n";
				s += shader.getInfoLog();
				s += shader.getInfoDebugLog();
				return ResultErr{s};
			}
			std::vector<std::uint32_t> ret;
			glslang::GlslangToSpv(*program.getIntermediate(stage), ret);
			return {ret};
		}

		Result<VkShaderModule> tryCreateVkShaderModule(std::shared_ptr<DeviceBackend>& deviceBackend, std::vector<u32> const& spirv) {

			VkShaderModuleCreateInfo shaderModuleCI{
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.pNext = nullptr,
				.codeSize = (u32)(spirv.size() * sizeof(u32)),
				.pCode = spirv.data(),
			};
			VkShaderModule shaderModule;
			if (vkCreateShaderModule(deviceBackend->device.device, (VkShaderModuleCreateInfo*)&shaderModuleCI, nullptr, &shaderModule) == VK_SUCCESS) {
				return { shaderModule };
			}
			else {
				return ResultErr{ "could not create shader module from spriv source" };
			}
		}

		Result<VkShaderModule> tryCreateVkShaderModule(std::shared_ptr<DeviceBackend>& deviceBackend, std::filesystem::path const& path, VkShaderStageFlagBits shaderStage) {
			auto src = tryLoadGLSLShaderFromFile(path);
			if (src.isErr()) {
				return ResultErr{ src.message() };
			}
			auto spirv = tryGenSPIRVFromGLSL(src.value(), shaderStage);
			if (spirv.isErr()) {
				return ResultErr{ spirv.message() + "; with path: " + path.string() };
			}
			auto shadMod = tryCreateVkShaderModule(deviceBackend, spirv.value());
			if (!shadMod.isErr()) {
				return ResultErr{ shadMod.message() };
			}
			return { shadMod.value() };
		}

		Result<ShaderModuleHandle> ShaderModuleHandle::tryCreateDAXAShaderModule(std::shared_ptr<DeviceBackend>& deviceBackend, std::filesystem::path const& path, std::string const& entryPoint, VkShaderStageFlagBits shaderStage) {
			auto src = tryLoadGLSLShaderFromFile(path);
			if (src.isErr()) {
				return ResultErr{ src.message() };
			}
			auto shadMod = tryCreateDAXAShaderModule(deviceBackend, src.value(), entryPoint, shaderStage);
			if (shadMod.isErr()) {
				return ResultErr{ shadMod.message() };
			}
			return { shadMod.value() };
		}

		Result<ShaderModuleHandle> ShaderModuleHandle::tryCreateDAXAShaderModule(std::shared_ptr<DeviceBackend>& deviceBackend, std::string const& glsl, std::string const& entryPoint, VkShaderStageFlagBits shaderStage) {
			auto spirv = tryGenSPIRVFromGLSL(glsl, shaderStage);
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
			std::string glslSource = {};
			if (ci.pathToSource) {
				auto src = tryLoadGLSLShaderFromFile(ci.pathToSource);
				if (src.isErr()) {
					return ResultErr{ src.message() };
				}
				glslSource = src.value();
			}
			else if (ci.glslSource) {
				glslSource = ci.glslSource;
			}
			else {
				return ResultErr{"no path given"};
			}

			auto shadMod = tryCreateDAXAShaderModule(deviceBackend, glslSource, ci.entryPoint, ci.stage);
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
