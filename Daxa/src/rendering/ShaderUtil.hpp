#pragma once


#include <ctime>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>

#include "../DaxaCore.hpp"

#include "Vulkan.hpp"

namespace daxa {

	inline constexpr TBuiltInResource getDefaultGLSLangResource()
	{
		return TBuiltInResource{
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
	}

	inline constexpr TBuiltInResource getDefaultGLSLangResourceVulkan() {
		auto ret = getDefaultGLSLangResource();
		ret.maxTextureUnits = 4096;
		ret.maxTextureImageUnits = 4096;
		return ret;
	}

	inline std::optional<std::vector<u32>> glslToSPIRV(const std::string& glsl, vk::ShaderStageFlagBits shaderStage) 
	{
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
		const char* shaderStrings[] = { glsl.c_str() };
		glslang::TShader shader(stage);
		shader.setStrings(shaderStrings, 1);
		auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
		TBuiltInResource resource = getDefaultGLSLangResourceVulkan();
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

	inline std::optional<std::string> loadShaderFromFile(const std::filesystem::path& filepath) 
	{
		std::ifstream ifs(filepath);
		if (!ifs) {
			std::cerr << "error: could not open file: " << filepath << std::endl;
			return {};
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
		}
		catch (...) {
			std::cerr << "error: UNKNOWN while reading file: " << filepath << std::endl;
			return {};
		}
		return ret;
	}

	inline std::optional<std::vector<std::uint32_t>> loadGLSLShaderToSpv(const std::filesystem::path& filepath) {
		auto sourceStringOpt = loadShaderFromFile(filepath);
		if (!sourceStringOpt) return {};
		vk::ShaderStageFlagBits shaderStage;

		if (filepath.extension() == ".vert") {
			shaderStage = vk::ShaderStageFlagBits::eVertex;
		}
		else if (filepath.extension() == ".frag") {
			shaderStage = vk::ShaderStageFlagBits::eFragment;
		}
		else if (filepath.extension() == ".comp") {
			shaderStage = vk::ShaderStageFlagBits::eCompute;
		}

		auto spvOpt = glslToSPIRV(sourceStringOpt.value(), shaderStage);
		if (!spvOpt) return {};

		return spvOpt.value();
	}

	class ShaderHotLoader {
	public:
		ShaderHotLoader(std::vector<std::filesystem::path> listenedFiles, std::function<void()> recompilePipeline) :
			files{ std::move(listenedFiles) }, recompilePipeline{ std::move(recompilePipeline) }
		{
			for (auto& file : files) {
				if (!std::filesystem::exists(file)) {
					std::cerr << "warning: ShaderHotLoader can not observe file: \"" << file << "\" as the path does not exist!\n";
					lastWriteTimes.push_back(std::filesystem::file_time_type::min());
				}
				else {
					lastWriteTimes.push_back(std::filesystem::last_write_time(file));
				}
			}
		}

		void update() 
		{
			bool bRecompile{ false };
			for (uz i = 0; i < files.size(); i++) {
				if (lastWriteTimes[i] != std::filesystem::file_time_type::min()) {
					auto currentLastWriteTime = std::filesystem::last_write_time(files[i]);
					if (currentLastWriteTime != lastWriteTimes[i]) {
						std::cout << "changed\n";
						lastWriteTimes[i] = currentLastWriteTime;
						bRecompile = true;
					}
				}
			}
			if (bRecompile) {
				recompilePipeline();
			}
		}
	private:
		std::vector<std::filesystem::path> files;
		std::vector<std::filesystem::file_time_type> lastWriteTimes;
		std::function<void()> recompilePipeline;
	};
}
