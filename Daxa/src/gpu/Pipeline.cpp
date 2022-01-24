#include "Pipeline.hpp"

#include <iostream>

#include <spirv_reflect.h>

#include "Instance.hpp"

namespace daxa {
	namespace gpu {
		void setPipelineDebugName(VkDevice device, char const* debugName, Pipeline& pipeline) {
			if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && debugName != nullptr) {
				VkDebugUtilsObjectNameInfoEXT nameInfo {
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = NULL,
					.objectType = VK_OBJECT_TYPE_PIPELINE,
					.objectHandle = (uint64_t)pipeline.pipeline,
					.pObjectName = debugName,
				};
				instance->pfnSetDebugUtilsObjectNameEXT(device, &nameInfo);
				pipeline.debugName = debugName;
			}
		}

		std::vector<VkDescriptorSetLayout> processReflectedDescriptorData(
			std::vector<std::unordered_map<u32, VkDescriptorSetLayoutBinding>>& descriptorSets,
			BindingSetLayoutCache& descCache,
			std::array<std::shared_ptr<BindingSetLayout const>, MAX_SETS_PER_PIPELINE>& bindingSetDescriptions
		) {
			std::vector<VkDescriptorSetLayout> descLayouts;
			BindingSetDescription description;
			for (i32 index = 0; index < descriptorSets.size(); index++) {
				auto& bindings = descriptorSets[index];
				DAXA_ASSERT_M(!bindings.empty(), "binding sets indices must be used in ascending order starting with 0");
				description.bindingsCount = 0;
				for (auto& [index, binding] : bindings) {
					description.bindings[description.bindingsCount++] = binding;
				}
				auto info = descCache.getInfoShared(description);
				descLayouts.push_back(info->getVkDescriptorSetLayout());
				bindingSetDescriptions[index] = info;
			}
			return std::move(descLayouts);
		}

		std::size_t sizeofFormat(VkFormat format) {
			if (format == VkFormat::VK_FORMAT_UNDEFINED)
				return -1;
			if (format == VkFormat::VK_FORMAT_R4G4_UNORM_PACK8)
				return 1;
			if (format >= VkFormat::VK_FORMAT_R4G4B4A4_UNORM_PACK16 && format <= VkFormat::VK_FORMAT_A1R5G5B5_UNORM_PACK16)
				return 2;
			if (format >= VkFormat::VK_FORMAT_R8_UNORM && format <= VkFormat::VK_FORMAT_R8_SRGB)
				return 1;
			if (format >= VkFormat::VK_FORMAT_R8G8_UNORM && format <= VkFormat::VK_FORMAT_R8G8_SRGB)
				return 2;
			if (format >= VkFormat::VK_FORMAT_R8G8B8_UNORM && format <= VkFormat::VK_FORMAT_R8G8B8_SRGB)
				return 3;
			if (format >= VkFormat::VK_FORMAT_R8G8B8_UNORM && format <= VkFormat::VK_FORMAT_A2B10G10R10_SINT_PACK32)
				return 4;
			if (format >= VkFormat::VK_FORMAT_R16_UNORM && format <= VkFormat::VK_FORMAT_R16_SFLOAT)
				return 2;
			if (format >= VkFormat::VK_FORMAT_R16G16_UNORM && format <= VkFormat::VK_FORMAT_R16G16_SFLOAT)
				return 4;
			if (format >= VkFormat::VK_FORMAT_R16G16B16_UNORM && format <= VkFormat::VK_FORMAT_R16G16B16_SFLOAT)
				return 6;
			if (format >= VkFormat::VK_FORMAT_R16G16B16A16_UNORM && format <= VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT)
				return 8;
			if (format >= VkFormat::VK_FORMAT_R32_UINT && format <= VkFormat::VK_FORMAT_R32_SFLOAT)
				return 4;
			if (format >= VkFormat::VK_FORMAT_R32G32_UINT && format <= VkFormat::VK_FORMAT_R32G32_SFLOAT)
				return 8;
			if (format >= VkFormat::VK_FORMAT_R32G32B32_UINT && format <= VkFormat::VK_FORMAT_R32G32B32_SFLOAT)
				return 12;
			if (format >= VkFormat::VK_FORMAT_R32G32B32A32_UINT && format <= VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT)
				return 16;
			if (format >= VkFormat::VK_FORMAT_R64_UINT && format <= VkFormat::VK_FORMAT_R64_SFLOAT)
				return 8;
			if (format >= VkFormat::VK_FORMAT_R64G64_UINT && format <= VkFormat::VK_FORMAT_R64G64_SFLOAT)
				return 16;
			if (format >= VkFormat::VK_FORMAT_R64G64B64_UINT && format <= VkFormat::VK_FORMAT_R64G64B64_SFLOAT)
				return 24;
			if (format >= VkFormat::VK_FORMAT_R64G64B64A64_UINT && format <= VkFormat::VK_FORMAT_R64G64B64A64_SFLOAT)
				return 32;
			if (format == VkFormat::VK_FORMAT_B10G11R11_UFLOAT_PACK32 || format == VkFormat::VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)
				return 32;
			if (format == VkFormat::VK_FORMAT_D16_UNORM)
				return 16;
			if (format == VkFormat::VK_FORMAT_X8_D24_UNORM_PACK32 || format == VkFormat::VK_FORMAT_D32_SFLOAT)
				return 32;
			if (format == VkFormat::VK_FORMAT_S8_UINT)
				return 8;
			return -1;
		}

		Pipeline::~Pipeline() {
			if (deviceBackend) {
				vkDestroyPipelineLayout(deviceBackend->device.device, layout, nullptr);
				vkDestroyPipeline(deviceBackend->device.device, pipeline, nullptr);
				deviceBackend = {};
			}
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::configurateDepthTest(DepthTestSettings const& depthTestSettings2) {
			depthTestSettings = depthTestSettings2;
			return *this;
		}
		
		GraphicsPipelineBuilder& GraphicsPipelineBuilder::setDebugName(char const* name) {
			debugName = name;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::beginVertexInputAttributeBinding(VkVertexInputRate inputRate) {
			if (bVertexAtrributeBindingBuildingOpen) {
				endVertexInputAttributeBinding();
			}
			bVertexAtrributeBindingBuildingOpen = true;
			currentVertexAttributeBindingInputRate = inputRate;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::addVertexInputAttributeSpacer(u32 spacing) {
			DAXA_ASSERT_M(bVertexAtrributeBindingBuildingOpen, "can not record attributes with no vertex binding");
			currentVertexAttributeBindingOffset += spacing;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::addVertexInputAttribute(VkFormat format) {
			vertexInputAttributeDescriptions.push_back({
				.location = currentVertexAttributeLocation,
				.binding = (u32)vertexInputBindingDescriptions.size(),
				.format = format,
				.offset = currentVertexAttributeBindingOffset,
			});
			currentVertexAttributeLocation += 1;
			currentVertexAttributeBindingOffset += (u32)sizeofFormat(format);
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::endVertexInputAttributeBinding() {
			DAXA_ASSERT_M(bVertexAtrributeBindingBuildingOpen, "can not end vertex binding if there is no binding beeing recorded");
			bVertexAtrributeBindingBuildingOpen = false;

			vertexInputBindingDescriptions.push_back({
				.binding = (u32)vertexInputBindingDescriptions.size(),
				.stride = currentVertexAttributeBindingOffset,
				.inputRate = currentVertexAttributeBindingInputRate,
			});
			currentVertexAttributeBindingOffset = 0;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::setInputAssembly(const VkPipelineInputAssemblyStateCreateInfo& inputAssembly) {
			this->inputAssembly = inputAssembly;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::setRasterization(RasterSettings const& rasterization) {
			this->rasterSettings = rasterization;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::setMultisampling(const VkPipelineMultisampleStateCreateInfo& multisampling) {
			this->multisampling = multisampling;
			return *this;
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

		std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>>
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

			std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>> setMap;
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
			std::vector<std::unordered_map<u32, VkDescriptorSetLayoutBinding>>& descriptorSets
		) {
			auto falserefl = reflectPushConstants(shaderModule->getSPIRV(), shaderModule->getVkShaderStage());
			auto refl2 = reinterpret_cast<std::vector<VkPushConstantRange>*>(&falserefl);
			auto& refl = *refl2;
			for (auto& r : refl) {
				auto iter = pushConstants.begin();
				for (; iter != pushConstants.end(); iter++) {
					if (iter->offset == r.offset && iter->size == r.size && iter->stageFlags == r.stageFlags) break;
				}

				if (iter != pushConstants.end()) {
					// if the same push constant is present in multiple stages we combine them to one
					iter->stageFlags |= (VkShaderStageFlagBits)shaderModule->getVkShaderStage();
				}
				else {
					pushConstants.push_back(r);
				}
			}

			// reflect spirv descriptor sets:
			auto reflDesc = reflectSetBindings(shaderModule->getSPIRV(), shaderModule->getVkShaderStage());
			for (auto& r : reflDesc) {
				if (r.first >= descriptorSets.size()) {
					descriptorSets.resize(r.first+1, {});
				}

				for (auto& binding : r.second) {
					if (!descriptorSets[r.first].contains(binding.first)) {
						descriptorSets[r.first][binding.first] = binding.second;
					}
					descriptorSets[r.first][binding.first].stageFlags |= (VkShaderStageFlagBits)shaderModule->getVkShaderStage();
				}
			}
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::addShaderStage(ShaderModuleHandle const& shaderModule) {
			auto pred = [=](ShaderModuleHandle const& a){
				return a->getVkShaderStage() == shaderModule->getVkShaderStage();
			};

			if (auto place = std::find_if(this->shaderModules.begin(), this->shaderModules.end(), pred); place != this->shaderModules.end()) {
				*place = shaderModule;
			} else {
				this->shaderModules.push_back(shaderModule);
			}

			return *this;
		}

		constexpr VkPipelineColorBlendAttachmentState NO_BLEND{
			.blendEnable = VK_FALSE,
			.colorWriteMask = (VkColorComponentFlags)(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT),
		};

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::addColorAttachment(const VkFormat& attachmentFormat) {
			this->colorAttachmentFormats.push_back(attachmentFormat);
			this->colorAttachmentBlends.push_back(NO_BLEND);
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::addColorAttachment(const VkFormat& attachmentFormat, const VkPipelineColorBlendAttachmentState& blend) {
			this->colorAttachmentFormats.push_back(attachmentFormat);
			this->colorAttachmentBlends.push_back(blend);
			return *this;
		}

		constexpr VkPipelineRasterizationStateCreateInfo DEFAULT_RASTER_STATE_CI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL,
			.cullMode = VkCullModeFlagBits::VK_CULL_MODE_NONE,
			.frontFace = VkFrontFace::VK_FRONT_FACE_CLOCKWISE,
			.lineWidth = 1.0f,
		};

		constexpr VkPipelineInputAssemblyStateCreateInfo DEFAULT_INPUT_ASSEMBLY_STATE_CI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.pNext = nullptr,
			.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		};

		constexpr VkPipelineMultisampleStateCreateInfo DEFAULT_MULTISAMPLE_STATE_CI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.pNext = nullptr,
			.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
			.minSampleShading = 1.0f,
		};

		constexpr VkViewport DEFAULT_VIEWPORT{
			.width = 1,
			.height = 1
		};

		constexpr VkRect2D DEFAULT_SCISSOR{
			.extent = {1,1},
		};

		constexpr VkPipelineVertexInputStateCreateInfo DEFAULT_VERTEX_INPUT_STATE_CI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = nullptr,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = nullptr,
		};

		daxa::Result<PipelineHandle> GraphicsPipelineBuilder::build(std::shared_ptr<DeviceBackend>& deviceBackend, BindingSetLayoutCache& bindingSetCache) {
			if (bVertexAtrributeBindingBuildingOpen) {
				endVertexInputAttributeBinding();
			}

			for (auto& shaderModule : this->shaderModules) {
				VkPipelineShaderStageCreateInfo pipelineShaderStageCI{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = nullptr,
					.stage = shaderModule->shaderStage,
					.module = shaderModule->shaderModule,
					.pName = shaderModule->entryPoint.c_str(),
				};
				this->shaderStageCreateInfo.push_back(pipelineShaderStageCI);

				reflectShader(shaderModule, pushConstants, descriptorSets);
			}

			auto pipelineHandle = PipelineHandle{ std::make_shared<Pipeline>() };
			Pipeline& ret = *pipelineHandle;
			ret.deviceBackend = deviceBackend;
			ret.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			ret.colorAttachmentFormats = colorAttachmentFormats;
			ret.depthAttachment = depthTestSettings.depthAttachmentFormat;
			ret.stencilAttachment = VK_FORMAT_UNDEFINED;

			std::vector<VkDescriptorSetLayout> descLayouts = processReflectedDescriptorData(this->descriptorSets, bindingSetCache, ret.bindingSetLayouts);

			// create pipeline layout:
			VkPipelineLayoutCreateInfo pipelineLayoutCI{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.pNext = nullptr,
				.setLayoutCount = static_cast<u32>(descLayouts.size()),
				.pSetLayouts = descLayouts.data(),
				.pushConstantRangeCount = static_cast<u32>(pushConstants.size()),
				.pPushConstantRanges = pushConstants.data(),
			};
			vkCreatePipelineLayout(deviceBackend->device.device, &pipelineLayoutCI, nullptr, &ret.layout);

			// create pipeline:
			VkPipelineVertexInputStateCreateInfo pVertexInput = VkPipelineVertexInputStateCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.vertexBindingDescriptionCount = (u32)vertexInputBindingDescriptions.size(),
				.pVertexBindingDescriptions = (VkVertexInputBindingDescription*)vertexInputBindingDescriptions.data(),
				.vertexAttributeDescriptionCount = (u32)vertexInputAttributeDescriptions.size(),
				.pVertexAttributeDescriptions = (VkVertexInputAttributeDescription*)vertexInputAttributeDescriptions.data(),
			};
			VkPipelineInputAssemblyStateCreateInfo pinputAssemlyStateCI = inputAssembly.value_or(DEFAULT_INPUT_ASSEMBLY_STATE_CI);
			VkPipelineRasterizationStateCreateInfo prasterizationStateCI{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				.pNext = nullptr,
				.depthClampEnable = (VkBool32)rasterSettings.depthBiasClamp,
				.rasterizerDiscardEnable = rasterSettings.rasterizerDiscardEnable,
				.polygonMode = rasterSettings.polygonMode,
				.cullMode = (VkBool32)rasterSettings.cullMode,
				.frontFace = rasterSettings.frontFace,
				.depthBiasEnable = rasterSettings.depthBiasEnable,
				.depthBiasConstantFactor = rasterSettings.depthBiasConstantFactor,
				.depthBiasClamp = rasterSettings.depthBiasClamp,
				.depthBiasSlopeFactor = rasterSettings.depthBiasSlopeFactor,
				.lineWidth = rasterSettings.lineWidth,
			};
			VkPipelineMultisampleStateCreateInfo pmultisamplerStateCI = multisampling.value_or(DEFAULT_MULTISAMPLE_STATE_CI);

			VkPipelineDepthStencilStateCreateInfo pDepthStencilStateCI{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = {},
				.depthTestEnable = depthTestSettings.enableDepthTest,
				.depthWriteEnable = depthTestSettings.enableDepthWrite,
				.depthCompareOp = depthTestSettings.depthTestCompareOp,
				.depthBoundsTestEnable = VK_FALSE,
				.stencilTestEnable = VK_FALSE,
				.front = {},
				.back = {},
				.minDepthBounds = depthTestSettings.minDepthBounds,
				.maxDepthBounds = depthTestSettings.maxDepthBounds,
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
				.attachmentCount = static_cast<u32>(this->colorAttachmentBlends.size()),
				.pAttachments = this->colorAttachmentBlends.data(),
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
				.colorAttachmentCount = static_cast<u32>(this->colorAttachmentFormats.size()),
				.pColorAttachmentFormats = this->colorAttachmentFormats.data(),
				.depthAttachmentFormat = this->depthTestSettings.depthAttachmentFormat,
				.stencilAttachmentFormat = VkFormat::VK_FORMAT_UNDEFINED,
			};

			VkGraphicsPipelineCreateInfo pipelineCI{
				.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.pNext = &pipelineRenderingCIKHR,
				.stageCount = static_cast<u32>(this->shaderStageCreateInfo.size()),
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

			auto err = vkCreateGraphicsPipelines(deviceBackend->device.device, nullptr, 1, &pipelineCI, nullptr, &ret.pipeline);
			DAXA_ASSERT_M(err == VK_SUCCESS, "could not create graphics pipeline");

			setPipelineDebugName(deviceBackend->device.device, debugName, ret);

			this->pushConstants.clear();
			this->descriptorSets.clear();
			this->shaderStageCreateInfo.clear();

			return pipelineHandle;
		}

		daxa::Result<PipelineHandle> createComputePipeline(std::shared_ptr<DeviceBackend>& deviceBackend, BindingSetLayoutCache& bindingSetCache, ComputePipelineCreateInfo const& ci) {
			auto pipelineHandle = PipelineHandle{ std::make_shared<Pipeline>() };
			Pipeline& ret = *pipelineHandle;
			ret.deviceBackend = deviceBackend;
			ret.bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

			VkPipelineShaderStageCreateInfo pipelineShaderStageCI{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = nullptr,
				.stage = ci.shaderModule->getVkShaderStage(),
				.module = ci.shaderModule->getVkShaderModule(),
				.pName = ci.shaderModule->getVkEntryPoint().c_str(),
			};

			std::vector<VkPushConstantRange> pushConstants;
			std::vector<std::unordered_map<u32, VkDescriptorSetLayoutBinding>> descriptorSets;

			reflectShader(ci.shaderModule, pushConstants, descriptorSets);

			std::vector<VkDescriptorSetLayout> descLayouts = processReflectedDescriptorData(descriptorSets, bindingSetCache, ret.bindingSetLayouts);

			// create pipeline layout:
			VkPipelineLayoutCreateInfo pipelineLayoutCI{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.pNext = nullptr,
				.setLayoutCount = static_cast<u32>(descLayouts.size()),
				.pSetLayouts = descLayouts.data(),
				.pushConstantRangeCount = static_cast<u32>(pushConstants.size()),
				.pPushConstantRanges = pushConstants.data(),
			};
			vkCreatePipelineLayout(deviceBackend->device.device, &pipelineLayoutCI, nullptr, &ret.layout);

			VkComputePipelineCreateInfo pipelineCI{
				.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
				.pNext = nullptr,
				.layout = ret.layout,
				.stage = pipelineShaderStageCI,
				.flags = 0,
				.basePipelineHandle  = VK_NULL_HANDLE,
				.basePipelineIndex = 0,
			};
			vkCreateComputePipelines(deviceBackend->device.device, nullptr, 1, &pipelineCI, nullptr, &ret.pipeline);

			setPipelineDebugName(deviceBackend->device.device, ci.debugName, ret);

			return pipelineHandle;
		}
	}
}