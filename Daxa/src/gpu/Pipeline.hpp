#pragma once

#include "../DaxaCore.hpp"

#include <set>
#include <memory>
#include <unordered_map>
#include <map>
#include <vector>
#include <array>
#include <string>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Handle.hpp"
#include "ShaderModule.hpp"
#include "BindingSet.hpp"

namespace daxa {
	class PipelineCompiler;
	constexpr VkPipelineRasterizationStateCreateInfo DEFAULT_RASTER_STATE_CI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL,
		.cullMode = VkCullModeFlagBits::VK_CULL_MODE_NONE,
		.frontFace = VkFrontFace::VK_FRONT_FACE_CLOCKWISE,
		.lineWidth = 1.0f,
	};

	constexpr VkPipelineInputAssemblyStateCreateInfo DEFAULT_INPUT_ASSEMBLY_STATE_CI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	constexpr VkPipelineMultisampleStateCreateInfo DEFAULT_MULTISAMPLE_STATE_CI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = nullptr,
		.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
		.minSampleShading = 1.0f,
	};

	constexpr VkViewport DEFAULT_VIEWPORT{
		.width = 1,
		.height = 1
	};

	constexpr VkRect2D DEFAULT_SCISSOR{
		.extent = {1,1},
	};

	constexpr VkPipelineVertexInputStateCreateInfo DEFAULT_VERTEX_INPUT_STATE_CI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr,
	};

	constexpr inline size_t MAX_SETS_PER_PIPELINE = 4;

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

	struct ComputePipelineCreateInfo {
		ShaderModuleCreateInfo 													shaderCI 		= {};
		std::array<std::optional<BindingSetDescription>, MAX_SETS_PER_PIPELINE> overwriteSets 	= {};
		size_t 																	pushConstantSize = 0;
		char const* 															debugName 		= {};
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
		GraphicsPipelineBuilder& addShaderStage(ShaderModuleCreateInfo const& ci);
		GraphicsPipelineBuilder& addColorAttachment(VkFormat const& attachmentFormat);
		GraphicsPipelineBuilder& addColorAttachment(VkFormat const& attachmentFormat, const VkPipelineColorBlendAttachmentState&);
		GraphicsPipelineBuilder& overwriteSet(u32 set, BindingSetDescription const& descr);
	private:
		friend class Device;
		friend class daxa::PipelineCompiler;

		std::string debugName = {};

		// Vertex Input Attribute building:
		bool bVertexAtrributeBindingBuildingOpen = false;
		VkVertexInputRate currentVertexAttributeBindingInputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		u32 currentVertexAttributeBindingOffset = 0;
		u32 currentVertexAttributeLocation = 0;
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;

		DepthTestSettings depthTestSettings = {};
		RasterSettings rasterSettings = {};
		std::optional<VkPipelineInputAssemblyStateCreateInfo> inputAssembly;
		std::optional<VkPipelineMultisampleStateCreateInfo> multisampling;
		std::vector<VkDynamicState> dynamicStateEnable;
		std::vector<ShaderModuleCreateInfo> shaderModuleCIs;
		std::vector<VkPipelineColorBlendAttachmentState> colorAttachmentBlends;
		std::vector<VkFormat> colorAttachmentFormats;
		std::vector<std::pair<u32, BindingSetDescription>> setDescriptionOverwrites;
	};

	//std::size_t sizeofFormat(VkFormat format);

	class Pipeline {
	public:
		Pipeline() 											= default;
		Pipeline(Pipeline&&) noexcept						= delete;
		Pipeline& operator=(Pipeline&&) noexcept			= delete;
		Pipeline(Pipeline const&)							= delete;
		Pipeline const& operator=(Pipeline const&) noexcept	= delete;
		~Pipeline();

		std::shared_ptr<BindingSetLayout const> getSetLayout(u32 set) const { 
			auto setLayout = bindingSetLayouts.at(set);
			DAXA_ASSERT_M(setLayout, "tried querring non existant binding set description from pipeline");
			return setLayout;
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
		friend class PipelineCompiler;
		void setPipelineDebugName(VkDevice device, char const* debugName);

		std::shared_ptr<DeviceBackend>														deviceBackend			= {};
		std::array<std::shared_ptr<BindingSetLayout const>, MAX_SETS_PER_PIPELINE> 			bindingSetLayouts 		= {};
		std::vector<VkFormat> 																colorAttachmentFormats 	= {};
		VkFormat 																			depthAttachment 		= VK_FORMAT_UNDEFINED;
		VkFormat 																			stencilAttachment 		= VK_FORMAT_UNDEFINED;
		VkPipelineBindPoint 																bindPoint 				= {};
		VkPipeline 																			pipeline				= {};
		VkPipelineLayout 																	layout					= {};
		std::string 																		debugName 				= {};
		std::variant<GraphicsPipelineBuilder, ComputePipelineCreateInfo, std::monostate> 	creator 				= {};
		std::set<std::pair<std::filesystem::path, std::chrono::file_clock::time_point>>		observedHotLoadFiles	= {};
		std::chrono::time_point<std::chrono::file_clock>									lastRecreationCheckTimePoint 		= {};
	};

	class PipelineHandle : public SharedHandle<Pipeline>{};
}