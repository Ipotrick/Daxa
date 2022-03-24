#pragma once

#include "Daxa.hpp"

class RenderContext {
public:
    RenderContext(daxa::Window& window) 
		: device{ daxa::gpu::Device::create() }
		, queue{ this->device->createCommandQueue({.batchCount = 3}) }
		, swapchain{ this->device->createSwapchain({
			.surface = window.getSurface(),
			.width = window.getWidth(),
			.height = window.getHeight(),
			.additionalUses = VK_IMAGE_USAGE_SAMPLED_BIT,
			.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
		})}
		, swapchainImage{ this->swapchain->aquireNextImage() }
		, presentSignal{ this->device->createSignal({.debugName = "present signal"}) }
		, pipelineCompiler{ this->device->createPipelineCompiler() }
    {
		defaultSampler = device->createSampler({});
		auto cmd = queue->getCommandList({});
		recreateFramebuffer(cmd, window.getWidth(), window.getHeight());
		cmd->finalize();
		queue->submitBlocking({.commandLists = {cmd}});

		pipelineCompiler->addShaderSourceRootPath("./DaxaMeshview/shaders/");
    }

    ~RenderContext() {
		waitIdle();
    }

    void resize(daxa::gpu::CommandListHandle& cmdList, u32 width, u32 height) {
        swapchain->resize(VkExtent2D{ .width = width, .height = height });
        swapchainImage = swapchain->aquireNextImage();
        recreateFramebuffer(cmdList, width, height);
    }

	void recreateFramebuffer(daxa::gpu::CommandListHandle& cmd, u32 width, u32 height) {
		std::string depthMapName = "depth image";
		this->depthImage = device->createImageView({
			.image = device->createImage({
				.format = VK_FORMAT_D32_SFLOAT,
				.extent = { width, height, 1},
				.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			}),
			.format = VK_FORMAT_D32_SFLOAT,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.defaultSampler = defaultSampler,
			.debugName = depthMapName.c_str(),//"depth image",
		});

		this->normalsImage = device->createImageView({
			.image = device->createImage({
				.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
				.extent = { width, height, 1},
				.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			}),
			.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
			.defaultSampler = defaultSampler,
			.debugName = "normals image",
		});

		this->hdrImage = device->createImageView({
			.image = device->createImage({
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,
				.extent = { width, height, 1 },
				.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			}),
			.format = VK_FORMAT_R16G16B16A16_SFLOAT,
			.debugName = "main hdr color image",
		});

		cmd->insertImageBarriers(std::array{
			daxa::gpu::ImageBarrier{
				.barrier = daxa::gpu::FULL_MEMORY_BARRIER,
				.image = depthImage,
				.layoutAfter = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			},
			daxa::gpu::ImageBarrier{
				.barrier = daxa::gpu::FULL_MEMORY_BARRIER,
				.image = normalsImage,
				.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			},
			daxa::gpu::ImageBarrier{
				.barrier = daxa::gpu::FULL_MEMORY_BARRIER,
				.image = hdrImage,
				.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			},
		});
	}

	void present() {
		queue->present(std::move(swapchainImage), presentSignal);
		swapchainImage = swapchain->aquireNextImage();
		queue->nextBatch();
		queue->checkForFinishedSubmits();
	}

	void waitIdle() {
		queue->waitIdle();
		queue->checkForFinishedSubmits();
		device->waitIdle();
	}

    daxa::gpu::DeviceHandle device = {};
	daxa::PipelineCompilerHandle pipelineCompiler;
	daxa::gpu::CommandQueueHandle queue = {};
	daxa::gpu::SwapchainHandle swapchain = {};
	daxa::gpu::SwapchainImage swapchainImage = {};
	daxa::gpu::SignalHandle presentSignal = {};
	daxa::gpu::ImageViewHandle depthImage = {};
	daxa::gpu::ImageViewHandle normalsImage = {};
	daxa::gpu::ImageViewHandle hdrImage = {};
	daxa::gpu::SamplerHandle defaultSampler = {};
};

struct Primitive {
	u32 indexCount = 0;
	daxa::gpu::BufferHandle indiexBuffer = {};
	daxa::gpu::BufferHandle vertexPositions = {};
	daxa::gpu::BufferHandle vertexUVs = {};
	daxa::gpu::BufferHandle vertexNormals = {};
	daxa::gpu::BufferHandle vertexTangents = {};
	daxa::gpu::ImageViewHandle albedoTexture = {};
	daxa::gpu::ImageViewHandle normalTexture = {};
};

struct DrawPrimCmd {
	glm::mat4 transform = {};
	Primitive* prim = {};
};