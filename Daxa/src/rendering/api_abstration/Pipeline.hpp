#pragma once

#include <memory>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

namespace gpu {

	class PipelineBuilder {
	public:
		PipelineBuilder() = default;
		PipelineBuilder& setViewport(vk::Viewport viewport);
		PipelineBuilder& setScissor(vk::Rect2D scissor);
		PipelineBuilder& setVertexInput(const vk::PipelineVertexInputStateCreateInfo& vertexInput);
		PipelineBuilder& setInputAssembly(const vk::PipelineInputAssemblyStateCreateInfo& inputassembly);
		PipelineBuilder& setRasterization(const vk::PipelineRasterizationStateCreateInfo& rasterization);
		PipelineBuilder& setMultisampling(const vk::PipelineMultisampleStateCreateInfo& multisampling);
		PipelineBuilder& setDepthStencil(const vk::PipelineDepthStencilStateCreateInfo& depthStencil);
		PipelineBuilder& setColorBlend(const vk::PipelineColorBlendStateCreateInfo& colorBlend);
		PipelineBuilder& addColorBlendAttachemnt(const vk::PipelineColorBlendAttachmentState& colorAttachmentBlend);
		PipelineBuilder& addShaderStage(
			const std::vector<uint32_t>* spv,
			vk::ShaderStageFlagBits shaderStage,
			vk::PipelineShaderStageCreateFlags flags = {},
			vk::SpecializationInfo* pSpecializationInfo = {});
	private:

	};

	class Pipeline {
	public:

	private:
		vkh::Pipeline pipeline;
	};

}
