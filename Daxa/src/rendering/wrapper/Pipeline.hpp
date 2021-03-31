#pragma once

#include "../Vulkan.hpp"

namespace daxa {
	namespace vk {

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

			VkPipeline build(VkRenderPass pass, VkDevice device = vk::mainDevice);
		};

		class Pipeline {
		public:
		private:
		};
	}
}
