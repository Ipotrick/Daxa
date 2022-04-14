#pragma once

#include "Daxa.hpp"

class RenderContext {
public:
    RenderContext(daxa::Window& window) 
		: device{ daxa::Device::create() }
		, pipelineCompiler{ this->device->createPipelineCompiler() }
		, queue{ this->device->createCommandQueue({.batchCount = 2}) }
		, swapchain{ this->device->createSwapchain({
			.surface = window.getSurface(),
			.width = window.getWidth(),
			.height = window.getHeight(),
			.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
			.additionalUses = VK_IMAGE_USAGE_SAMPLED_BIT,
		})}
		, swapchainImage{ this->swapchain->aquireNextImage(), }
		, presentSignal{ this->device->createSignal({.debugName = "present signal"}) }
    {
		defaultSampler = device->createSampler({
			.maxLod = 14,
			.debugName = "default sampler",
		});
		auto cmd = queue->getCommandList({});
		recreateFramebuffer(cmd, window.getWidth(), window.getHeight());
		cmd.finalize();
		queue->submitBlocking({.commandLists = {cmd}});

		pipelineCompiler->addShaderSourceRootPath("./DaxaMeshview/shaders/");
		pipelineCompiler->addShaderSourceRootPath("./Daxa/shaders/");
    }

    ~RenderContext() {
		waitIdle();
    }

    void resize(daxa::CommandListHandle& cmdList, u32 width, u32 height) {
        swapchain->resize(VkExtent2D{ .width = width, .height = height });
        swapchainImage = swapchain->aquireNextImage();
        recreateFramebuffer(cmdList, width, height);
    }

	void recreateFramebuffer(daxa::CommandListHandle& cmd, u32 width, u32 height) {
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
			.debugName = "depth image",
		});
		this->priorFrameDepthImage = device->createImageView({
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
			.debugName = "prior frame depth image"
		});

		this->normalsImage = device->createImageView({
			.image = device->createImage({
				.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
				.extent = { width, height, 1},
				.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				.debugName = "normals image",
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
				.debugName = "main hdr color image",
			}),
			.format = VK_FORMAT_R16G16B16A16_SFLOAT,
			.defaultSampler = defaultSampler,
			.debugName = "main hdr color image",
		});
		this->priorFrameHdrImage = device->createImageView({
			.image = device->createImage({
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,
				.extent = { width, height, 1 },
				.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				.debugName = "prior frame hdr color image",
			}),
			.format = VK_FORMAT_R16G16B16A16_SFLOAT,
			.defaultSampler = defaultSampler,
			.debugName = "prior frame hdr color image",
		});
		this->motionImage = device->createImageView({
			.image = device->createImage({
				.format = VK_FORMAT_R16G16_SFLOAT,
				.extent = { width, height, 1 },
				.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				.debugName = "prior frame hdr color image",
			}),
			.format = VK_FORMAT_R16G16_SFLOAT,
			.defaultSampler = defaultSampler,
			.debugName = "prior frame hdr color image",
		});

		cmd.queueImageBarrier({
			.barrier = daxa::FULL_MEMORY_BARRIER,
			.image = depthImage,
			.layoutAfter = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		});
		cmd.queueImageBarrier({
			.barrier = daxa::FULL_MEMORY_BARRIER,
			.image = priorFrameDepthImage,
			.layoutAfter = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		});
		cmd.queueImageBarrier({
			.barrier = daxa::FULL_MEMORY_BARRIER,
			.image = normalsImage,
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		});
		cmd.queueImageBarrier({
			.barrier = daxa::FULL_MEMORY_BARRIER,
			.image = hdrImage,
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		});
		cmd.queueImageBarrier({
			.barrier = daxa::FULL_MEMORY_BARRIER,
			.image = priorFrameHdrImage,
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		});
		cmd.queueImageBarrier({
			.barrier = daxa::FULL_MEMORY_BARRIER,
			.image = motionImage,
			.layoutAfter = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
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

    daxa::DeviceHandle device = {};
	daxa::PipelineCompilerHandle pipelineCompiler;
	daxa::CommandQueueHandle queue = {};
	daxa::SwapchainHandle swapchain = {};
	daxa::SwapchainImage swapchainImage = {};
	daxa::SignalHandle presentSignal = {};
	daxa::ImageViewHandle depthImage = {};
	daxa::ImageViewHandle normalsImage = {};
	daxa::ImageViewHandle hdrImage = {};
	daxa::SamplerHandle defaultSampler = {};

	// used by TAA:
	daxa::ImageViewHandle motionImage = {};
	daxa::ImageViewHandle priorFrameHdrImage = {};
	daxa::ImageViewHandle priorFrameDepthImage = {};
};

struct Primitive {
	u32 indexCount = 0;
	daxa::BufferHandle indiexBuffer = {};
	daxa::BufferHandle vertexPositions = {};
	daxa::BufferHandle vertexUVs = {};
	daxa::BufferHandle vertexNormals = {};
	daxa::BufferHandle vertexTangents = {};
	daxa::ImageViewHandle albedoMap = {};
	daxa::ImageViewHandle normalMap = {};
};

struct DrawPrimCmd {
	glm::mat4 transform = {};
	Primitive* prim = {};
};