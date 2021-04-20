#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <filesystem>
#include <functional>
#include <set>
#include <unordered_map>

#if defined(VULKANHELPER_IMPLEMENTATION)
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <iostream>
#include <fstream>
#endif

#if defined(_WIN32) || defined(_WIN64)
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#undef max
#undef min
#else
#error "CURRENTLY UNSUPPORTED PLATFORM"
#endif

namespace vkh {
	struct VertexDescription {
		std::vector<vk::VertexInputBindingDescription> bindings;
		std::vector<vk::VertexInputAttributeDescription> attributes;
		vk::PipelineVertexInputStateCreateFlags flags{};

		vk::PipelineVertexInputStateCreateInfo makePipelineVertexInputStateCreateInfo() const {
			return vk::PipelineVertexInputStateCreateInfo{
				.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size()),
				.pVertexBindingDescriptions = bindings.data(),
				.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size()),
				.pVertexAttributeDescriptions = attributes.data(),
			};
		}
	};

	class VertexDiscriptionBuilder {
	public:
		VertexDiscriptionBuilder& beginBinding(uint32_t stride, vk::VertexInputRate inputRate = vk::VertexInputRate::eVertex);
		VertexDiscriptionBuilder& addAttribute(vk::Format format);
		VertexDiscriptionBuilder& stageCreateFlags(vk::PipelineVertexInputStateCreateFlags flags);
		VertexDescription build();

	private:
		uint32_t stride{ 0 };
		uint32_t offset{ 0 };
		uint32_t location{ 0 };
		vk::PipelineVertexInputStateCreateFlags flags{};
		std::vector<vk::VertexInputBindingDescription> bindings;
		std::vector<vk::VertexInputAttributeDescription> attributes;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
	VertexDiscriptionBuilder& VertexDiscriptionBuilder::beginBinding(uint32_t stride, vk::VertexInputRate inputRate) {
		offset = 0;
		location = 0;
		vk::VertexInputBindingDescription binding{
			.binding = static_cast<uint32_t>(bindings.size()),
			.stride = stride,
			.inputRate = inputRate,
		};
		bindings.push_back(binding);
		return *this;
	}

	VertexDiscriptionBuilder& VertexDiscriptionBuilder::addAttribute(vk::Format format) {
		vk::VertexInputAttributeDescription attribute{
			.location = location,
			.binding = static_cast<uint32_t>(bindings.size() - 1),
			.format = format,
			.offset = offset,
		};

		attributes.push_back(attribute);

		location += 1;

		switch (format) {
		case vk::Format::eR32G32B32A32Sfloat:
		case vk::Format::eR32G32B32A32Sint:
		case vk::Format::eR32G32B32A32Uint:
			offset += sizeof(uint32_t) * 4;
			break;
		case vk::Format::eR32G32B32Sfloat:
		case vk::Format::eR32G32B32Sint:
		case vk::Format::eR32G32B32Uint:
			offset += sizeof(uint32_t) * 3;
			break;
		case vk::Format::eR32G32Sfloat:
		case vk::Format::eR32G32Sint:
		case vk::Format::eR32G32Uint:
			offset += sizeof(uint32_t) * 2;
			break;
		case vk::Format::eR32Sfloat:
		case vk::Format::eR32Sint:
		case vk::Format::eR32Uint:
			offset += sizeof(uint32_t) * 1;
			break;
		default:
			assert(false);
		}

		return *this;
	}

	VertexDiscriptionBuilder& VertexDiscriptionBuilder::stageCreateFlags(vk::PipelineVertexInputStateCreateFlags flags) {
		this->flags = flags;
		return *this;
	}

	VertexDescription VertexDiscriptionBuilder::build() {
		assert(bindings.size() > 0);
		return VertexDescription{
			bindings,
			attributes,
			flags };
	}
#endif

	vk::PipelineRasterizationStateCreateInfo makeDefaultRasterisationStateCreateInfo(vk::PolygonMode polygonMode);

	vk::PipelineMultisampleStateCreateInfo makeDefaultMultisampleStateCreateInfo();

	vk::PipelineColorBlendAttachmentState makeDefaultColorBlendSAttachmentState();

	struct Pipeline {
		vk::UniquePipeline pipeline;
		vk::UniquePipelineLayout layout;
	};
	class GraphicsPipelineBuilder {
	public:
		Pipeline build(vk::Device device, vk::RenderPass pass, uint32_t subpass = 0, vk::PipelineCache pipelineCache = nullptr);

		GraphicsPipelineBuilder& setViewport(const vk::Viewport& viewport);
		GraphicsPipelineBuilder& setScissor(const vk::Rect2D& scissor);
		GraphicsPipelineBuilder& setVertexInput(const vk::PipelineVertexInputStateCreateInfo& vertexInput);
		GraphicsPipelineBuilder& setInputAssembly(const vk::PipelineInputAssemblyStateCreateInfo& inputassembly);
		GraphicsPipelineBuilder& setRasterization(const vk::PipelineRasterizationStateCreateInfo& rasterization);
		GraphicsPipelineBuilder& setMultisampling(const vk::PipelineMultisampleStateCreateInfo& multisampling);
		GraphicsPipelineBuilder& setDepthStencil(const vk::PipelineDepthStencilStateCreateInfo& depthStencil);
		GraphicsPipelineBuilder& setColorBlend(const vk::PipelineColorBlendStateCreateInfo& colorBlend);
		GraphicsPipelineBuilder& addColorBlendAttachemnt(const vk::PipelineColorBlendAttachmentState& colorAttachmentBlend);
		GraphicsPipelineBuilder& addShaderStage(const vk::PipelineShaderStageCreateInfo& shaderStage);
		GraphicsPipelineBuilder& addDynamicState(const vk::DynamicState& dynamicstates);
		GraphicsPipelineBuilder& addPushConstants(const vk::PushConstantRange& pushconstants);
		GraphicsPipelineBuilder& addDescriptorLayout(const vk::DescriptorSetLayout& layout);

	private:
		std::optional<vk::Viewport> viewport;
		std::optional<vk::Rect2D> scissor;
		std::optional<vk::PipelineVertexInputStateCreateInfo> vertexInput;
		std::optional<vk::PipelineInputAssemblyStateCreateInfo> inputAssembly;
		std::optional<vk::PipelineRasterizationStateCreateInfo> rasterization;
		std::optional<vk::PipelineMultisampleStateCreateInfo> multisampling;
		std::optional<vk::PipelineDepthStencilStateCreateInfo> depthStencil;
		std::optional<vk::PipelineColorBlendStateCreateInfo> colorBlend;
		std::vector<vk::PipelineColorBlendAttachmentState> colorAttachmentBlends;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		std::vector<vk::DynamicState> dynamicStateEnable;
		std::vector<vk::PushConstantRange> pushConstants;
		std::vector<vk::DescriptorSetLayout> descLayouts;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setViewport(const vk::Viewport& viewport) {
		this->viewport = viewport;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setScissor(const vk::Rect2D& scissor) {
		this->scissor = scissor;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setVertexInput(const vk::PipelineVertexInputStateCreateInfo& vertexInput) {
		this->vertexInput = vertexInput;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setInputAssembly(const vk::PipelineInputAssemblyStateCreateInfo& inputAssembly) {
		this->inputAssembly = inputAssembly;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setRasterization(const vk::PipelineRasterizationStateCreateInfo& rasterization) {
		this->rasterization = rasterization;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setMultisampling(const vk::PipelineMultisampleStateCreateInfo& multisampling) {
		this->multisampling = multisampling;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setDepthStencil(const vk::PipelineDepthStencilStateCreateInfo& depthStencil) {
		this->depthStencil = depthStencil;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setColorBlend(const vk::PipelineColorBlendStateCreateInfo& colorBlend) {
		this->colorBlend = colorBlend;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addColorBlendAttachemnt(const vk::PipelineColorBlendAttachmentState& colorAttachmentBlend) {
		this->colorAttachmentBlends.push_back(colorAttachmentBlend);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addShaderStage(const vk::PipelineShaderStageCreateInfo& shaderStage) {
		this->shaderStages.push_back(shaderStage);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addDynamicState(const vk::DynamicState& dynamicstates) {
		this->dynamicStateEnable.push_back(dynamicstates);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addPushConstants(const vk::PushConstantRange& pushconstant) {
		this->pushConstants.push_back(pushconstant);
		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addDescriptorLayout(const vk::DescriptorSetLayout& layout) {
		this->descLayouts.push_back(layout);
		return *this;
	}

	vk::PipelineRasterizationStateCreateInfo makeDefaultRasterisationStateCreateInfo(vk::PolygonMode polygonMode) {
		return vk::PipelineRasterizationStateCreateInfo{
			.polygonMode = polygonMode,
			.cullMode = vk::CullModeFlagBits::eNone,
			.frontFace = vk::FrontFace::eClockwise,
			.lineWidth = 1.0f,
		};
	}

	vk::PipelineMultisampleStateCreateInfo makeDefaultMultisampleStateCreateInfo() {
		return vk::PipelineMultisampleStateCreateInfo{ .minSampleShading = 1.0f };
	}

	vk::PipelineColorBlendAttachmentState makeDefaultColorBlendSAttachmentState() {
		return vk::PipelineColorBlendAttachmentState{
			.blendEnable = VK_FALSE,
			.colorWriteMask =
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA,
		};
	}

	Pipeline GraphicsPipelineBuilder::build(vk::Device device, vk::RenderPass pass, uint32_t subpass, vk::PipelineCache pipelineCache) {
		if (!vertexInput) {
			std::cout << "error: vertexInput was not specified in pipeline builder!\n";
			exit(-1);
		}

		// set state create infos:
		vk::PipelineVertexInputStateCreateInfo pvertexInputCI = vertexInput.value();
		vk::PipelineInputAssemblyStateCreateInfo pinputAssemlyStateCI = inputAssembly.value_or(vk::PipelineInputAssemblyStateCreateInfo{ .topology = vk::PrimitiveTopology::eTriangleList });
		vk::PipelineRasterizationStateCreateInfo prasterizationStateCI = rasterization.value_or(vkh::makeDefaultRasterisationStateCreateInfo(vk::PolygonMode::eFill));
		vk::PipelineMultisampleStateCreateInfo multisamplerStateCI = multisampling.value_or(vkh::makeDefaultMultisampleStateCreateInfo());
		vk::PipelineDepthStencilStateCreateInfo pDepthStencilStateCI = depthStencil.value_or(vk::PipelineDepthStencilStateCreateInfo{});
		vk::Viewport pviewport = viewport.value_or(vk::Viewport{ .width = 1, .height = 1 });
		vk::Rect2D pscissor = scissor.value_or(vk::Rect2D{ .extent = {static_cast<uint32_t>(pviewport.width), static_cast<uint32_t>(pviewport.height)} });

		Pipeline pipeline;

		//build pipeline layout:
		vk::PipelineLayoutCreateInfo layoutCI{
			.setLayoutCount = static_cast<uint32_t>(descLayouts.size()),
			.pSetLayouts = descLayouts.data(),
			.pushConstantRangeCount = uint32_t(pushConstants.size()),
			.pPushConstantRanges = pushConstants.data(),
		};

		pipeline.layout = device.createPipelineLayoutUnique(layoutCI);

		vk::PipelineViewportStateCreateInfo viewportStateCI{
			.viewportCount = 1,
			.pViewports = &pviewport,
			.scissorCount = 1,
			.pScissors = &pscissor,
		};

		vk::PipelineColorBlendAttachmentState defaultColorBlendAttachmentStateCI = makeDefaultColorBlendSAttachmentState();
		vk::PipelineColorBlendStateCreateInfo colorBlendingSCI;
		if (this->colorBlend.has_value()) {
			colorBlendingSCI = this->colorBlend.value();
		}
		else if (this->colorAttachmentBlends.size() > 0) {
			colorBlendingSCI = vk::PipelineColorBlendStateCreateInfo{
				.logicOpEnable = VK_FALSE,
				.logicOp = vk::LogicOp::eCopy,
				.attachmentCount = static_cast<uint32_t>(this->colorAttachmentBlends.size()),
				.pAttachments = this->colorAttachmentBlends.data(),
			};
		}
		else {
			colorBlendingSCI = vk::PipelineColorBlendStateCreateInfo{
				.logicOpEnable = VK_FALSE,
				.logicOp = vk::LogicOp::eCopy,
				.attachmentCount = 1,
				.pAttachments = &defaultColorBlendAttachmentStateCI,
			};
		}

		// dynamic state setup:
		if (std::find(dynamicStateEnable.begin(), dynamicStateEnable.end(), vk::DynamicState::eViewport) == dynamicStateEnable.end()) {
			dynamicStateEnable.push_back(vk::DynamicState::eViewport);
		}
		if (std::find(dynamicStateEnable.begin(), dynamicStateEnable.end(), vk::DynamicState::eScissor) == dynamicStateEnable.end()) {
			dynamicStateEnable.push_back(vk::DynamicState::eScissor);
		}

		vk::PipelineDynamicStateCreateInfo dynamicStateCI{
			.dynamicStateCount = (uint32_t)dynamicStateEnable.size(),
			.pDynamicStates = dynamicStateEnable.data(),
		};

		//we now use all of the info structs we have been writing into into this one to create the pipeline
		vk::GraphicsPipelineCreateInfo pipelineCI{
			.stageCount = (uint32_t)shaderStages.size(),
			.pStages = shaderStages.data(),
			.pVertexInputState = &pvertexInputCI,
			.pInputAssemblyState = &pinputAssemlyStateCI,
			.pViewportState = &viewportStateCI,
			.pRasterizationState = &prasterizationStateCI,
			.pMultisampleState = &multisamplerStateCI,
			.pDepthStencilState = &pDepthStencilStateCI,
			.pColorBlendState = &colorBlendingSCI,
			.pDynamicState = &dynamicStateCI,
			.layout = pipeline.layout.get(),
			.renderPass = pass,
			.subpass = subpass,
		};

		auto ret = device.createGraphicsPipelineUnique(pipelineCache, pipelineCI);
		if (ret.result != vk::Result::eSuccess) {
			throw std::runtime_error("error: Failed to create graphics pipeline!");
		}
		pipeline.pipeline = std::move(ret.value);

		return pipeline;
	}
#endif

	std::optional<vk::UniqueShaderModule> loadShaderModule(vk::Device device, std::filesystem::path filePath);

	vk::PipelineShaderStageCreateInfo makeShaderStageCreateInfo(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule);

#if defined(VULKANHELPER_IMPLEMENTATION)
	std::optional<vk::UniqueShaderModule> loadShaderModule(vk::Device device, std::filesystem::path filePath) {
		std::ifstream file{ filePath, std::ios::ate | std::ios::binary };

		if (!file.is_open()) {
			return {};
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
		file.seekg(0);
		file.read((char*)buffer.data(), fileSize);
		file.close();

		vk::ShaderModuleCreateInfo createInfo = {};

		createInfo.codeSize = buffer.size() * sizeof(uint32_t);
		createInfo.pCode = buffer.data();

		vk::ShaderModule shaderModule;
		return std::move(device.createShaderModuleUnique(createInfo));
	}

	vk::PipelineShaderStageCreateInfo makeShaderStageCreateInfo(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule) {
		vk::PipelineShaderStageCreateInfo info{};

		info.stage = stage;
		info.module = shaderModule;
		info.pName = "main";
		return info;
	}

#endif

	vk::FenceCreateInfo makeDefaultFenceCI();

	vk::AttachmentDescription makeDefaultColorAttackmentDescription();

#if defined(VULKANHELPER_IMPLEMENTATION)
	vk::FenceCreateInfo makeDefaultFenceCI() {
		vk::FenceCreateInfo info{};
		info.flags |= vk::FenceCreateFlagBits::eSignaled;
		return info;
	}

	vk::AttachmentDescription makeDefaultColorAttackmentDescription() {
		return vk::AttachmentDescription{
			.format = vk::Format::eUndefined,
			.samples = vk::SampleCountFlagBits::e1,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
			.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			.initialLayout = vk::ImageLayout::eUndefined,
			.finalLayout = vk::ImageLayout::ePresentSrcKHR,
		};
	}
#endif

	template <typename T>
	class Pool {
	public:
		Pool() = default;
		Pool(std::function<T(void)> creator, std::function<void(T)> destroyer, std::function<void(T)> resetter) : creator{ std::move(creator) }, destroyer{ std::move(destroyer) }, resetter{ std::move(resetter) } {}

		Pool(Pool&& other) {
			this->creator = std::move(other.creator);
			this->destroyer = std::move(other.destroyer);
			this->resetter = std::move(other.resetter);
			this->pool = std::move(other.pool);
			this->usedList = std::move(other.usedList);
		}

		Pool& operator=(Pool&& other) {
			if (&other == this)
				return *this;
			return *new (this) Pool(std::move(other));
		}

		~Pool() {
			for (auto& el : pool) {
				destroyer(el);
			}
			for (auto& el : usedList) {
				destroyer(el);
			}
			pool.clear();
		}

		void flush() {
			for (auto& el : usedList) {
				resetter(el);
			}
			pool.insert(pool.end(), usedList.begin(), usedList.end());
			usedList.clear();
		}

		T get() {
			if (pool.size() == 0) {
				pool.push_back(creator());
			}

			auto el = pool.back();
			pool.pop_back();
			usedList.push_back(el);
			return el;
		}

	private:
		std::function<T(void)> creator;
		std::function<void(T)> destroyer;
		std::function<void(T)> resetter;
		std::vector<T> pool;
		std::vector<T> usedList;
	};



	class DescriptorLayoutCache {
	public:
		DescriptorLayoutCache(vk::Device device);
		vk::DescriptorSetLayout getLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings);
	private:
		struct DescriptorLayoutHash {
			std::size_t operator()(const std::vector<vk::DescriptorSetLayoutBinding>& bindings) const;
		};

		vk::Device device;
		std::unordered_map<std::vector<vk::DescriptorSetLayoutBinding>, vk::UniqueDescriptorSetLayout, DescriptorLayoutHash> bindingsToLayout;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
	DescriptorLayoutCache::DescriptorLayoutCache(vk::Device device)
		: device{ device } {
	}

	vk::DescriptorSetLayout DescriptorLayoutCache::getLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings) {
		auto iter = bindingsToLayout.find(bindings);
		if (iter != bindingsToLayout.end()) {
			return *iter->second;
		}
		else {
			auto allocateInfo = vk::DescriptorSetLayoutCreateInfo{
				.bindingCount = static_cast<uint32_t>(bindings.size()),
				.pBindings = bindings.data(),
			};
			return *(bindingsToLayout[bindings] = device.createDescriptorSetLayoutUnique(allocateInfo));
		}
	}
	std::size_t DescriptorLayoutCache::DescriptorLayoutHash::operator()(const std::vector<vk::DescriptorSetLayoutBinding>& bindings) const {
		size_t h{ 0 };
		for (const auto& binding : bindings) {
			h ^= static_cast<size_t>(binding.descriptorType);
			h ^= static_cast<size_t>(binding.descriptorType);
		}
		return h;
	}
#endif



	class DescriptorAllocator {
	public:
		DescriptorAllocator() = default;
		DescriptorAllocator(vk::Device device, std::vector<vk::DescriptorPoolSize> specificPoolSizes = {}, vk::DescriptorPoolCreateFlagBits poolCreateFlags = {}, uint32_t maxSetsPerPool = 500);
		void reset();
		vk::DescriptorSet allocate(vk::DescriptorSetLayout layout);
		std::vector<vk::DescriptorSet> allocate(vk::DescriptorSetLayout layout, uint32_t count);

	protected:
		DescriptorAllocator(vk::Device device, vk::DescriptorPoolCreateFlagBits poolFlags, uint32_t maxSetsPerPool);

		vk::UniqueDescriptorPool getUnusedPool();
		vk::UniqueDescriptorPool createPool();

		const vk::DescriptorPoolCreateFlagBits poolCreateFlags;
		const uint32_t maxSetsPerPool;

		vk::Device device;
		vk::UniqueDescriptorPool currentPool;
		std::vector<vk::DescriptorPoolSize> descriptorPoolSizes;
		std::vector<vk::UniqueDescriptorPool> usedPools;
		std::vector<vk::UniqueDescriptorPool> unusedPools;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
	DescriptorAllocator::DescriptorAllocator(vk::Device device, std::vector<vk::DescriptorPoolSize> specificPoolSizes, vk::DescriptorPoolCreateFlagBits poolCreateFlags, uint32_t maxSetsPerPool)
		: device{ device }, maxSetsPerPool{ maxSetsPerPool }, poolCreateFlags{ poolCreateFlags } {
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eSampler, .descriptorCount = 500u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = 4000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eSampledImage, .descriptorCount = 4000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eUniformTexelBuffer, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eStorageTexelBuffer, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eUniformBufferDynamic, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eStorageBufferDynamic, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eInputAttachment, .descriptorCount = 500u });

		for (auto specificPoolSize : specificPoolSizes) {
			vk::DescriptorPoolSize* foundValue = nullptr;
			for (auto& poolSize : this->descriptorPoolSizes) {
				if (poolSize.type == specificPoolSize.type) {
					foundValue = &poolSize;
					break;
				}
			}
			if (foundValue) {
				foundValue->descriptorCount = specificPoolSize.descriptorCount;
			}
			else {
				this->descriptorPoolSizes.push_back(specificPoolSize);
			}
		}

		currentPool = createPool();
	}
	DescriptorAllocator::DescriptorAllocator(vk::Device device, vk::DescriptorPoolCreateFlagBits poolFlags, uint32_t maxSetsPerPool)
		: device{ device }, poolCreateFlags{ poolFlags }, maxSetsPerPool{ maxSetsPerPool } {
	}
	void DescriptorAllocator::reset() {
		if (currentPool) {
			device.resetDescriptorPool(*currentPool);
			unusedPools.push_back(std::move(currentPool));
			currentPool.release();
		}

		for (auto&& pool : usedPools) {
			device.resetDescriptorPool(*pool);
			unusedPools.push_back(std::move(pool));
		}
		usedPools.clear();
	}
	vk::DescriptorSet DescriptorAllocator::allocate(vk::DescriptorSetLayout layout) {
		auto result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = 1, .pSetLayouts = &layout });
		if (result.size() == 0) /* we need to reallocate with another pool */ {
			usedPools.push_back(std::move(currentPool));
			currentPool = getUnusedPool();

			result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = 1, .pSetLayouts = &layout });

			if (result.size() == 0) {
				// possible cause for this error is that the requested descriptor count exceeds the maximum descriptor pool size
				std::cerr << "error: discriptor allocator can not allocate this descriptor set layout!\n";
				assert(false);
			}
		}

		return std::move(result.front());
	}
	std::vector<vk::DescriptorSet> DescriptorAllocator::allocate(vk::DescriptorSetLayout layout, uint32_t count) {
		std::vector<vk::DescriptorSetLayout> layouts(count, layout);
		auto result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = count, .pSetLayouts = layouts.data() });
		if (result.size() == 0) /* we need to reallocate with another pool */ {
			usedPools.push_back(std::move(currentPool));
			currentPool = getUnusedPool();

			result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = count, .pSetLayouts = layouts.data() });

			if (result.size() == 0) {
				// possible cause for this error is that the requested descriptor count exceeds the maximum descriptor pool size
				std::cerr << "error: discriptor allocator can not allocate this descriptor set layout!\n";
				assert(false);
			}
		}

		return std::move(result);
	}
	vk::UniqueDescriptorPool DescriptorAllocator::getUnusedPool() {
		if (unusedPools.empty()) {
			return std::move(createPool());
		}
		else {
			auto pool = std::move(unusedPools.back());
			unusedPools.pop_back();
			return std::move(pool);
		}
	}
	vk::UniqueDescriptorPool DescriptorAllocator::createPool() {
		auto allocInfo = vk::DescriptorPoolCreateInfo{
			.flags = poolCreateFlags,
			.maxSets = maxSetsPerPool,
			.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
			.pPoolSizes = descriptorPoolSizes.data(),
		};

		return device.createDescriptorPoolUnique(allocInfo);
	}
#endif



	class DescriptorSetAllocator : public DescriptorAllocator {
	public:
		DescriptorSetAllocator(vk::Device device, vk::DescriptorSetLayoutCreateInfo layoutCreateInfo, vk::DescriptorSetLayout layout, vk::DescriptorPoolCreateFlagBits poolFlags = {}, uint32_t maxSetsPerPool = 500);
		vk::DescriptorSet allocate();
		std::vector<vk::DescriptorSet> allocate(uint32_t count);

	private:
		using DescriptorAllocator::allocate;
		vk::DescriptorSetLayout layout;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
	DescriptorSetAllocator::DescriptorSetAllocator(vk::Device device, vk::DescriptorSetLayoutCreateInfo layoutCreateInfo, vk::DescriptorSetLayout layout, vk::DescriptorPoolCreateFlagBits poolFlags, uint32_t maxSetsPerPool)
		: DescriptorAllocator{ device, poolFlags, maxSetsPerPool }, layout{ layout } {
		const uint32_t descriptorPoolSizesCount = layoutCreateInfo.bindingCount;
		this->descriptorPoolSizes.resize(descriptorPoolSizesCount);

		for (uint32_t i = 0; i < descriptorPoolSizesCount; i++) {
			this->descriptorPoolSizes[i].type = layoutCreateInfo.pBindings[i].descriptorType;
			this->descriptorPoolSizes[i].descriptorCount = layoutCreateInfo.pBindings[i].descriptorCount * maxSetsPerPool;
		}

		currentPool = createPool();
	}
	vk::DescriptorSet DescriptorSetAllocator::allocate() {
		auto result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = 1, .pSetLayouts = &layout });
		if (result.size() == 0) /* we need to reallocate with another pool */ {
			usedPools.push_back(std::move(currentPool));
			currentPool = getUnusedPool();

			result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = 1, .pSetLayouts = &layout });
		}

		return std::move(result.front());
	}
	std::vector<vk::DescriptorSet> DescriptorSetAllocator::allocate(uint32_t count) {
		std::vector<vk::DescriptorSetLayout> layouts(count, layout);
		auto result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = count, .pSetLayouts = layouts.data() });
		if (result.size() == 0) /* we need to reallocate with another pool */ {
			usedPools.push_back(std::move(currentPool));
			currentPool = getUnusedPool();

			result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = count, .pSetLayouts = layouts.data() });
		}

		return std::move(result);
	}
#endif



	vk::DescriptorSet createDescriptorSet(const std::vector<vk::DescriptorSetLayoutBinding>& bindings, DescriptorAllocator& alloc, DescriptorLayoutCache& layoutCache);

#if defined(VULKANHELPER_IMPLEMENTATION)
	vk::DescriptorSet createDescriptorSet(const std::vector<vk::DescriptorSetLayoutBinding>& bindings, DescriptorAllocator& alloc, DescriptorLayoutCache& layoutCache) {
		return alloc.allocate(layoutCache.getLayout(bindings));
	}
#endif



	// TODO ADD DESCRIPTOR SET BUILDER THAT DIRECTLY LINKS THE DESCRIPTORS



	// TODO ADD EASY TO USE DESCRIPTOR BINDING UPDATE FUNCTION



	class CommandBufferAllocator {
	public:
		CommandBufferAllocator() = default;
		CommandBufferAllocator(vk::Device device, const vk::CommandPoolCreateInfo& poolCI, const vk::CommandBufferLevel& bufferLevel = vk::CommandBufferLevel::ePrimary);

		void flush();

		vk::CommandBuffer getElement();

		vk::CommandPool get();

		vk::CommandPool operator*();

	private:
		vk::Device device;
		vk::UniqueCommandPool pool;
		vk::CommandBufferLevel bufferLevel;
		std::vector<vk::CommandBuffer> unused;
		std::vector<vk::CommandBuffer> used;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
	CommandBufferAllocator::CommandBufferAllocator(vk::Device device, const vk::CommandPoolCreateInfo& createInfo, const vk::CommandBufferLevel& bufferLevel) : device{ device }, pool{ device.createCommandPoolUnique(createInfo) }, bufferLevel{ bufferLevel } {}
	void CommandBufferAllocator::flush() {
		device.resetCommandPool(*pool);
		unused.insert(unused.end(), used.begin(), used.end());
		used.clear();
	}

	vk::CommandBuffer CommandBufferAllocator::getElement() {
		if (unused.empty()) {
			unused.push_back(device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{ .commandPool = pool.get(), .level = bufferLevel, .commandBufferCount = 1 }).back());
		}
		auto buffer = unused.back();
		unused.pop_back();
		used.push_back(buffer);
		return buffer;
	}

	vk::CommandPool CommandBufferAllocator::get() {
		return pool.get();
	}

	vk::CommandPool CommandBufferAllocator::operator*() {
		return get();
	}

#endif
} // namespace vkh

namespace vkh_detail {
	static PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
	static PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;
} // namespace vkh_detail

#if defined(VULKANHELPER_IMPLEMENTATION)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
	return vkh_detail::pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const* pAllocator) {
	return vkh_detail::pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}
#endif

namespace vkh {
	vk::Instance createInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

	vk::DebugUtilsMessengerEXT createDebugMessenger(vk::Instance instance);

	vk::PhysicalDevice selectPhysicalDevice(vk::Instance instance, const std::function<std::size_t(vk::PhysicalDevice)>& rateDeviceSuitability);

	vk::Device createLogicalDevice(vk::PhysicalDevice physicalDevice, const std::set<std::size_t>& queueIndices, const std::vector<const char*>& extensions);

	std::uint32_t findMemoryTypeIndex(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask);

#if defined(VULKANHELPER_IMPLEMENTATION)
	vk::Instance createInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions) {
		vk::ApplicationInfo vulkanApplicationInfo{ .apiVersion = VK_API_VERSION_1_1 };
		return vk::createInstance({
			.pApplicationInfo = &vulkanApplicationInfo,
			.enabledLayerCount = static_cast<uint32_t>(layers.size()),
			.ppEnabledLayerNames = layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
			.ppEnabledExtensionNames = extensions.data(),
			});
	}

	vk::DebugUtilsMessengerEXT createDebugMessenger(vk::Instance instance) {
		vkh_detail::pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
		if (!vkh_detail::pfnVkCreateDebugUtilsMessengerEXT)
			throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function.");
		vkh_detail::pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
		if (!vkh_detail::pfnVkDestroyDebugUtilsMessengerEXT)
			throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function.");

		return instance.createDebugUtilsMessengerEXT(vk::DebugUtilsMessengerCreateInfoEXT{
			.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
			.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
			.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
								  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) -> VkBool32 {
				if (pCallbackData->messageIdNumber == 648835635) {
					// UNASSIGNED-khronos-Validation-debug-build-warning-message
					return VK_FALSE;
				}
				if (pCallbackData->messageIdNumber == 767975156) {
					// UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension
					return VK_FALSE;
				}
				std::string message =
					vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) + ": " +
					vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) + ":\n\ttmessage name   = <" +
					pCallbackData->pMessageIdName + ">\n\tmessage number = " +
					std::to_string(pCallbackData->messageIdNumber) + "\n\tmessage        = <" +
					pCallbackData->pMessage + ">\n";
				if (0 < pCallbackData->queueLabelCount) {
					message += "\tQueue Labels:\n";
					for (uint8_t i = 0; i < pCallbackData->queueLabelCount; i++)
						message += std::string("\t\tlabelName = <") + pCallbackData->pQueueLabels[i].pLabelName + ">\n";
				}
				if (0 < pCallbackData->cmdBufLabelCount) {
					message += "\tCommandBuffer Labels:\n";
					for (uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; i++)
						message += std::string("\t\tlabelName = <") + pCallbackData->pCmdBufLabels[i].pLabelName + ">\n";
				}
				if (0 < pCallbackData->objectCount) {
					message += "\tObjects:\n";
					for (uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
						message += std::string("\t\tlabelName = <") +
								   pCallbackData->pCmdBufLabels[i].pLabelName + ">\nObject " + std::to_string(i) + "\n\t\t\tobjectType   = " +
								   vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) + "\n\t\t\tobjectHandle = " +
								   std::to_string(pCallbackData->pObjects[i].objectHandle) + "\n";
						if (pCallbackData->pObjects[i].pObjectName)
							message += std::string("\t\t\tobjectName   = <") + pCallbackData->pObjects[i].pObjectName + ">\n";
					}
				}
				//MessageBox(nullptr, message.c_str(), "Vulkan Validation Error", MB_OK);
				return VK_TRUE;
			},
			});
	}

	vk::PhysicalDevice selectPhysicalDevice(vk::Instance instance, const std::function<std::size_t(vk::PhysicalDevice)>& rateDeviceSuitability) {
		auto devices = instance.enumeratePhysicalDevices();
		std::vector<std::size_t> devicesSuitability;
		devicesSuitability.reserve(devices.size());
		for (const auto& device : devices)
			devicesSuitability.push_back(rateDeviceSuitability(device));
		auto bestDeviceIter = std::max_element(devicesSuitability.begin(), devicesSuitability.end());
		if (*bestDeviceIter == 0)
			throw std::runtime_error("Failed to find a suitable physical device");
		return devices[std::distance(devicesSuitability.begin(), bestDeviceIter)];
	}

	vk::Device createLogicalDevice(vk::PhysicalDevice physicalDevice, const std::set<std::size_t>& queueIndices, const std::vector<const char*>& extensions) {
		float queuePriority = 0.0f;
		std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateinfos;
		for (auto index : queueIndices) {
			deviceQueueCreateinfos.push_back({
				.queueFamilyIndex = static_cast<std::uint32_t>(index),
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
				});
		}
		return physicalDevice.createDevice({
			.queueCreateInfoCount = static_cast<std::uint32_t>(deviceQueueCreateinfos.size()),
			.pQueueCreateInfos = deviceQueueCreateinfos.data(),
			.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
			.ppEnabledExtensionNames = extensions.data(),
			});
	}

	std::uint32_t findMemoryTypeIndex(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask) {
		std::uint32_t type_index = std::uint32_t(~0);
		for (std::uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
			if (typeBits & 1 && (memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask) {
				type_index = i;
				break;
			}
			typeBits >>= 1;
		}
		if (type_index == std::uint32_t(~0))
			throw std::runtime_error("Unable to find suitable memory type index");
		return type_index;
	}
#endif
} // namespace vkh
