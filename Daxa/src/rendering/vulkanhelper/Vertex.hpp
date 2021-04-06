#pragma once

#include <vector>

#include "../Vulkan.hpp"

namespace daxa {
	namespace vkh {

		struct VertexDescription {
			std::vector<vk::VertexInputBindingDescription> bindings;
			std::vector<vk::VertexInputAttributeDescription> attributes;

			vk::PipelineVertexInputStateCreateFlags flags{};

			vk::PipelineVertexInputStateCreateInfo makePipelineVertexInpitSCI() const
			{
				return vk::PipelineVertexInputStateCreateInfo{
					.vertexBindingDescriptionCount = (u32)bindings.size(),
					.pVertexBindingDescriptions = bindings.data(),
					.vertexAttributeDescriptionCount = (u32)attributes.size(),
					.pVertexAttributeDescriptions = attributes.data(),
				};
			}
		};

		class VertexDiscriptionBuilder {
		public:
			VertexDiscriptionBuilder& beginBinding(u32 stride, vk::VertexInputRate inputRate = vk::VertexInputRate::eVertex);
			VertexDiscriptionBuilder& setAttribute(vk::Format format);
			VertexDiscriptionBuilder& stageCreateFlags(vk::PipelineVertexInputStateCreateFlags flags);
			VertexDescription build();
		private:
			u32 stride{ 0 };
			u32 offset{ 0 };
			u32 location{ 0 };
			vk::PipelineVertexInputStateCreateFlags flags{};
			std::vector<vk::VertexInputBindingDescription> bindings;
			std::vector<vk::VertexInputAttributeDescription> attributes;
		};
	}
}
