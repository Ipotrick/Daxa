#include "ShaderModule.hpp"

#include <fstream>
#include <iostream>

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>

namespace daxa {
	namespace gpu {

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

		std::optional<std::string> tryLoadGLSLShaderFromFile(std::filesystem::path const& path) {
			std::ifstream ifs(path);
			if (!ifs) {
				return std::nullopt;
			}

			std::string ret;
			try {
				ifs.seekg(0, std::ios::end);
				ret.reserve(ifs.tellg());
				ifs.seekg(0, std::ios::beg);
				ret.assign(
					std::istreambuf_iterator<char>(ifs),
					std::istreambuf_iterator<char>());
				ifs.close();
				return ret;
			}
			catch (...) {
				return std::nullopt;
			}
		}

		std::optional<std::vector<u32>> tryGenSPIRVFromGLSL(std::string const& src, vk::ShaderStageFlagBits shaderStage) {
			auto translateShaderStage = [](vk::ShaderStageFlagBits stage) -> EShLanguage {
				switch (stage) {
				case vk::ShaderStageFlagBits::eVertex: return EShLangVertex;
				case vk::ShaderStageFlagBits::eTessellationControl: return EShLangTessControl;
				case vk::ShaderStageFlagBits::eTessellationEvaluation: return EShLangTessEvaluation;
				case vk::ShaderStageFlagBits::eGeometry: return EShLangGeometry;
				case vk::ShaderStageFlagBits::eFragment: return EShLangFragment;
				case vk::ShaderStageFlagBits::eCompute: return EShLangCompute;
				case vk::ShaderStageFlagBits::eRaygenNV: return EShLangRayGenNV;
				case vk::ShaderStageFlagBits::eAnyHitNV: return EShLangAnyHitNV;
				case vk::ShaderStageFlagBits::eClosestHitNV: return EShLangClosestHitNV;
				case vk::ShaderStageFlagBits::eMissNV: return EShLangMissNV;
				case vk::ShaderStageFlagBits::eIntersectionNV: return EShLangIntersectNV;
				case vk::ShaderStageFlagBits::eCallableNV: return EShLangCallableNV;
				case vk::ShaderStageFlagBits::eTaskNV: return EShLangTaskNV;
				case vk::ShaderStageFlagBits::eMeshNV: return EShLangMeshNV;
				default:
					std::cerr << "error: glslToSPIRV: UNKNOWN SHADER STAGE!\n";
					return {};
				}
			};
			const auto stage = translateShaderStage(shaderStage);
			const char* shaderStrings[] = { src.c_str() };
			glslang::TShader shader(stage);
			shader.setStrings(shaderStrings, 1);
			auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
			TBuiltInResource resource = DAXA_DEFAULT_SHADER_RESSOURCE_SIZES;
			if (!shader.parse(&resource, 100, false, messages)) {
				std::cerr << shader.getInfoLog() << '\n' << shader.getInfoDebugLog() << std::endl;
				return {};
			}
			glslang::TProgram program;
			program.addShader(&shader);
			if (!program.link(messages)) {
				std::cerr << shader.getInfoLog() << '\n' << shader.getInfoDebugLog() << std::endl;
				return {};
			}
			std::vector<std::uint32_t> ret;
			glslang::GlslangToSpv(*program.getIntermediate(stage), ret);
			return ret;
		}

		std::optional<vk::UniqueShaderModule> tryCreateVkShaderModule(vk::Device device, std::vector<u32> const& spirv) {

			vk::ShaderModuleCreateInfo sci{};
			sci.codeSize = static_cast<u32>(spirv.size() * sizeof(u32));
			sci.pCode = spirv.data();

			VkShaderModule shaderModule;



			auto err = vkCreateShaderModule(device, (VkShaderModuleCreateInfo*)&sci, nullptr, &shaderModule);
			if (err == VK_SUCCESS) {
				return vk::UniqueShaderModule{ shaderModule, device };
			}
			else {
				return std::nullopt;
			}
		}

		std::optional<vk::UniqueShaderModule> tryCreateVkShaderModule(vk::Device device, std::filesystem::path const& path, vk::ShaderStageFlagBits shaderStage) {
			auto src = tryLoadGLSLShaderFromFile(path);
			if (src.has_value()) {
				auto spirv = tryGenSPIRVFromGLSL(src.value(), shaderStage);
				if (spirv.has_value()) {
					return tryCreateVkShaderModule(device, spirv.value());
				}
			}
			return std::nullopt;
		}

		std::optional<ShaderModuleHandle> ShaderModuleHandle::tryCreateDAXAShaderModule(vk::Device device, std::filesystem::path const& path, std::string const& entryPoint, vk::ShaderStageFlagBits shaderStage) {
			auto src = tryLoadGLSLShaderFromFile(path);
			if (src.has_value()) {
				return tryCreateDAXAShaderModule(device, src.value(), entryPoint, shaderStage);
			}
			return std::nullopt;
		}

		std::optional<ShaderModuleHandle> ShaderModuleHandle::tryCreateDAXAShaderModule(vk::Device device, std::string const& glsl, std::string const& entryPoint, vk::ShaderStageFlagBits shaderStage) {
			auto spirv = tryGenSPIRVFromGLSL(glsl, shaderStage);
			if (spirv.has_value()) {
				auto shaderModuleOpt = tryCreateVkShaderModule(device, spirv.value());
				if (shaderModuleOpt.has_value()) {
					ShaderModule shaderMod;
					shaderMod.entryPoint = entryPoint;
					shaderMod.shaderModule = std::move(shaderModuleOpt.value());
					shaderMod.spirv = spirv.value();
					shaderMod.shaderStage = shaderStage;
					ShaderModuleHandle handle;
					handle.shaderModule = std::make_shared<ShaderModule>(std::move(shaderMod));
					return handle;
				}
			}
			return std::nullopt;
		}
	}
}
