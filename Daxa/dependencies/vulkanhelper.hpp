#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <filesystem>

#ifdef VULKANHELPER_IMPLEMENTATION
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <iostream>
#include <fstream>
#endif 

#include <vulkan/vulkan.hpp>

namespace vkh {
	struct VertexDescription {
		std::vector<vk::VertexInputBindingDescription> bindings;
		std::vector<vk::VertexInputAttributeDescription> attributes;
		vk::PipelineVertexInputStateCreateFlags flags{};

		vk::PipelineVertexInputStateCreateInfo makePipelineVertexInputStateCreateInfo() const
		{
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
		VertexDiscriptionBuilder& setAttribute(vk::Format format);
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

#ifdef VULKANHELPER_IMPLEMENTATION
	VertexDiscriptionBuilder& VertexDiscriptionBuilder::beginBinding(uint32_t stride, vk::VertexInputRate inputRate)
	{
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

	VertexDiscriptionBuilder& VertexDiscriptionBuilder::setAttribute(vk::Format format)
	{
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

	VertexDiscriptionBuilder& VertexDiscriptionBuilder::stageCreateFlags(vk::PipelineVertexInputStateCreateFlags flags)
	{
		this->flags = flags;
		return *this;
	}

	VertexDescription VertexDiscriptionBuilder::build()
	{
		assert(bindings.size() > 0);
		return VertexDescription{
			bindings,
			attributes,
			flags
		};
	}
#endif



	vk::PipelineRasterizationStateCreateInfo makeDefaultRasterisationStateCreateInfo(vk::PolygonMode polygonMode);

	vk::PipelineMultisampleStateCreateInfo makeDefaultMultisampleStateCreateInfo();

	vk::PipelineColorBlendAttachmentState makeDefaultColorBlendSAttachmentState();

	struct Pipeline {
		vk::UniquePipeline pipeline;
		vk::UniquePipelineLayout layout;
	};

	class PipelineBuilder {
	public:
		std::optional<vk::Viewport> viewport;
		std::optional<vk::Rect2D> scissor;
		std::optional<vk::PipelineVertexInputStateCreateInfo> vertexInput;
		std::optional<vk::PipelineInputAssemblyStateCreateInfo> inputAssembly;
		std::optional<vk::PipelineRasterizationStateCreateInfo> rasterization;
		std::optional<vk::PipelineMultisampleStateCreateInfo> multisampling;
		std::optional<vk::PipelineDepthStencilStateCreateInfo> depthStencil;
		std::optional<vk::PipelineColorBlendAttachmentState> colorBlendAttachment;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		std::vector<vk::DynamicState> dynamicStateEnable;
		std::vector<vk::PushConstantRange> pushConstants;

		Pipeline build(vk::Device device, vk::RenderPass pass, uint32_t subpass = 0);
	};

#ifdef VULKANHELPER_IMPLEMENTATION
	vk::PipelineRasterizationStateCreateInfo makeDefaultRasterisationStateCreateInfo(vk::PolygonMode polygonMode)
	{
		return vk::PipelineRasterizationStateCreateInfo{
			.polygonMode = polygonMode,
			.frontFace = vk::FrontFace::eClockwise,
			.lineWidth = 1.0f,
		};
	}

	vk::PipelineMultisampleStateCreateInfo makeDefaultMultisampleStateCreateInfo()
	{
		return vk::PipelineMultisampleStateCreateInfo{ .minSampleShading = 1.0f };
	}

	vk::PipelineColorBlendAttachmentState makeDefaultColorBlendSAttachmentState()
	{
		return vk::PipelineColorBlendAttachmentState{
		.colorWriteMask =
			vk::ColorComponentFlagBits::eR |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eA,
		};
	}

	Pipeline PipelineBuilder::build(vk::Device device, vk::RenderPass pass, uint32_t subpass)
	{
		if (!vertexInput) {
			std::cout << "error: vertexInput was not specified in pipeline builder!\n";
			exit(-1);
		}

		// set state create infos:
		vk::PipelineVertexInputStateCreateInfo    pvertexInputCI = vertexInput.value();
		vk::PipelineColorBlendAttachmentState     pcolorBlendAttachmentCI = colorBlendAttachment.value_or(vkh::makeDefaultColorBlendSAttachmentState());
		vk::PipelineInputAssemblyStateCreateInfo  pinputAssemlyStateCI = inputAssembly.value_or(vk::PipelineInputAssemblyStateCreateInfo{ .topology = vk::PrimitiveTopology::eTriangleList });
		vk::PipelineRasterizationStateCreateInfo  prasterizationStateCI = rasterization.value_or(vkh::makeDefaultRasterisationStateCreateInfo(vk::PolygonMode::eFill));
		vk::PipelineMultisampleStateCreateInfo    multisamplerStateCI = multisampling.value_or(vkh::makeDefaultMultisampleStateCreateInfo());
		vk::PipelineDepthStencilStateCreateInfo   pDepthStencilStateCI = depthStencil.value_or(vk::PipelineDepthStencilStateCreateInfo{});
		vk::Viewport                              pviewport = viewport.value_or(vk::Viewport{ .width = 1,.height = 1 });
		vk::Rect2D                                pscissor = scissor.value_or(vk::Rect2D{ .extent = {static_cast<uint32_t>(pviewport.width), static_cast<uint32_t>(pviewport.height)} });

		Pipeline pipeline;

		//build pipeline layout:
		vk::PipelineLayoutCreateInfo layoutCI{
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

		//setup dummy color blending. We aren't using transparent objects yet
		//the blending is just "no blend", but we do write to the color attachment
		vk::PipelineColorBlendStateCreateInfo colorBlendingSCI{
			.logicOpEnable = VK_FALSE,
			.logicOp = vk::LogicOp::eCopy,
			.attachmentCount = 1,
			.pAttachments = &pcolorBlendAttachmentCI,
		};

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

		auto ret = device.createGraphicsPipelineUnique({}, pipelineCI);
		if (ret.result != vk::Result::eSuccess) {
			std::cerr << "error: could not compile pipeline!\n";
			exit(-1);
		}
		pipeline.pipeline = std::move(ret.value);

		return pipeline;
	}
#endif



	std::optional<vk::UniqueShaderModule> loadShaderModule(vk::Device device, std::filesystem::path filePath);

	vk::PipelineShaderStageCreateInfo makeShaderStageCreateInfo(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule);

#ifdef VULKANHELPER_IMPLEMENTATION


	std::optional<vk::UniqueShaderModule> loadShaderModule(vk::Device device, std::filesystem::path filePath)
	{

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

	vk::PipelineShaderStageCreateInfo makeShaderStageCreateInfo(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule)
	{
		vk::PipelineShaderStageCreateInfo info{};

		info.stage = stage;
		info.module = shaderModule;
		info.pName = "main";
		return info;
		}

#endif



	vk::FenceCreateInfo makeDefaultFenceCI();

	vk::AttachmentDescription makeDefaultColorAttackmentDescription();

#ifdef VULKANHELPER_IMPLEMENTATION
	vk::FenceCreateInfo makeDefaultFenceCI()
	{
		vk::FenceCreateInfo info{};
		info.flags |= vk::FenceCreateFlagBits::eSignaled;
		return info;
	}

	vk::AttachmentDescription makeDefaultColorAttackmentDescription()
	{
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



	template<typename T>
	class Pool {
	public:
		Pool() = default;
		Pool(std::function<T(void)> creator, std::function<void(T)> destroyer, std::function<void(T)> resetter) :
			creator{ std::move(creator) }, destroyer{ std::move(destroyer) }, resetter{ std::move(resetter) }
		{ }

		Pool(Pool&& other)
		{
			this->creator = std::move(other.creator);
			this->destroyer = std::move(other.destroyer);
			this->resetter = std::move(other.resetter);
			this->pool = std::move(other.pool);
			this->zombies = std::move(other.zombies);
		}

		Pool& operator=(Pool&& other)
		{
			if (&other == this) return *this;
			return *new (this) Pool(std::move(other));
		}

		~Pool()
		{
			for (auto& el : pool) {
				destroyer(el);
			}
			for (auto& list : zombies) {
				for (auto& el : list) {
					destroyer(el);
				}
			}
			pool.clear();
		}

		auto flush()
		{
			for (auto& el : zombies[1]) {
				resetter(el);
			}
			pool.insert(pool.end(), zombies[1].begin(), zombies[1].end());
			zombies[1].clear();
			std::swap(zombies[0], zombies[1]);
		}

		T get()
		{
			if (pool.size() == 0) {
				pool.push_back(creator());
			}

			auto el = pool.back();
			pool.pop_back();
			zombies[0].push_back(el);
			return el;
		}
	private:
		std::function<T(void)> creator;
		std::function<void(T)> destroyer;
		std::function<void(T)> resetter;
		std::vector<T> pool;
		std::array<std::vector<T>, 2> zombies{ std::vector<T>{}, std::vector<T>{} };
	};



	vk::UniqueCommandPool makeUniqueCommandPool(
		vk::Device device,
		uint32_t queueFamilyIndex,
		vk::CommandPoolCreateFlagBits flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	class CommandPool {
	public:
		CommandPool(
			vk::Device device,
			uint32_t queueFamilyIndex,
			vk::CommandPoolCreateFlagBits flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer
		);

		CommandPool(CommandPool&& other) noexcept;

		operator const vk::UniqueCommandPool& ();

		const vk::UniqueCommandPool& getPool();

		vk::CommandBuffer getBuffer();

		void flush();

	private:
		uint32_t queueFamilyIndex{ 0xFFFFFFFF };
		vk::Device device;
		vk::UniqueCommandPool pool{};
		Pool<vk::CommandBuffer> bufferPool;
	};

#ifdef VULKANHELPER_IMPLEMENTATION

	vk::UniqueCommandPool makeUniqueCommandPool(
		vk::Device device,
		uint32_t queueFamilyIndex,
		vk::CommandPoolCreateFlagBits flags)
	{
		vk::CommandPoolCreateInfo cmdPoolCreateInfo = {};
		cmdPoolCreateInfo.flags = flags;   // we can reset individual command buffers from this pool
		cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

		return device.createCommandPoolUnique(cmdPoolCreateInfo);
	}

	CommandPool::CommandPool(
		vk::Device device,
		uint32_t queueFamilyIndex,
		vk::CommandPoolCreateFlagBits flags
	) :
		queueFamilyIndex{ queueFamilyIndex },
		device{ device },
		pool{ makeUniqueCommandPool(device, queueFamilyIndex, flags ) },
		bufferPool{
			[=]() { return device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{.commandPool = *pool, .commandBufferCount = 1}).front(); },
			[=](vk::CommandBuffer buffer) { /* gets freed anyway */ },
			[=](vk::CommandBuffer buffer) { buffer.reset(); }
	}
	{ }

	CommandPool::CommandPool(CommandPool&& other) noexcept
	{
		this->queueFamilyIndex = other.queueFamilyIndex;
		this->device = std::move(other.device);
		this->pool = std::move(other.pool);
		this->bufferPool = std::move(other.bufferPool);
		other.queueFamilyIndex = 0xFFFFFFFF;
	}

	CommandPool::operator const vk::UniqueCommandPool& ()
	{
		assert(pool); // USE AFTER MOVE
		return pool;
	}

	const vk::UniqueCommandPool& CommandPool::getPool()
	{
		assert(pool); // USE AFTER MOVE
		return pool;
	}

	vk::CommandBuffer CommandPool::getBuffer()
	{
		return bufferPool.get();
	}

	void CommandPool::flush()
	{
		bufferPool.flush();
	}
#endif
}