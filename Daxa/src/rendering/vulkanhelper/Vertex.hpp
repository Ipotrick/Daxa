#pragma once

#include <vector>

#include "../Vulkan.hpp"

namespace daxa {
	namespace vkh {
		VkBufferCreateInfo makeVertexBufferCreateInfo(uz size);

		struct VertexDescription {
			std::vector<VkVertexInputBindingDescription> bindings;
			std::vector<VkVertexInputAttributeDescription> attributes;

			VkPipelineVertexInputStateCreateFlags flags = 0;
		};

		class VertexDiscriptionBuilder {
		public:
			VertexDiscriptionBuilder& beginBinding(u32 stride, VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX);
			VertexDiscriptionBuilder& setAttribute(VkFormat format);
			VertexDiscriptionBuilder& stageCreateFlags(VkPipelineVertexInputStateCreateFlags flags);
			VertexDescription build();
		private:
			u32 stride{ 0 };
			u32 offset{ 0 };
			u32 location{ 0 };
			VkPipelineVertexInputStateCreateFlags flags = 0;
			std::vector<VkVertexInputBindingDescription> bindings;
			std::vector<VkVertexInputAttributeDescription> attributes;
		};
	}
}
