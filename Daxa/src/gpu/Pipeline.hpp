#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <unordered_map>
#include <vector>
#include <array>
#include <string>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Handle.hpp"
#include "ShaderModule.hpp"
#include "BindingSet.hpp"

namespace daxa {
	namespace gpu {

		constexpr inline size_t MAX_SETS_PER_PIPELINE = 4;

		struct ComputePipelineCreateInfo {
			ShaderModuleHandle 	shaderModule 	= {};
			char const* 		debugName 		= {};
		};

		class PipelineHandle;

		class Pipeline {
		public:
			Pipeline() 											= default;
			Pipeline(Pipeline&&) noexcept						= delete;
			Pipeline& operator=(Pipeline&&) noexcept			= delete;
			Pipeline(Pipeline const&)							= delete;
			Pipeline const& operator=(Pipeline const&) noexcept	= delete;
			~Pipeline();

			BindingSetDescription const* getSetDescription(u32 set) const { 
				auto setDescr = bindingSetDescriptions.at(set);
				DAXA_ASSERT_M(setDescr, "tried querring non existant binding set description from pipeline");
				return setDescr;
			}

			VkPipelineBindPoint getVkBindPoint() const { return bindPoint; }
			VkPipeline const& getVkPipeline() const { return pipeline; }
			VkPipelineLayout const& getVkPipelineLayout() const { return layout; }

			std::vector<VkFormat> const& getColorAttachemntFormats() const { return colorAttachmentFormats; }
			VkFormat getDepthAttachmentFormat() const { return depthAttachment; }
			VkFormat getStencilAttachmentFormat() const { return stencilAttachment; }

			std::string const& getDebugName() const { return debugName; }
		private:
			friend class GraphicsPipelineBuilder;
			friend PipelineHandle createComputePipeline(VkDevice device, BindingSetDescriptionCache& bindingSetCache, ComputePipelineCreateInfo const& ci);
			friend void setPipelineDebugName(VkDevice device, char const* debugName, Pipeline& pipeline);

			std::array<BindingSetDescription const*, MAX_SETS_PER_PIPELINE> bindingSetDescriptions = {};
			std::vector<VkFormat> colorAttachmentFormats = {};
			VkFormat depthAttachment = VK_FORMAT_UNDEFINED;
			VkFormat stencilAttachment = VK_FORMAT_UNDEFINED;

			VkPipelineBindPoint bindPoint 	= {};
			VkDevice device					= {};
			VkPipeline pipeline				= {};
			VkPipelineLayout layout			= {};
			std::string debugName 			= {};
		};

		class PipelineHandle : public SharedHandle<Pipeline>{};

		struct DepthTestSettings {
			VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;
			bool enableDepthTest = false;
			bool enableDepthWrite = false;
			VkCompareOp depthTestCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			f32 minDepthBounds = 0.0f;
			f32 maxDepthBounds = 1.0f;
		};

		struct RasterSettings {
			VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
			VkCullModeFlagBits cullMode = VK_CULL_MODE_NONE;
			VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
			bool depthClampEnable = false;
			bool rasterizerDiscardEnable = false;
			bool depthBiasEnable = false;
			float depthBiasConstantFactor = 0.0f;
			float depthBiasClamp = 0.0f;
			float depthBiasSlopeFactor = 0.0f;
			f32 lineWidth = 1.0f;
		};

		class GraphicsPipelineBuilder {
		public:
			GraphicsPipelineBuilder() = default;
			GraphicsPipelineBuilder& setDebugName(char const* name);
			GraphicsPipelineBuilder& beginVertexInputAttributeBinding(VkVertexInputRate rate);
			GraphicsPipelineBuilder& addVertexInputAttribute(VkFormat format);
			GraphicsPipelineBuilder& addVertexInputAttributeSpacer(u32 spacing);
			GraphicsPipelineBuilder& endVertexInputAttributeBinding();
			GraphicsPipelineBuilder& configurateDepthTest(DepthTestSettings const& depthTestSettings2);
			GraphicsPipelineBuilder& setInputAssembly(VkPipelineInputAssemblyStateCreateInfo const& inputassembly);
			GraphicsPipelineBuilder& setRasterization(RasterSettings const& rasterization);
			GraphicsPipelineBuilder& setMultisampling(VkPipelineMultisampleStateCreateInfo const& multisampling);
			GraphicsPipelineBuilder& addShaderStage(ShaderModuleHandle const& shaderModule);
			GraphicsPipelineBuilder& addColorAttachment(VkFormat const& attachmentFormat);
			GraphicsPipelineBuilder& addColorAttachment(VkFormat const& attachmentFormat, const VkPipelineColorBlendAttachmentState&);
		private:
			friend class Device;
			PipelineHandle build(VkDevice, BindingSetDescriptionCache& bindingSetCache);

			char const* debugName = {};

			// Vertex Input Attribute building:
			bool bVertexAtrributeBindingBuildingOpen = false;
			VkVertexInputRate currentVertexAttributeBindingInputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			u32 currentVertexAttributeBindingOffset = 0;
			u32 currentVertexAttributeLocation = 0;
			std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
			std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;

			VkPipelineCache pipelineCache;
			DepthTestSettings depthTestSettings = {};
			RasterSettings rasterSettings = {};
			std::optional<VkPipelineInputAssemblyStateCreateInfo> inputAssembly;
			std::optional<VkPipelineMultisampleStateCreateInfo> multisampling;
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
			std::vector<VkDynamicState> dynamicStateEnable;
			std::vector<VkPushConstantRange> pushConstants;
			std::vector<std::unordered_map<u32, VkDescriptorSetLayoutBinding>> descriptorSets;
			std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo;
			std::vector<VkPipelineColorBlendAttachmentState> colorAttachmentBlends;
			std::vector<VkFormat> colorAttachmentFormats;
		};

		PipelineHandle createComputePipeline(VkDevice device, BindingSetDescriptionCache& bindingSetCache, ComputePipelineCreateInfo const& ci);
	}
}