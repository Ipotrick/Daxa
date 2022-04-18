#include "PipelineCompiler.hpp"

#include <fstream>
#include <thread>
#include <locale>
#include <regex>
#include <sstream>

#include "spirv_reflect.h"
#include "shaderc/shaderc.h"
#define NOMINMAX
#ifdef _WIN32
	#include "Windows.h"
	#include "wrl/client.h"
	using namespace Microsoft::WRL;
#else
#endif
#include "dxcapi.h"

#include "Instance.hpp"
#include "util.hpp"
#include "backend/DeviceBackend.hpp"

using Path = std::filesystem::path;

static const std::regex PRAGMA_ONCE_REGEX = std::regex(R"reg(#\s*pragma\s*once\s*)reg");
static const std::regex REPLACE_REGEX = std::regex(R"reg(\W)reg");

static void shaderPreprocess(std::string& fileStr, std::filesystem::path const& path) {
    std::smatch matches;
    std::string line;
    std::stringstream fileSS{fileStr};
    std::stringstream resultSS;

    bool has_pragma_once = false;
    auto abspathStr = std::filesystem::absolute(path).string();

    for (std::size_t line_num = 0; std::getline(fileSS, line); ++line_num) {
        if (std::regex_match(line, matches, PRAGMA_ONCE_REGEX)) {
            resultSS << "#if !defined(";
            std::regex_replace(std::ostreambuf_iterator<char>(resultSS), abspathStr.begin(), abspathStr.end(), REPLACE_REGEX, "_");
            resultSS << ")\n";
            has_pragma_once = true;
        } else {
            resultSS << line << "\n";
        }
    }

    if (has_pragma_once) {
        resultSS << "\n#define ";
        std::regex_replace(std::ostreambuf_iterator<char>(resultSS), abspathStr.begin(), abspathStr.end(), REPLACE_REGEX, "_");
        resultSS << "\n#endif";
    }

    fileStr = resultSS.str();
}

namespace daxa {
	#define BACKEND (*static_cast<Backend*>(this->backend.get()))

	struct Backend{
        shaderc::Compiler compiler = {};
        shaderc::CompileOptions options = {};
		ComPtr<IDxcUtils> dxcUtils = {};
		ComPtr<IDxcCompiler3> dxcCompiler = {};
		ComPtr<IDxcIncludeHandler> dxcIncludeHandler = {};
	};

	void backendDeletor(void* backend) {
		delete static_cast<Backend*>(backend);
	}

	std::vector<VkDescriptorSetLayout> processReflectedDescriptorData(
		std::vector<BindingSetDescription>& setDescriptions,
		BindingSetLayoutCache& descCache,
		std::array<std::shared_ptr<BindingSetLayout const>, MAX_SETS_PER_PIPELINE>& setLayouts
	) {
		std::vector<VkDescriptorSetLayout> descLayouts;
		BindingSetDescription description;
		for (size_t set = 0; set < setDescriptions.size(); set++) {
			auto layout = descCache.getLayoutShared(setDescriptions[set]);
			setLayouts[set] = layout;
			descLayouts.push_back(layout->getVkDescriptorSetLayout());
		}
		return std::move(descLayouts);
	}
	
	Result<std::string> PipelineCompilerShadedData::tryLoadShaderSourceFromFile(std::filesystem::path const& path) {
		auto result = findFullPathOfFile(path);
		if (result.isErr()) {
			return ResultErr{ .message = result.message() };
		}

		auto startTime = std::chrono::steady_clock::now();
		while ((std::chrono::steady_clock::now() - startTime).count() < 100'000'000) {
			std::ifstream ifs(result.value());
			if (!ifs.good()) {
				std::string err = "could not find or open file: \"";
				err += result.value().string();
				err += '\"';
				return ResultErr{ err.c_str() };
			}
			observedHotLoadFiles->insert({result.value(), std::filesystem::last_write_time(result.value())});
			std::string str;

			ifs.seekg(0, std::ios::end);   
			str.reserve(ifs.tellg());
			ifs.seekg(0, std::ios::beg);

			str.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

			if (str.size() < 1) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}

            shaderPreprocess(str, path);

			return str;
		}

		std::string err = "time out while trying to read file: \"";
		err += result.value().string();
		err += '\"';
		return ResultErr{ err.c_str() };
	}

	std::wstring u8_ascii_to_wstring(char const* str) {
		std::wstring ret;
		for (int i = 0; i < std::strlen(str)+1 && str != nullptr; i++) {
			ret.push_back(str[i]);
		}
		return ret;
	}

	Result<std::vector<u32>> PipelineCompiler::tryGenSPIRVFromDxc(
		std::string const& src, 
		VkShaderStageFlagBits shaderStage, 
		char const* entryPoint, 
		char const* sourceFileName, 
		std::vector<std::string> const& defines
	) {
		std::vector<LPCWSTR> args;

		std::vector<std::wstring> wstringBuffer;
		wstringBuffer.reserve(defines.size());
		for (auto& define : defines) {
			wstringBuffer.push_back(u8_ascii_to_wstring(define.c_str()));
			args.push_back(L"-D");
			args.push_back(wstringBuffer.back().c_str());
		}

		auto sourceFileNameWString = u8_ascii_to_wstring(sourceFileName);
		args.push_back(sourceFileNameWString.c_str());

		// Bugfix: 	Do not make a vector of strings and get the data ptr to them.
		//			The data ptr will be invalidated on push_back when the path is short enough to be short string optimized.
		for (auto& root : sharedData->rootPaths) {
			args.push_back(L"-I");
			args.push_back(root.c_str());
		}

		// set matrix packing to column major
		args.push_back(L"-Zpc");

		// set warnings as errors
		args.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX

		// setting target
		args.push_back(L"-spirv");
		args.push_back(L"-fspv-target-env=vulkan1.1");

		// set optimization setting
		args.push_back(L"-O3");

		// setting entry point
		args.push_back(L"-E");
		std::wstring entryAsWideString;
		for (int i = 0; i < std::strlen(entryPoint)+1 && entryPoint != nullptr; i++) {
			entryAsWideString.push_back(entryPoint[i]);
		}
		args.push_back(entryAsWideString.data());
		
		// set shader model
		args.push_back(L"-T");
		std::wstring profile;
		switch (shaderStage) {
			case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT: profile = L"vs_6_0"; break;
			case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT: profile = L"ps_6_0"; break;
			case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT: profile = L"cs_6_0"; break;
			default: return daxa::ResultErr{ .message = "shader compilation error: the given shader stage is either missing or not supported by dxc" };
		}
		args.push_back(profile.data());

		// set hlsl version to 2021
		args.push_back(L"-HV");
		args.push_back(L"2021");

		DxcBuffer srcBuffer{
			.Ptr = src.c_str(),
			.Size = static_cast<u32>(src.size()),
			.Encoding = static_cast<u32>(0),
		};

		ComPtr<IDxcResult> result;
		BACKEND.dxcCompiler->Compile(&srcBuffer, args.data(), static_cast<u32>(args.size()), BACKEND.dxcIncludeHandler.Get(), IID_PPV_ARGS(&result));

		ComPtr<IDxcBlobUtf8> errorMessage;
		result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorMessage), nullptr);

		if (errorMessage && errorMessage->GetStringLength() > 0) {
			auto str = std::string();
			str.resize(errorMessage->GetBufferSize());
			for (size_t i = 0; i < str.size(); i++) {
				str[i] = static_cast<char const*>(errorMessage->GetBufferPointer())[i];
			}
			str = std::string("DXC: ") + str;
			//str = std::string(sourceFileName) + str;
			
			return daxa::ResultErr{.message = str };
		}

		ComPtr<IDxcBlob> shaderobj;
		result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderobj), nullptr);

		std::vector<u32> spv;
		spv.resize(shaderobj->GetBufferSize()/4);
		for (size_t i = 0; i < spv.size(); i++) {
			spv[i] = static_cast<u32*>(shaderobj->GetBufferPointer())[i];
		}

		return {spv};
	}

	Result<std::vector<u32>> PipelineCompiler::tryGenSPIRVFromShaderc(
		std::string const& src, 
		VkShaderStageFlagBits shaderStage, 
		ShaderLang lang, 
		char const* sourceFileName,
		std::vector<std::string> const& defines
	) {
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
		BACKEND.options.SetSourceLanguage(langType);
		BACKEND.options.SetTargetEnvironment(shaderc_target_env_vulkan, VK_API_VERSION_1_2);
		BACKEND.options.SetTargetSpirv(shaderc_spirv_version::shaderc_spirv_version_1_3);
		// TODO: implement defines
		//options.SetOptimizationLevel(shaderc_optimization_level_performance);
		
		shaderc::SpvCompilationResult module = BACKEND.compiler.CompileGlslToSpv(src, stage, sourceFileName, BACKEND.options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
			return daxa::ResultErr{.message = std::move(module.GetErrorMessage())};
		}

		return { std::vector<u32>{ module.begin(), module.end()} };
	}

	std::vector<VkPushConstantRange> reflectPushConstants(const std::vector<uint32_t>& spv, VkShaderStageFlagBits shaderStage) {
		SpvReflectShaderModule module = {};
		SpvReflectResult result = spvReflectCreateShaderModule(spv.size() * sizeof(uint32_t), spv.data(), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t count = 0;
		result = spvReflectEnumeratePushConstantBlocks(&module, &count, NULL);

		std::vector<SpvReflectBlockVariable*> blocks(count);
		result = spvReflectEnumeratePushConstantBlocks(&module, &count, blocks.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<VkPushConstantRange> ret;
		for (auto* block : blocks) {
			ret.push_back(VkPushConstantRange{
				.stageFlags = (VkShaderStageFlags)shaderStage,
				.offset = block->offset,
				.size = block->size,
				});
		}
		return std::move(ret);
	}

	std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>>
	reflectSetBindings(const std::vector<uint32_t>& spv, VkShaderStageFlagBits shaderStage) {
		SpvReflectShaderModule module = {};
		SpvReflectResult result = spvReflectCreateShaderModule(spv.size() * sizeof(uint32_t), spv.data(), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t count = 0;
		result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet*> sets(count);
		result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> setMap;
		for (auto* set : sets) {
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			for (uint32_t i = 0; i < set->binding_count; i++) {
				auto* reflBinding = set->bindings[i];

				VkDescriptorSetLayoutBinding binding{
					.binding = reflBinding->binding,
					.descriptorType = static_cast<VkDescriptorType>(reflBinding->descriptor_type),
					.descriptorCount = reflBinding->count,
					.stageFlags = (VkShaderStageFlags)shaderStage
				};
				setMap[set->set][binding.binding] = binding;
			}
		}

		spvReflectDestroyShaderModule(&module);
		return std::move(setMap);
	}

	void reflectShader(
		ShaderModuleHandle const& shaderModule, 
		std::vector<VkPushConstantRange>& pushConstants, 
		std::vector<BindingSetDescription>& bindingSetDescriptions
	) {
		auto falserefl = reflectPushConstants(shaderModule->getSPIRV(), shaderModule->getVkShaderStage());
		auto refl2 = reinterpret_cast<std::vector<VkPushConstantRange>*>(&falserefl);
		auto& refl = *refl2;
		for (auto& r : refl) {
			auto iter = pushConstants.begin();
			for (; iter != pushConstants.end(); iter++) {
				if (iter->offset == r.offset) {
					if (iter->size != r.size) {
						DAXA_ASSERT_M(false, "push constant ranges of the same offset must have the same size");
					}
					// found the same push contant range:
					iter->stageFlags |= r.stageFlags;
					break;
				}
			}
			if (iter == pushConstants.end()) {
				// did not find same push constant. make new one:
				pushConstants.push_back(r);
			}
		}

		// reflect spirv descriptor sets:
		auto reflDesc = reflectSetBindings(shaderModule->getSPIRV(), shaderModule->getVkShaderStage());
		//}
		for (auto& [set, bindingLayouts] : reflDesc) {
			if (set >= bindingSetDescriptions.size()) {
				bindingSetDescriptions.resize(set+1, {});
			}

			for (auto& [binding, layout] : bindingLayouts) {
				bindingSetDescriptions[set].layouts[binding].descriptorType = layout.descriptorType;
				bindingSetDescriptions[set].layouts[binding].descriptorCount = layout.descriptorCount;
				bindingSetDescriptions[set].layouts[binding].stageFlags |= layout.stageFlags;
			}
		}
	}

	
	Result<ShaderModuleHandle> PipelineCompiler::tryCreateShaderModule(ShaderModuleCreateInfo const& ci) {
		std::string sourceCode = {};
		if (!ci.pathToSource.empty()) {
			auto src = sharedData->tryLoadShaderSourceFromFile(ci.pathToSource);
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

		daxa::Result<std::vector<u32>> spirv = daxa::ResultErr{};
		if (ci.shaderLang == ShaderLang::GLSL) {
			spirv = tryGenSPIRVFromShaderc(sourceCode, ci.stage, ci.shaderLang, (ci.pathToSource.empty() ? "inline source" : ci.pathToSource.string().c_str()), ci.defines);
		} 
		else {
			spirv = tryGenSPIRVFromDxc(sourceCode, ci.stage, ci.entryPoint, (ci.pathToSource.empty() ? "inline source" : ci.pathToSource.string().c_str()), ci.defines);
		}
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

		if (daxa::instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
			shadMod->debugName = ci.debugName;

			VkDebugUtilsObjectNameInfoEXT imageNameInfo {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_SHADER_MODULE,
				.objectHandle = (uint64_t)shadMod->shaderModule,
				.pObjectName = ci.debugName,
			};
			daxa::instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &imageNameInfo);
		}

		return { ShaderModuleHandle{ shadMod } };
	}

	class DxcFileIncluder : public IDxcIncludeHandler {
	public:
		std::shared_ptr<PipelineCompilerShadedData> sharedData = {};
		ComPtr<IDxcUtils> pUtils;
		ComPtr<IDxcIncludeHandler> pDefaultIncludeHandler;
		HRESULT LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override
		{
			if (pFilename[0] == '.') {
				pFilename += 2;
			}

			auto result = sharedData->findFullPathOfFile(pFilename);
			if (result.isErr()) {
				*ppIncludeSource = nullptr;
				return SCARD_E_FILE_NOT_FOUND;
			}
			auto fullPath = result.value();
			auto searchPred = [&](std::filesystem::path const& p){ return p == fullPath; };

			ComPtr<IDxcBlobEncoding> pEncoding;
			if (std::find_if(sharedData->currentShaderSeenFiles.begin(), sharedData->currentShaderSeenFiles.end(), searchPred) != sharedData->currentShaderSeenFiles.end()) {
				// Return empty string blob if this file has been included before
				static const char nullStr[] = " ";
				pUtils->CreateBlob(nullStr, ARRAYSIZE(nullStr), CP_UTF8, pEncoding.GetAddressOf());
				*ppIncludeSource = pEncoding.Detach();
				return S_OK;
			}
			else {
				sharedData->observedHotLoadFiles->insert({ fullPath, std::chrono::file_clock::now() });
			}

            auto str_result = sharedData->tryLoadShaderSourceFromFile(fullPath);
            if (str_result.isErr()) {
				*ppIncludeSource = nullptr;
				return SCARD_E_INVALID_PARAMETER;
			}
            std::string str = str_result.value();

			pUtils->CreateBlob(str.data(), static_cast<u32>(str.size()), CP_UTF8, pEncoding.GetAddressOf());
			*ppIncludeSource = pEncoding.Detach();
			return S_OK;
		}

		HRESULT QueryInterface(REFIID riid, void * * ppvObject) override
		{
			return pDefaultIncludeHandler->QueryInterface(riid, ppvObject);
		}

		ULONG STDMETHODCALLTYPE AddRef(void) override {	return 0; }
		ULONG STDMETHODCALLTYPE Release(void) override { return 0; }

	};

	class ShadercFileIncluder : public shaderc::CompileOptions::IncluderInterface {
	public:
		constexpr static inline size_t DELETE_SOURCE_NAME = 0x1;
		constexpr static inline size_t DELETE_CONTENT = 0x2;

		virtual shaderc_include_result* GetInclude(
			const char* requested_source,
            shaderc_include_type type,
            const char* requesting_source,
            size_t include_depth
		) override {
			shaderc_include_result* res = new shaderc_include_result{};
			if (include_depth <= 10) {
				res->source_name = requested_source;
				res->source_name_length = strlen(requested_source);
				res->user_data = 0;

				auto result = sharedData->findFullPathOfFile(requested_source);
				if (result.isOk()) {
					std::filesystem::path requested_source_path = std::move(result.value());
					auto searchPred = [&](std::filesystem::path const& p){ return p == requested_source_path; };

					if (std::find_if(sharedData->currentShaderSeenFiles.begin(), sharedData->currentShaderSeenFiles.end(), searchPred) == sharedData->currentShaderSeenFiles.end()) {
						std::ifstream ifs{requested_source_path};
						std::string str;
						ifs.seekg(0, std::ios::end);   
						str.reserve(ifs.tellg());
						ifs.seekg(0, std::ios::beg);
						str.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

						res->user_data = reinterpret_cast<void*>(reinterpret_cast<size_t>(res->user_data) | DELETE_CONTENT);
						char* data = new char[str.size()+1];
						for (size_t i = 0; i < str.size()+1; i++) {
							data[i] = str[i];
						}
						res->content_length = str.size();
						res->content = data;

						std::string requestedSourcePathAsString = requested_source_path.string();

						char* includerPath = new char[requestedSourcePathAsString.size()+1];
						for (size_t i = 0; i < requestedSourcePathAsString.size()+1; i++) {
							includerPath[i] = requestedSourcePathAsString[i];
						}

						res->source_name = const_cast<const char*>(includerPath);
						res->user_data = reinterpret_cast<void*>(reinterpret_cast<size_t>(res->user_data) | DELETE_SOURCE_NAME);;
						sharedData->currentShaderSeenFiles.push_back(requested_source);
						sharedData->observedHotLoadFiles->insert({requested_source_path,std::filesystem::last_write_time(requested_source_path)});
					} else {
						// double includes are ignored
						res->content = nullptr;
						res->content_length = 0;
					}
				} else {
					// file path could not be resolved
					res->content = "could not find file";
					res->content_length = strlen(res->content);

					res->source_name = nullptr;
					res->source_name_length = 0;
				}
			}
			else {
				// max include depth exceeded
				res->content = "current include depth of 10 was exceeded";
				res->content_length = std::strlen(res->content);

				res->source_name = nullptr;
				res->source_name_length = 0;
			}
			return res;
		}

    	virtual void ReleaseInclude(shaderc_include_result* data) override {
			if (data) {
				if (reinterpret_cast<size_t>(data->user_data) & DELETE_CONTENT) {
					delete data->content;
				}
				if (reinterpret_cast<size_t>(data->source_name) & DELETE_SOURCE_NAME) {
					delete data->source_name;
				}
				delete data;
			}
		};

		std::shared_ptr<PipelineCompilerShadedData> sharedData = {};
	};

    PipelineCompiler::PipelineCompiler(std::shared_ptr<DeviceBackend> deviceBackend, std::shared_ptr<BindingSetLayoutCache> bindSetLayoutCache) 
		: deviceBackend{ deviceBackend }
		, bindSetLayoutCache{ bindSetLayoutCache }
		, sharedData{ std::make_shared<PipelineCompilerShadedData>() }
		, recreationCooldown{ 250 }
		, backend{ std::unique_ptr<void, void(*)(void*)>(new Backend(), backendDeletor) }
	{
		auto includer = std::make_unique<ShadercFileIncluder>();
		includer->sharedData = sharedData;
		BACKEND.options.SetIncluder(std::move(includer));
		if (!SUCCEEDED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(BACKEND.dxcUtils.ReleaseAndGetAddressOf())))) {
			std::cout << "[[DXA ERROR]] could not create DXC Instance" << std::endl;
		};

		if (!SUCCEEDED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&BACKEND.dxcCompiler)))) {
			std::cout << "[[DXA ERROR]] could not create DXC Compiler" << std::endl;
		}

		ComPtr<DxcFileIncluder> dxcIncluder = new DxcFileIncluder();
		dxcIncluder->sharedData = sharedData;
		dxcIncluder->pUtils = BACKEND.dxcUtils;
		BACKEND.dxcUtils->CreateDefaultIncludeHandler(dxcIncluder->pDefaultIncludeHandler.ReleaseAndGetAddressOf());
		BACKEND.dxcIncludeHandler = dxcIncluder;
	}

    bool PipelineCompiler::checkIfSourcesChanged(PipelineHandle& pipeline) {
		std::chrono::time_point<std::chrono::file_clock> now = std::chrono::file_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - pipeline->lastRecreationCheckTimePoint) < recreationCooldown) {
			return false;
		}
		pipeline->lastRecreationCheckTimePoint = now;
		bool reload = false;
		for (auto& [path, recordedWriteTime] : pipeline->observedHotLoadFiles) {
			auto ifs = std::ifstream(path);
			if (ifs.good()) {
				auto latestWriteTime = std::filesystem::last_write_time(path);

				if (latestWriteTime > recordedWriteTime) {
					reload = true;
				}
			}
		}
		if (reload) {
			for (auto& pair : pipeline->observedHotLoadFiles) {
				auto ifs = std::ifstream(pair.first);
				if (ifs.good()) {
					*(std::chrono::file_clock::time_point*)(&pair.second) = std::filesystem::last_write_time(pair.first);
				}
			}
		}
		return reload;
	}

	Result<PipelineHandle> PipelineCompiler::recreatePipeline(PipelineHandle const& pipeline) {
		auto handleResult = [&](Result<PipelineHandle> const& result) -> Result<PipelineHandle> {
			if (result.isOk()) {
				return {std::move(result.value())};
			}
			else {
				return ResultErr{ .message = result.message() };
			}
		};
		if (auto graphicsCreator = std::get_if<GraphicsPipelineBuilder>(&pipeline->creator)) {
			auto result = createGraphicsPipeline(*graphicsCreator);
			return std::move(handleResult(result));
		}
		else if (auto computeCreator = std::get_if<ComputePipelineCreateInfo>(&pipeline->creator)) {
			auto result = createComputePipeline(*computeCreator);
			return std::move(handleResult(result));
		}
		else {
			DAXA_ASSERT_M(false, "unreachable. missing arm for potential new variant type");
            return Result<PipelineHandle>{nullptr};
		}
	}

	
    void PipelineCompiler::recreateIfChanged(PipelineHandle& pipeline) {
		if (checkIfSourcesChanged(pipeline)) {
			auto result = recreatePipeline(pipeline);
			std::cout << result << std::endl;
			if (result.isOk()) {
				pipeline = result.value();
			}
		}
	}

    void PipelineCompiler::addShaderSourceRootPath(Path const& root) {
        this->sharedData->rootPaths.push_back(root); 
    }

    Result<Path> PipelineCompilerShadedData::findFullPathOfFile(Path const& file) {
		std::ifstream ifs{file};
		if (ifs.good()) {
			return { file };
		}
        Path potentialPath;
        for (auto& root : this->rootPaths) {
            potentialPath.clear();
            potentialPath = root / file;

            std::ifstream ifs{potentialPath};
            if (ifs.good()) {
                return { potentialPath };
            }
        }
        std::string errorMessage;
        errorMessage += "could not find file :\"";
        errorMessage += file.string();
        errorMessage += "\"";
        return ResultErr{ .message = std::move(errorMessage) };
    }

    Result<PipelineHandle> PipelineCompiler::createGraphicsPipeline(GraphicsPipelineBuilder const& builder) {
        DAXA_ASSERT_M(!builder.bVertexAtrributeBindingBuildingOpen, "vertex attribute bindings must be completed before creating a pipeline");

		auto pipelineHandle = PipelineHandle{ std::make_shared<Pipeline>() };
		Pipeline& ret = *pipelineHandle;
		ret.deviceBackend = deviceBackend;
		ret.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		ret.colorAttachmentFormats = builder.colorAttachmentFormats;
		ret.depthAttachment = builder.depthTestSettings.depthAttachmentFormat;
		ret.stencilAttachment = VK_FORMAT_UNDEFINED;
		ret.creator = builder;

		sharedData->observedHotLoadFiles = &ret.observedHotLoadFiles;

		std::vector<VkPushConstantRange> pushConstants;
		std::vector<BindingSetDescription> bindingSetDescriptions;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo;
		std::vector<ShaderModuleHandle> shaderModules;
		shaderModules.reserve(builder.shaderModuleCIs.size());

		for (auto& shaderCI : builder.shaderModuleCIs) {
			sharedData->currentShaderSeenFiles.clear();
			auto result = tryCreateShaderModule(shaderCI);
			if (result.isOk()) {
				shaderModules.push_back(result.value());
			} else {
				return ResultErr{ .message = result.message() };
			}
		}

		for (auto& shaderModule : shaderModules) {
			VkPipelineShaderStageCreateInfo pipelineShaderStageCI{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = nullptr,
				.stage = shaderModule->shaderStage,
				.module = shaderModule->shaderModule,
				.pName = shaderModule->entryPoint.c_str(),
			};
			shaderStageCreateInfo.push_back(pipelineShaderStageCI);
			reflectShader(shaderModule, pushConstants, bindingSetDescriptions);
		}

		for (auto& [set, descr] : builder.setDescriptionOverwrites) {
			bindingSetDescriptions[set] = descr;
		}

		auto descLayouts = processReflectedDescriptorData(bindingSetDescriptions, *bindSetLayoutCache, ret.bindingSetLayouts);

		// create pipeline layout:
		VkPipelineLayoutCreateInfo pipelineLayoutCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.setLayoutCount = static_cast<u32>(descLayouts.size()),
			.pSetLayouts = descLayouts.data(),
			.pushConstantRangeCount = static_cast<u32>(pushConstants.size()),
			.pPushConstantRanges = pushConstants.data(),
		};
		DAXA_CHECK_VK_RESULT_M(vkCreatePipelineLayout(deviceBackend->device.device, &pipelineLayoutCI, nullptr, &ret.layout), "failed to create graphics pipeline");

		// create pipeline:
		VkPipelineVertexInputStateCreateInfo pVertexInput = VkPipelineVertexInputStateCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.vertexBindingDescriptionCount = (u32)builder.vertexInputBindingDescriptions.size(),
			.pVertexBindingDescriptions = (VkVertexInputBindingDescription*)builder.vertexInputBindingDescriptions.data(),
			.vertexAttributeDescriptionCount = (u32)builder.vertexInputAttributeDescriptions.size(),
			.pVertexAttributeDescriptions = (VkVertexInputAttributeDescription*)builder.vertexInputAttributeDescriptions.data(),
		};
		VkPipelineInputAssemblyStateCreateInfo pinputAssemlyStateCI = builder.inputAssembly.value_or(DEFAULT_INPUT_ASSEMBLY_STATE_CI);
		VkPipelineRasterizationStateCreateInfo prasterizationStateCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.depthClampEnable = (VkBool32)builder.rasterSettings.depthBiasClamp,
			.rasterizerDiscardEnable = builder.rasterSettings.rasterizerDiscardEnable,
			.polygonMode = builder.rasterSettings.polygonMode,
			.cullMode = (VkBool32)builder.rasterSettings.cullMode,
			.frontFace = builder.rasterSettings.frontFace,
			.depthBiasEnable = builder.rasterSettings.depthBiasEnable,
			.depthBiasConstantFactor = builder.rasterSettings.depthBiasConstantFactor,
			.depthBiasClamp = builder.rasterSettings.depthBiasClamp,
			.depthBiasSlopeFactor = builder.rasterSettings.depthBiasSlopeFactor,
			.lineWidth = builder.rasterSettings.lineWidth,
		};
		VkPipelineMultisampleStateCreateInfo pmultisamplerStateCI = builder.multisampling.value_or(DEFAULT_MULTISAMPLE_STATE_CI);

		VkPipelineDepthStencilStateCreateInfo pDepthStencilStateCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = {},
			.depthTestEnable = builder.depthTestSettings.enableDepthTest,
			.depthWriteEnable = builder.depthTestSettings.enableDepthWrite,
			.depthCompareOp = builder.depthTestSettings.depthTestCompareOp,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = {},
			.back = {},
			.minDepthBounds = builder.depthTestSettings.minDepthBounds,
			.maxDepthBounds = builder.depthTestSettings.maxDepthBounds,
		};

		VkPipelineViewportStateCreateInfo viewportStateCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.viewportCount = 1,
			.pViewports = &DEFAULT_VIEWPORT,
			.scissorCount = 1,
			.pScissors = &DEFAULT_SCISSOR,
		};

		// create color blend state:
		VkPipelineColorBlendStateCreateInfo pcolorBlendStateCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.pNext = nullptr,
			.attachmentCount = static_cast<u32>(builder.colorAttachmentBlends.size()),
			.pAttachments = builder.colorAttachmentBlends.data(),
			.blendConstants = { 1.0f, 1.0f, 1.0f, 1.0f },
		};

		auto dynamicState = std::array{
			VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
			VkDynamicState::VK_DYNAMIC_STATE_SCISSOR,
		};
		VkPipelineDynamicStateCreateInfo pdynamicStateCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.pNext = nullptr,
			.dynamicStateCount = dynamicState.size(),
			.pDynamicStates = dynamicState.data(),
		};

		VkPipelineRenderingCreateInfoKHR pipelineRenderingCIKHR{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
			.pNext = nullptr,
			.colorAttachmentCount = static_cast<u32>(builder.colorAttachmentFormats.size()),
			.pColorAttachmentFormats = builder.colorAttachmentFormats.data(),
			.depthAttachmentFormat = builder.depthTestSettings.depthAttachmentFormat,
			.stencilAttachmentFormat = VkFormat::VK_FORMAT_UNDEFINED,
		};

		VkGraphicsPipelineCreateInfo pipelineCI{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &pipelineRenderingCIKHR,
			.stageCount = static_cast<u32>(shaderStageCreateInfo.size()),
			.pStages = shaderStageCreateInfo.data(),
			.pVertexInputState = &pVertexInput,
			.pInputAssemblyState = &pinputAssemlyStateCI,
			.pViewportState = &viewportStateCI,
			.pRasterizationState = &prasterizationStateCI,
			.pMultisampleState = &pmultisamplerStateCI,
			.pDepthStencilState = &pDepthStencilStateCI,
			.pColorBlendState = &pcolorBlendStateCI,
			.pDynamicState = &pdynamicStateCI,
			.layout = ret.layout,
			.renderPass = nullptr,							// we do dynamic rendering!
			.subpass = 0,
		};

		auto d = vkCreateGraphicsPipelines(deviceBackend->device.device, nullptr, 1, &pipelineCI, nullptr, &ret.pipeline);
		DAXA_CHECK_VK_RESULT_M(d, "failed to create graphics pipeline");

		ret.setPipelineDebugName(deviceBackend->device.device, builder.debugName.c_str());
		ret.lastRecreationCheckTimePoint = std::chrono::file_clock::now();

		return pipelineHandle;
    }

	Result<PipelineHandle> PipelineCompiler::createComputePipeline(ComputePipelineCreateInfo const& ci) {
		sharedData->currentShaderSeenFiles.clear();

		auto pipelineHandle = PipelineHandle{ std::make_shared<Pipeline>() };
		Pipeline& ret = *pipelineHandle;
		ret.deviceBackend = deviceBackend;
		ret.bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
		ret.creator = ci;

		sharedData->observedHotLoadFiles = &ret.observedHotLoadFiles;

		auto result = tryCreateShaderModule(ci.shaderCI);
		ShaderModuleHandle shader;
		if (result.isOk()) {
			shader = result.value();
		} else {
			return ResultErr{ .message = result.message() };
		}

		VkPipelineShaderStageCreateInfo pipelineShaderStageCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = nullptr,
			.stage = shader->getVkShaderStage(),
			.module = shader->getVkShaderModule(),
			.pName = shader->getVkEntryPoint().c_str(),
		};

		std::vector<VkPushConstantRange> pushConstants;
		std::vector<BindingSetDescription> bindingSetDescriptions;

		reflectShader(shader, pushConstants, bindingSetDescriptions);

        bindingSetDescriptions.resize(std::max(bindingSetDescriptions.size(), ci.overwriteSets.size()));

		for (size_t i = 0; i < MAX_SETS_PER_PIPELINE; i++) {
			if (ci.overwriteSets[i].has_value()) {
				bindingSetDescriptions[i] = ci.overwriteSets[i].value();
			}
		}

		std::vector<VkDescriptorSetLayout> descLayouts = processReflectedDescriptorData(bindingSetDescriptions, *bindSetLayoutCache, ret.bindingSetLayouts);

		if (ci.pushConstantSize > 0) {
			pushConstants.clear();
			pushConstants.push_back(VkPushConstantRange{
    			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
				.offset = 0,
    			.size = static_cast<u32>(ci.pushConstantSize),
			});
		}

		// create pipeline layout:
		VkPipelineLayoutCreateInfo pipelineLayoutCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.setLayoutCount = static_cast<u32>(descLayouts.size()),
			.pSetLayouts = descLayouts.data(),
			.pushConstantRangeCount = static_cast<u32>(pushConstants.size()),
			.pPushConstantRanges = pushConstants.data(),
		};
		DAXA_CHECK_VK_RESULT_M(vkCreatePipelineLayout(deviceBackend->device.device, &pipelineLayoutCI, nullptr, &ret.layout), "failed to create compute pipeline layout");

		VkComputePipelineCreateInfo pipelineCI{
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = pipelineShaderStageCI,
			.layout = ret.layout,
			.basePipelineHandle  = VK_NULL_HANDLE,
			.basePipelineIndex = 0,
		};
		DAXA_CHECK_VK_RESULT_M(vkCreateComputePipelines(deviceBackend->device.device, nullptr, 1, &pipelineCI, nullptr, &ret.pipeline), "failed to create compute pipeline");

		ret.setPipelineDebugName(deviceBackend->device.device, ci.debugName);
		ret.lastRecreationCheckTimePoint = std::chrono::file_clock::now();

		return pipelineHandle;
	}
}