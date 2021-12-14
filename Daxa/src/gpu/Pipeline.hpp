#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <unordered_map>
#include <vector>
#include <array>
#include <string>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "ShaderModule.hpp"
#include "BindingSet.hpp"

namespace daxa {
	namespace gpu {

		constexpr inline size_t MAX_SETS_PER_PIPELINE = 4;

		class GraphicsPipeline {
		public:
			GraphicsPipeline() 													= default;
			GraphicsPipeline(GraphicsPipeline&&) noexcept						= delete;
			GraphicsPipeline& operator=(GraphicsPipeline&&) noexcept			= delete;
			GraphicsPipeline(GraphicsPipeline const&)							= delete;
			GraphicsPipeline const& operator=(GraphicsPipeline const&) noexcept	= delete;
			~GraphicsPipeline();

			BindingSetDescription const* getSetDescription(u32 set) const { 
				auto setDescr = bindingSetDescriptions.at(set);
				DAXA_ASSERT_M(setDescr, "tried querring non existant binding set description from pipeline");
				return setDescr;
			}

			VkPipeline const& getVkPipeline() const { return pipeline; }
			VkPipelineLayout const& getVkPipelineLayout() const { return layout; }
		private:
			friend class GraphicsPipelineBuilder;

			std::array<BindingSetDescription const*, MAX_SETS_PER_PIPELINE> bindingSetDescriptions = {};

			VkDevice device;
			VkPipeline pipeline;
			VkPipelineLayout layout;
		};

		class GraphicsPipelineHandle {
		public:
			GraphicsPipelineHandle(std::shared_ptr<GraphicsPipeline> pipeline) 
				: pipeline{ std::move(pipeline) }
			{}
			GraphicsPipelineHandle() = default;

			GraphicsPipeline const& operator*() const { return *pipeline; }
			GraphicsPipeline& operator*() { return *pipeline; }
			GraphicsPipeline const* operator->() const { return pipeline.get(); }
			GraphicsPipeline* operator->() { return pipeline.get(); }

			size_t getRefCount() const { return pipeline.use_count(); }

			operator bool() const { return pipeline.operator bool(); }
			bool operator!() const { return !pipeline; }
			bool valid() const { return *this; }
		private:
			std::shared_ptr<GraphicsPipeline> pipeline = {};
		};

		struct DepthTestSettings {
			VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;
			bool enableDepthTest = false;
			bool enableDepthWrite = false;
			VkCompareOp depthTestCompareOp = VK_COMPARE_OP_LESS;
			f32 minDepthBounds = 0.0f;
			f32 maxDepthBounds = 1.0f;
		};

		class GraphicsPipelineBuilder {
		public:
			GraphicsPipelineBuilder() = default;
			GraphicsPipelineBuilder& beginVertexInputAttributeBinding(VkVertexInputRate rate);
			GraphicsPipelineBuilder& addVertexInputAttribute(VkFormat format);
			GraphicsPipelineBuilder& addVertexInputAttributeSpacer(u32 spacing);
			GraphicsPipelineBuilder& endVertexInputAttributeBinding();
			GraphicsPipelineBuilder& configurateDepthTest(DepthTestSettings depthTestSettings2);
			GraphicsPipelineBuilder& setInputAssembly(const VkPipelineInputAssemblyStateCreateInfo& inputassembly);
			GraphicsPipelineBuilder& setRasterization(const VkPipelineRasterizationStateCreateInfo& rasterization);
			GraphicsPipelineBuilder& setMultisampling(const VkPipelineMultisampleStateCreateInfo& multisampling);
			GraphicsPipelineBuilder& addShaderStage(const ShaderModuleHandle& shaderModule);
			GraphicsPipelineBuilder& addColorAttachment(const VkFormat& attachmentFormat);
			GraphicsPipelineBuilder& addColorAttachment(const VkFormat& attachmentFormat, const VkPipelineColorBlendAttachmentState&);
		private:
			friend class Device;
			GraphicsPipelineHandle build(VkDevice, BindingSetDescriptionCache& bindingSetCache);

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
			DepthTestSettings depthTestSettings = {};
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
			std::vector<VkDynamicState> dynamicStateEnable;
			std::vector<VkPushConstantRange> pushConstants;
			std::unordered_map<u32, std::unordered_map<u32, VkDescriptorSetLayoutBinding>> descriptorSets;
			std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo;
			std::vector<VkPipelineColorBlendAttachmentState> colorAttachmentBlends;
			std::vector<VkFormat> colorAttachmentFormats;
		};
	}
}