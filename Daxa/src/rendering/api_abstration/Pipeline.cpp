#include "Pipeline.hpp"

#include <iostream>

namespace gpu {

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setViewport(vk::Viewport viewport) {
		this->viewport = viewport;
		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setScissor(vk::Rect2D scissor) { 
		this->scissor = scissor;
		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setVertexInput(const vk::PipelineVertexInputStateCreateInfo& vertexInput) {
		this->vertexInput = vertexInput;
		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setInputAssembly(const vk::PipelineInputAssemblyStateCreateInfo& inputAssembly) {
		this->inputAssembly = inputAssembly;
		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setRasterization(const vk::PipelineRasterizationStateCreateInfo& rasterization) {
		this->rasterization = rasterization;
		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setMultisampling(const vk::PipelineMultisampleStateCreateInfo& multisampling) {
		this->multisampling = multisampling;
		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setDepthStencil(const vk::PipelineDepthStencilStateCreateInfo& depthStencil) {
		this->depthStencil = depthStencil;
		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addShaderStage(const ShaderModule& shaderModule) {

		vk::PipelineShaderStageCreateInfo sci{};
		sci.module = *shaderModule.shaderModule;
		sci.stage = shaderModule.shaderStage;
		sci.pName = shaderModule.name.c_str();

		this->shaderStageCreateInfo.push_back(sci);

		// reflect spirv push constants:
		auto refl = vkh::reflectPushConstants(shaderModule.spirv, shaderModule.shaderStage);
		for (auto& r : refl) {
			if (auto iter = std::find(pushConstants.begin(), pushConstants.end(), r); iter != pushConstants.end()) {
				// if the same push constant is present in multiple stages we combine them to one
				iter->stageFlags |= shaderModule.shaderStage;
			}
			else {
				pushConstants.push_back(r);
			}
		}

		// reflect spirv descriptor sets:
		auto reflDesc = vkh::reflectSetBindings(shaderModule.spirv, shaderModule.shaderStage);
		for (auto& r : reflDesc) {
			if (!descriptorSets.contains(r.first)) {
				descriptorSets[r.first] = std::unordered_map<u32, vk::DescriptorSetLayoutBinding>{};
			}

			for (auto& binding : r.second) {
				if (!descriptorSets[r.first].contains(binding.first)) {
					descriptorSets[r.first][binding.first] = binding.second;
				}
				descriptorSets[r.first][binding.first].stageFlags |= shaderModule.shaderStage;
			}
		}

		return *this;
	}

	constexpr vk::PipelineColorBlendAttachmentState NO_BLEND {
		.blendEnable = VK_FALSE
	};

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addColorAttachment(const vk::AttachmentDescription& attachmentDescription) { 
		this->attachmentDescriptions.push_back(attachmentDescription);
		this->colorAttachmentRefs.push_back(vk::AttachmentReference{
			.attachment = static_cast<u32>(this->attachmentDescriptions.size() - 1),
			.layout = vk::ImageLayout::eUndefined
		});
		this->colorAttachmentBlends.push_back(NO_BLEND);
		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addColorAttachment(const vk::AttachmentDescription& attachmentDescription, const vk::PipelineColorBlendAttachmentState& blend) {
		this->attachmentDescriptions.push_back(attachmentDescription);
		this->colorAttachmentRefs.push_back(vk::AttachmentReference{
			.attachment = static_cast<u32>(this->attachmentDescriptions.size()-1),
			.layout = vk::ImageLayout::eUndefined
		});
		this->colorAttachmentBlends.push_back(blend);
		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addDepthAttachment(const vk::AttachmentDescription& attachmentDescription) {
		this->attachmentDescriptions.push_back(attachmentDescription);
		this->depthAttachmentRef = vk::AttachmentReference{
			.attachment = static_cast<u32>(this->attachmentDescriptions.size() - 1),
			.layout = vk::ImageLayout::eUndefined
		};
		return *this;
	}

	constexpr vk::PipelineRasterizationStateCreateInfo DEFAULT_RASTER_STATE_CI{
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eNone,
		.frontFace = vk::FrontFace::eClockwise,
		.lineWidth = 1.0f,
	};

	constexpr vk::PipelineInputAssemblyStateCreateInfo DEFAULT_INPUT_ASSEMBLY_STATE_CI{
		.topology = vk::PrimitiveTopology::eTriangleList
	};

	constexpr vk::PipelineMultisampleStateCreateInfo DEFAULT_MULTISAMPLE_STATE_CI{
		.minSampleShading = 1.0f
	};

	constexpr vk::PipelineDepthStencilStateCreateInfo DEFAULT_DEPTH_STENCIL_STATE_CI{

	};

	constexpr vk::Viewport DEFAULT_VIEWPORT{
		.width = 1,
		.height = 1
	};

	GraphicsPipelineHandle GraphicsPipelineBuilder::build(vk::Device device, vkh::DescriptorSetLayoutCache& layoutCache) {
		if (!vertexInput) {
			std::cerr << "error: vertexInput must be specified when building a GraphicsPipeline!" << std::endl;
			exit(-1);
		}

		std::vector<vk::DescriptorSetLayout> descLayouts;
		{
			std::vector<vk::DescriptorSetLayoutBinding> tempBindings;
			for (auto& [index, bindings] : this->descriptorSets) {
				tempBindings.clear();
				for (auto& [index, binding] : bindings) {
					tempBindings.push_back(binding);
				}
				descLayouts.push_back(layoutCache.getLayout(tempBindings));
			}
		}

		// create pipeline layout:
		vk::PipelineLayoutCreateInfo lci{};
		lci.setLayoutCount = static_cast<u32>(descLayouts.size());
		lci.pSetLayouts = descLayouts.data();
		lci.pushConstantRangeCount = static_cast<u32>(pushConstants.size());
		lci.pPushConstantRanges = pushConstants.data();

		auto pipelineLayout = device.createPipelineLayoutUnique(lci);

		// create pipeline:
		vk::PipelineInputAssemblyStateCreateInfo pinputAssemlyStateCI	= inputAssembly.value_or(DEFAULT_INPUT_ASSEMBLY_STATE_CI);
		vk::PipelineRasterizationStateCreateInfo prasterizationStateCI	= rasterization.value_or(DEFAULT_RASTER_STATE_CI);
		vk::PipelineMultisampleStateCreateInfo pmultisamplerStateCI		= multisampling.value_or(DEFAULT_MULTISAMPLE_STATE_CI);
		vk::PipelineDepthStencilStateCreateInfo pDepthStencilStateCI	= depthStencil.value_or(DEFAULT_DEPTH_STENCIL_STATE_CI);
		vk::Viewport pviewport											= viewport.value_or(DEFAULT_VIEWPORT);
		const vk::Rect2D DEFAULT_SCISSOR{ .extent = {static_cast<uint32_t>(pviewport.width), static_cast<uint32_t>(pviewport.height)} };
		vk::Rect2D pscissor												= scissor.value_or(DEFAULT_SCISSOR);
		
		vk::PipelineViewportStateCreateInfo viewportStateCI{};
		viewportStateCI.viewportCount = 1;
		viewportStateCI.pViewports = &pviewport;
		viewportStateCI.scissorCount = 1;
		viewportStateCI.pScissors = &pscissor;

		// create color blend state:
		vk::PipelineColorBlendStateCreateInfo pcolorBlendStateCI{};
		pcolorBlendStateCI.attachmentCount = static_cast<u32>(this->colorAttachmentBlends.size());
		pcolorBlendStateCI.pAttachments = this->colorAttachmentBlends.data();
		pcolorBlendStateCI.blendConstants = std::array{ 1.0f, 1.0f, 1.0f, 1.0f };

		auto dynamicState = std::array{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor,
		};
		vk::PipelineDynamicStateCreateInfo pdynamicStateCI{};
		pdynamicStateCI.dynamicStateCount = dynamicState.size();
		pdynamicStateCI.pDynamicStates = dynamicState.data();

		vk::SubpassDescription subpassDescription{};
		subpassDescription.colorAttachmentCount = static_cast<u32>(this->colorAttachmentRefs.size());
		subpassDescription.pColorAttachments = this->colorAttachmentRefs.data();
		subpassDescription.pDepthStencilAttachment = this->depthAttachmentRef.has_value() ? &this->depthAttachmentRef.value() : nullptr;

		// create dummy renderpass:
		vk::RenderPassCreateInfo rpci{};
		rpci.attachmentCount = static_cast<u32>(this->attachmentDescriptions.size());
		rpci.pAttachments = this->attachmentDescriptions.data();
		rpci.subpassCount = 1;
		rpci.pSubpasses = &subpassDescription;

		auto dummyRenderPass = device.createRenderPassUnique(rpci);

		vk::GraphicsPipelineCreateInfo pci{};
		pci.stageCount = static_cast<u32>(this->shaderStageCreateInfo.size());	
		pci.pStages = shaderStageCreateInfo.data();
		pci.pVertexInputState = &vertexInput.value();
		pci.pInputAssemblyState = &pinputAssemlyStateCI;
		pci.pViewportState = &viewportStateCI;
		pci.pRasterizationState = &prasterizationStateCI;
		pci.pMultisampleState = &pmultisamplerStateCI;
		pci.pDepthStencilState = &pDepthStencilStateCI;
		pci.pColorBlendState = &pcolorBlendStateCI;
		pci.pDynamicState = &pdynamicStateCI;
		pci.layout = *pipelineLayout;
		pci.renderPass = *dummyRenderPass;
		pci.subpass = 0;

		auto pipeline = device.createGraphicsPipelineUnique(nullptr, pci);

		GraphicsPipeline gpipeline;
		gpipeline.pipeline = std::move(pipeline);
		gpipeline.layout = std::move(pipelineLayout);
		GraphicsPipelineHandle pipelineHandle;
		pipelineHandle.pipeline = std::make_shared<GraphicsPipeline>(std::move(gpipeline));
		return pipelineHandle;
	}
}
