#include "Pipeline.hpp"

#include <iostream>

#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"
#include "util.hpp"

namespace daxa {
	void Pipeline::setPipelineDebugName(VkDevice device, char const* debugName) {
		if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && debugName != nullptr) {
			VkDebugUtilsObjectNameInfoEXT nameInfo {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_PIPELINE,
				.objectHandle = (uint64_t)pipeline,
				.pObjectName = debugName,
			};
			instance->pfnSetDebugUtilsObjectNameEXT(device, &nameInfo);
			VkDebugUtilsObjectNameInfoEXT nameInfoLayout {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT,
				.objectHandle = (uint64_t)layout,
				.pObjectName = debugName,
			};
			instance->pfnSetDebugUtilsObjectNameEXT(device, &nameInfoLayout);
			debugName = debugName;
		}
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

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addShaderStage(ShaderModuleCreateInfo const& createInfo) {
		auto pred = [=](ShaderModuleCreateInfo const& a){
			return a.stage == createInfo.stage;
		};

		if (auto place = std::find_if(this->shaderModuleCIs.begin(), this->shaderModuleCIs.end(), pred); place != this->shaderModuleCIs.end()) {
			*place = createInfo;
		} else {
			this->shaderModuleCIs.push_back(createInfo);
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
	
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::overwriteSet(u32 set, BindingSetDescription const& descr) {
		setDescriptionOverwrites.push_back({set, descr});
		return *this;
	}
}