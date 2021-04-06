#pragma once

#include <optional>

#include "../Vulkan.hpp"

#include "Vertex.hpp"
#include "Initialization.hpp"

namespace daxa {
	namespace vkh {

		vk::PipelineVertexInputStateCreateInfo makeVertexInputStageCreateInfo();

		vk::PipelineInputAssemblyStateCreateInfo makeInputAssemblyStateCreateInfo(vk::PrimitiveTopology topology);

		vk::PipelineRasterizationStateCreateInfo makeRasterisationStateCreateInfo(vk::PolygonMode polygonMode);

		vk::PipelineMultisampleStateCreateInfo makeMultisampleStateCreateInfo();

		vk::PipelineColorBlendAttachmentState makeColorBlendSAttachmentState();

		vk::PipelineLayoutCreateInfo makePipelineLayoutCreateInfo();

		struct Pipeline {
			vk::UniquePipeline pipeline;
			vk::UniquePipelineLayout layout;
		};

		class PipelineBuilder {
		public:
			std::optional<vk::Viewport> viewport;
			std::optional<vk::Rect2D> scissor;
			std::optional<vk::PipelineVertexInputStateCreateInfo> vertexInput;
			std::optional<vk::PipelineInputAssemblyStateCreateInfo> inputAssembly;
			std::optional<vk::PipelineRasterizationStateCreateInfo> rasterization;
			std::optional<vk::PipelineMultisampleStateCreateInfo> multisampling;
			std::optional<vk::PipelineDepthStencilStateCreateInfo> depthStencil;
			std::optional<vk::PipelineColorBlendAttachmentState> colorBlendAttachment;
			std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
			std::vector<vk::DynamicState> dynamicStateEnable;
			std::vector<vk::PushConstantRange> pushConstants;

			Pipeline build(vk::RenderPass pass, u32 subpass = 0, vk::Device device = vkh::device);
		};
	}
}
