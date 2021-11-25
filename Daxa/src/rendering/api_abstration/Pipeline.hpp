#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <array>
#include <string>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

#include "ShaderModule.hpp"

namespace daxa {
	namespace gpu {

		class GraphicsPipeline {
		public:
			vk::Pipeline const& getVkPipeline() const { return *pipeline; }
			vk::PipelineLayout const& getVkPipelineLayout() const { return *layout; }
		private:
			friend class GraphicsPipelineBuilder;
			vk::UniquePipeline pipeline;
			vk::UniquePipelineLayout layout;
		};

		class GraphicsPipelineHandle {
		public:
			GraphicsPipeline const& operator * () const { return *pipeline; }
			GraphicsPipeline const* operator -> () const { return &*pipeline; }
		private:
			friend class GraphicsPipelineBuilder;
			std::shared_ptr<GraphicsPipeline> pipeline;
		};

		class GraphicsPipelineBuilder {
		public:
			GraphicsPipelineBuilder() = default;
			GraphicsPipelineBuilder& setViewport(vk::Viewport viewport);
			GraphicsPipelineBuilder& setScissor(vk::Rect2D scissor);
			GraphicsPipelineBuilder& setVertexInput(const vk::PipelineVertexInputStateCreateInfo& vertexInput);
			GraphicsPipelineBuilder& setInputAssembly(const vk::PipelineInputAssemblyStateCreateInfo& inputassembly);
			GraphicsPipelineBuilder& setRasterization(const vk::PipelineRasterizationStateCreateInfo& rasterization);
			GraphicsPipelineBuilder& setMultisampling(const vk::PipelineMultisampleStateCreateInfo& multisampling);
			GraphicsPipelineBuilder& setDepthStencil(const vk::PipelineDepthStencilStateCreateInfo& depthStencil);
			GraphicsPipelineBuilder& addShaderStage(const ShaderModuleHandle& shaderModule);
			GraphicsPipelineBuilder& addColorAttachment(const vk::Format& attachmentFormat);
			GraphicsPipelineBuilder& addColorAttachment(const vk::Format& attachmentFormat, const vk::PipelineColorBlendAttachmentState&);
			GraphicsPipelineBuilder& addDepthAttachment(const vk::Format& attachmentFormat);
			GraphicsPipelineBuilder& addStencilAttachment(const vk::Format& attachmentFormat);
		private:
			friend class Device;
			GraphicsPipelineHandle build(vk::Device, vkh::DescriptorSetLayoutCache& layoutCache) const;

			vk::PipelineCache pipelineCache;
			std::optional<vk::Viewport> viewport;
			std::optional<vk::Rect2D> scissor;
			std::optional<vk::PipelineVertexInputStateCreateInfo> vertexInput;
			std::optional<vk::PipelineInputAssemblyStateCreateInfo> inputAssembly;
			std::optional<vk::PipelineRasterizationStateCreateInfo> rasterization;
			std::optional<vk::PipelineMultisampleStateCreateInfo> multisampling;
			std::optional<vk::PipelineDepthStencilStateCreateInfo> depthStencil;
			std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
			std::vector<vk::DynamicState> dynamicStateEnable;
			std::vector<vk::PushConstantRange> pushConstants;
			std::unordered_map<u32, std::unordered_map<u32, vk::DescriptorSetLayoutBinding>> descriptorSets;
			std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfo;
			std::vector<vk::PipelineColorBlendAttachmentState> colorAttachmentBlends;
			std::vector<vk::Format> colorAttachmentFormats;
			std::optional<vk::Format> depthAttachmentFormat;
			std::optional<vk::Format> stencilAttachmentFormat;
		};

	}
}