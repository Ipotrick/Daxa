#pragma once

#include "../../DaxaCore.hpp"

#include <memory>
#include <unordered_map>
#include <vector>
#include <array>
#include <string>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <../dependencies/vulkanhelper.hpp>
#include "../dependencies/vk_mem_alloc.hpp"

#include "ShaderModule.hpp"
#include "DescriptorSetLayoutCache.hpp"

namespace daxa {
	namespace gpu {

		class GraphicsPipeline {
		public:
			GraphicsPipeline() = default;
			GraphicsPipeline(GraphicsPipeline&&) noexcept;
			GraphicsPipeline& operator=(GraphicsPipeline&&) noexcept;
			~GraphicsPipeline();

			VkPipeline const& getVkPipeline() const { return pipeline; }
			VkPipelineLayout const& getVkPipelineLayout() const { return layout; }
		private:
			friend class GraphicsPipelineBuilder;
			VkDevice device;
			VkPipeline pipeline;
			VkPipelineLayout layout;
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
			GraphicsPipelineBuilder& beginVertexInputAttributeBinding(VkVertexInputRate rate);
			GraphicsPipelineBuilder& addVertexInputAttribute(VkFormat format);
			GraphicsPipelineBuilder& addVertexInputAttributeSpacer(u32 spacing);
			GraphicsPipelineBuilder& endVertexInputAttributeBinding();
			GraphicsPipelineBuilder& setInputAssembly(const VkPipelineInputAssemblyStateCreateInfo& inputassembly);
			GraphicsPipelineBuilder& setRasterization(const VkPipelineRasterizationStateCreateInfo& rasterization);
			GraphicsPipelineBuilder& setMultisampling(const VkPipelineMultisampleStateCreateInfo& multisampling);
			GraphicsPipelineBuilder& setDepthStencil(const VkPipelineDepthStencilStateCreateInfo& depthStencil);
			GraphicsPipelineBuilder& addShaderStage(const ShaderModuleHandle& shaderModule);
			GraphicsPipelineBuilder& addColorAttachment(const VkFormat& attachmentFormat);
			GraphicsPipelineBuilder& addColorAttachment(const VkFormat& attachmentFormat, const VkPipelineColorBlendAttachmentState&);
			GraphicsPipelineBuilder& addDepthAttachment(const VkFormat& attachmentFormat);
			GraphicsPipelineBuilder& addStencilAttachment(const VkFormat& attachmentFormat);
		private:
			friend class Device;
			GraphicsPipelineHandle build(VkDevice, DescriptorSetLayoutCache& layoutCache);

			// Vertex Input Attribute building:
			bool bVertexAtrributeBindingBuildingOpen = false;
			VkVertexInputRate currentVertexAttributeBindingInputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			u32 currentVertexAttributeBindingOffset = 0;
			u32 currentVertexAttributeLocation = 0;
			std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
			std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;

			VkPipelineCache pipelineCache;
			std::optional<VkPipelineInputAssemblyStateCreateInfo> inputAssembly;
			std::optional<VkPipelineRasterizationStateCreateInfo> rasterization;
			std::optional<VkPipelineMultisampleStateCreateInfo> multisampling;
			std::optional<VkPipelineDepthStencilStateCreateInfo> depthStencil;
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
			std::vector<VkDynamicState> dynamicStateEnable;
			std::vector<VkPushConstantRange> pushConstants;
			std::unordered_map<u32, std::unordered_map<u32, VkDescriptorSetLayoutBinding>> descriptorSets;
			std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo;
			std::vector<VkPipelineColorBlendAttachmentState> colorAttachmentBlends;
			std::vector<VkFormat> colorAttachmentFormats;
			std::optional<VkFormat> depthAttachmentFormat;
			std::optional<VkFormat> stencilAttachmentFormat;
		};
	}
}