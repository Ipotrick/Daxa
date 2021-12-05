#include "Pipeline.hpp"

#include <iostream>

#include "common.hpp"

namespace daxa {
	namespace gpu {
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

		DAXA_DEFINE_TRIVIAL_MOVE(GraphicsPipeline)

		GraphicsPipeline::~GraphicsPipeline() {
			if (device) {
				vkDestroyPipelineLayout(device, layout, nullptr);
				vkDestroyPipeline(device, pipeline, nullptr);
				std::memset(this, 0, sizeof(GraphicsPipeline));
			}
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
			currentVertexAttributeLocation = 0;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::setInputAssembly(const VkPipelineInputAssemblyStateCreateInfo& inputAssembly) {
			this->inputAssembly = inputAssembly;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::setRasterization(const VkPipelineRasterizationStateCreateInfo& rasterization) {
			this->rasterization = rasterization;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::setMultisampling(const VkPipelineMultisampleStateCreateInfo& multisampling) {
			this->multisampling = multisampling;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::setDepthStencil(const VkPipelineDepthStencilStateCreateInfo& depthStencil) {
			this->depthStencil = depthStencil;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::addShaderStage(const ShaderModuleHandle& shaderModule) {
			VkPipelineShaderStageCreateInfo pipelineShaderStageCI{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = nullptr,
				.stage = shaderModule->shaderStage,
				.module = shaderModule->shaderModule,
				.pName = shaderModule->entryPoint.c_str(),
			};
			this->shaderStageCreateInfo.push_back(pipelineShaderStageCI);

			// reflect spirv push constants:
			auto falserefl = vkh::reflectPushConstants(shaderModule->spirv, (vk::ShaderStageFlagBits)shaderModule->shaderStage);
			auto refl2 = reinterpret_cast<std::vector<VkPushConstantRange>*>(&falserefl);
			auto& refl = *refl2;
			for (auto& r : refl) {
				auto iter = pushConstants.begin();
				for (; iter != pushConstants.end(); iter++) {
					if (iter->offset == r.offset && iter->size == r.size && iter->stageFlags == r.stageFlags) break;
				}

				if (iter != pushConstants.end()) {
					// if the same push constant is present in multiple stages we combine them to one
					iter->stageFlags |= (VkShaderStageFlagBits)shaderModule->shaderStage;
				}
				else {
					pushConstants.push_back(r);
				}
			}

			// reflect spirv descriptor sets:
			auto reflDesc = vkh::reflectSetBindings(shaderModule->spirv, (vk::ShaderStageFlagBits)shaderModule->shaderStage);
			for (auto& r : reflDesc) {
				if (!descriptorSets.contains(r.first)) {
					descriptorSets[r.first] = std::unordered_map<u32, VkDescriptorSetLayoutBinding>{};
				}

				for (auto& binding : r.second) {
					if (!descriptorSets[r.first].contains(binding.first)) {
						descriptorSets[r.first][binding.first] = binding.second;
					}
					descriptorSets[r.first][binding.first].stageFlags |= (VkShaderStageFlagBits)shaderModule->shaderStage;
				}
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

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::addDepthAttachment(const VkFormat& attachmentFormat) {
			this->depthAttachmentFormat = attachmentFormat;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::addStencilAttachment(const VkFormat& attachmentFormat) {
			this->stencilAttachmentFormat = attachmentFormat;
			return *this;
		}

		constexpr VkPipelineRasterizationStateCreateInfo DEFAULT_RASTER_STATE_CI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL,
			.cullMode = VkCullModeFlagBits::VK_CULL_MODE_NONE,
			.frontFace = VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.lineWidth = 1.0f,
		};

		constexpr VkPipelineInputAssemblyStateCreateInfo DEFAULT_INPUT_ASSEMBLY_STATE_CI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.pNext = nullptr,
			.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
		};

		constexpr VkPipelineMultisampleStateCreateInfo DEFAULT_MULTISAMPLE_STATE_CI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.pNext = nullptr,
			.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
			.minSampleShading = 1.0f,
		};

		constexpr VkPipelineDepthStencilStateCreateInfo DEFAULT_DEPTH_STENCIL_STATE_CI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = nullptr,
			.depthTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
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

		GraphicsPipelineHandle GraphicsPipelineBuilder::build(VkDevice device, BindingSetDescriptionCache& bindingSetCache) {
			if (bVertexAtrributeBindingBuildingOpen) {
				endVertexInputAttributeBinding();
			}

			GraphicsPipeline ret;
			ret.device = device;

			std::vector<VkDescriptorSetLayout> descLayouts;
			{
				std::vector<VkDescriptorSetLayoutBinding> tempBindings;
				for (auto& [index, bindings] : this->descriptorSets) {
					tempBindings.clear();
					for (auto& [index, binding] : bindings) {
						tempBindings.push_back(binding);
					}
					auto description = bindingSetCache.getSetDescription(tempBindings);
					descLayouts.push_back(description->layout);
					ret.bindingSetDescriptions[index] = description;
				}
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
			vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &ret.layout);

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
			VkPipelineRasterizationStateCreateInfo prasterizationStateCI = rasterization.value_or(DEFAULT_RASTER_STATE_CI);
			VkPipelineMultisampleStateCreateInfo pmultisamplerStateCI = multisampling.value_or(DEFAULT_MULTISAMPLE_STATE_CI);
			VkPipelineDepthStencilStateCreateInfo pDepthStencilStateCI = depthStencil.value_or(DEFAULT_DEPTH_STENCIL_STATE_CI);

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
				.depthAttachmentFormat = this->depthAttachmentFormat.value_or(VkFormat::VK_FORMAT_UNDEFINED),
				.stencilAttachmentFormat = this->stencilAttachmentFormat.value_or(VkFormat::VK_FORMAT_UNDEFINED),
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

			auto err = vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCI, nullptr, &ret.pipeline);
			DAXA_ASSERT_M(err == VK_SUCCESS, "could not create graphics pipeline");

			GraphicsPipelineHandle pipelineHandle;
			pipelineHandle.pipeline = std::make_shared<GraphicsPipeline>(std::move(ret));
			return pipelineHandle;
		}
	}
}