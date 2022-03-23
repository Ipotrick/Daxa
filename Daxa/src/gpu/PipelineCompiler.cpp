#include "PipelineCompiler.hpp"

#include <fstream>

using Path = std::filesystem::path;

using namespace daxa::gpu;

namespace daxa {
	class FileIncluder : public shaderc::CompileOptions::IncluderInterface {
	public:
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

						char* data = new char[str.size()+1];
						for (size_t i = 0; i < str.size()+1; i++) {
							data[i] = str[i];
						}
						res->content_length = str.size();
						res->content = data;

						sharedData->currentShaderSeenFiles.push_back(requested_source);
						sharedData->observedHotLoadFiles->insert({requested_source_path,std::filesystem::last_write_time(requested_source_path)});
					} else {
						// double includes are ignored
						res->content = "";
						res->content_length = 0;
					}
				} else {
					// file path could not be resolved
					res->content = "could not find file";
					res->content_length = strlen(res->content);

					res->source_name = "";
					res->source_name_length = 0;
				}
			}
			else {
				// max include depth exceeded
				res->content = "current include depth of 10 was exceeded";
				res->content_length = strlen(res->content);

				res->source_name = "";
				res->source_name_length = 0;
			}
			return res;
		}

    	virtual void ReleaseInclude(shaderc_include_result* data) override {
			if (data->source_name_length > 0 && data->content != "") {
				delete data->content;
			}

			delete data;
		};

		std::shared_ptr<PipelineCompilerShadedData> sharedData = {};
	};

    PipelineCompiler::PipelineCompiler(std::shared_ptr<gpu::DeviceBackend> deviceBackend, std::shared_ptr<gpu::BindingSetLayoutCache> bindSetLayoutCache) 
		: deviceBackend{ deviceBackend }
		, bindSetLayoutCache{ bindSetLayoutCache }
		, sharedData{ std::make_shared<PipelineCompilerShadedData>() }
	{
		auto includer = std::make_unique<FileIncluder>();
		includer->sharedData = sharedData;
		options.SetIncluder(std::move(includer));
	}

    bool PipelineCompiler::checkIfSourcesChanged(gpu::PipelineHandle& pipeline) {
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
		return reload;
	}

	Result<gpu::PipelineHandle> PipelineCompiler::recreatePipeline(gpu::PipelineHandle const& pipeline) {
		auto handleResult = [&](Result<gpu::PipelineHandle> const& result) -> Result<gpu::PipelineHandle> {
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
		}
	}

    void PipelineCompiler::addShaderSourceRootPath(Path const& root) {
        this->sharedData->rootPaths.push_back(root); 
    }

    Result<Path> PipelineCompilerShadedData::findFullPathOfFile(Path const& file) {
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

    Result<gpu::PipelineHandle> PipelineCompiler::createGraphicsPipeline(gpu::GraphicsPipelineBuilder const& builder) {
        DAXA_ASSERT_M(!builder.bVertexAtrributeBindingBuildingOpen, "vertex attribute bindings must be completed before creating a pipeline");

		auto pipelineHandle = gpu::PipelineHandle{ std::make_shared<gpu::Pipeline>() };
		gpu::Pipeline& ret = *pipelineHandle;
		ret.deviceBackend = deviceBackend;
		ret.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		ret.colorAttachmentFormats = builder.colorAttachmentFormats;
		ret.depthAttachment = builder.depthTestSettings.depthAttachmentFormat;
		ret.stencilAttachment = VK_FORMAT_UNDEFINED;
		ret.creator = builder;

		sharedData->observedHotLoadFiles = &ret.observedHotLoadFiles;

		std::vector<VkPushConstantRange> pushConstants;
		std::vector<gpu::BindingSetDescription> bindingSetDescriptions;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo;
		std::vector<gpu::ShaderModuleHandle> shaderModules;
		shaderModules.reserve(builder.shaderModuleCIs.size());

		for (auto& shaderCI : builder.shaderModuleCIs) {
			sharedData->currentShaderSeenFiles.clear();
			auto result = gpu::ShaderModuleHandle::tryCreateDAXAShaderModule(deviceBackend, shaderCI, compiler, options, ret.observedHotLoadFiles);
			if (result.isOk()) {
				shaderModules.push_back(result.value());
			} else {
				return ResultErr{ .message = result.message() };
			}
		}

		std::cout << "compiled pipeline with the following observed files for hot reloading:" << std::endl;
		for (auto& [path, lastWriteTime] : ret.observedHotLoadFiles) {
			std::cout << "  path: " << path << std::endl;
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

		setPipelineDebugName(deviceBackend->device.device, builder.debugName.c_str(), ret);

		return pipelineHandle;
    }

	Result<gpu::PipelineHandle> PipelineCompiler::createComputePipeline(gpu::ComputePipelineCreateInfo const& ci) {
		sharedData->currentShaderSeenFiles.clear();

		auto pipelineHandle = PipelineHandle{ std::make_shared<Pipeline>() };
		Pipeline& ret = *pipelineHandle;
		ret.deviceBackend = deviceBackend;
		ret.bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
		ret.creator = ci;

		sharedData->observedHotLoadFiles = &ret.observedHotLoadFiles;

		auto result = gpu::ShaderModuleHandle::tryCreateDAXAShaderModule(deviceBackend, ci.shaderCI, compiler, options, ret.observedHotLoadFiles);
		gpu::ShaderModuleHandle shader;
		if (result.isOk()) {
			shader = result.value();
		} else {
			return ResultErr{ .message = result.message() };
		}

		std::cout << "compiled pipeline with the following observed files for hot reloading:" << std::endl;
		for (auto& [path, lastWriteTime] : ret.observedHotLoadFiles) {
			std::cout << "  path: " << path << std::endl;
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

		for (size_t i = 0; i < MAX_SETS_PER_PIPELINE; i++) {
			if (ci.overwriteSets[i].has_value()) {
				bindingSetDescriptions[i] = ci.overwriteSets[i].value();
			}
		}

		std::vector<VkDescriptorSetLayout> descLayouts = processReflectedDescriptorData(bindingSetDescriptions, *bindSetLayoutCache, ret.bindingSetLayouts);

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

		setPipelineDebugName(deviceBackend->device.device, ci.debugName, ret);

		return pipelineHandle;
	}
}