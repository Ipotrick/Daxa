#pragma once

#include <optional>

#include "../Vulkan.hpp"

#include "Vertex.hpp"

namespace daxa {
	namespace vkh {

		VkPipelineVertexInputStateCreateInfo makeVertexInputStageCreateInfo();

		VkPipelineInputAssemblyStateCreateInfo makeInputAssemblyStateCreateInfo(VkPrimitiveTopology topology);

		VkPipelineRasterizationStateCreateInfo makeRasterisationStateCreateInfo(VkPolygonMode polygonMode);

		VkPipelineMultisampleStateCreateInfo makeMultisampleStateCreateInfo();

		VkPipelineColorBlendAttachmentState makeColorBlendSAttachmentState();

		VkPipelineLayoutCreateInfo makeLayoutCreateInfo();

		class PipelineBuilder {
		public:
			std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
			VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
			VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
			VkViewport _viewport;
			VkRect2D _scissor;
			VkPipelineRasterizationStateCreateInfo _rasterizer;
			VkPipelineColorBlendAttachmentState _colorBlendAttachment;
			VkPipelineMultisampleStateCreateInfo _multisampling;
			VkPipelineLayout _pipelineLayout;

			VkPipeline build(VkRenderPass pass, VkDevice device = vkh::mainDevice);
		};

		class BetterPipelineBuilder {
		public:
			std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
			std::optional<VkPipelineVertexInputStateCreateInfo> vertexInputInfo;
			std::optional<VkPipelineInputAssemblyStateCreateInfo> inputAssembly;
			std::optional<VkViewport> viewport;
			std::optional<VkRect2D> scissor;
			std::optional<VkPipelineRasterizationStateCreateInfo> rasterizer;
			std::optional<VkPipelineColorBlendAttachmentState> colorBlendAttachment;
			std::optional<VkPipelineMultisampleStateCreateInfo> multisampling;
			std::optional<VkPipelineLayout> pipelineLayout;

			void setVertexInfo(const VertexDescription& vd);

			VkPipeline build(VkRenderPass pass, VkDevice device = vkh::mainDevice);
		};
	}
}
