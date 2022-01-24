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
#include "DeviceBackend.hpp"
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

			std::shared_ptr<BindingSetInfo const> getSetInfo(u32 set) const { 
				auto setInfo = bindingSetDescriptions.at(set);
				DAXA_ASSERT_M(setInfo, "tried querring non existant binding set description from pipeline");
				return std::move(setInfo);
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
			friend daxa::Result<PipelineHandle> createComputePipeline(std::shared_ptr<DeviceBackend>& deviceBackend, BindingSetDescriptionCache& bindingSetCache, ComputePipelineCreateInfo const& ci);
			friend void setPipelineDebugName(VkDevice device, char const* debugName, Pipeline& pipeline);

			std::shared_ptr<DeviceBackend>												deviceBackend			= {};
			std::array<std::shared_ptr<BindingSetInfo const>, MAX_SETS_PER_PIPELINE> 	bindingSetDescriptions 	= {};
			std::vector<VkFormat> 														colorAttachmentFormats 	= {};
			VkFormat 																	depthAttachment 		= VK_FORMAT_UNDEFINED;
			VkFormat 																	stencilAttachment 		= VK_FORMAT_UNDEFINED;
			VkPipelineBindPoint 														bindPoint 				= {};
			VkPipeline 																	pipeline				= {};
			VkPipelineLayout 															layout					= {};
			std::string 																debugName 				= {};
		};

		class PipelineHandle : public SharedHandle<Pipeline>{};

		struct DepthTestSettings {
			VkFormat 	depthAttachmentFormat 	= VK_FORMAT_UNDEFINED;
			bool 		enableDepthTest 		= false;
			bool 		enableDepthWrite 		= false;
			VkCompareOp depthTestCompareOp 		= VK_COMPARE_OP_LESS_OR_EQUAL;
			f32 		minDepthBounds 			= 0.0f;
			f32 		maxDepthBounds 			= 1.0f;
		};

		struct RasterSettings {
			VkPolygonMode 		polygonMode 			= VK_POLYGON_MODE_FILL;
			VkCullModeFlagBits 	cullMode 				= VK_CULL_MODE_NONE;
			VkFrontFace 		frontFace 				= VK_FRONT_FACE_CLOCKWISE;
			bool 				depthClampEnable 		= false;
			bool 				rasterizerDiscardEnable = false;
			bool 				depthBiasEnable 		= false;
			float 				depthBiasConstantFactor = 0.0f;
			float 				depthBiasClamp 			= 0.0f;
			float 				depthBiasSlopeFactor 	= 0.0f;
			f32 				lineWidth 				= 1.0f;
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
			daxa::Result<PipelineHandle> build(std::shared_ptr<DeviceBackend>& deviceBackend, BindingSetDescriptionCache& bindingSetCache);

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
			std::vector<VkDynamicState> dynamicStateEnable;
			std::vector<ShaderModuleHandle> shaderModules;
			std::vector<VkPipelineColorBlendAttachmentState> colorAttachmentBlends;
			std::vector<VkFormat> colorAttachmentFormats;

			// temporaries:
			std::vector<VkPushConstantRange> pushConstants;
			std::vector<std::unordered_map<u32, VkDescriptorSetLayoutBinding>> descriptorSets;
			std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo;
		};

		daxa::Result<PipelineHandle> createComputePipeline(std::shared_ptr<DeviceBackend>& deviceBackend, BindingSetDescriptionCache& bindingSetCache, ComputePipelineCreateInfo const& ci);
	}
}