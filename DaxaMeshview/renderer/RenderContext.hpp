#pragma once

#include "Daxa.hpp"

class RenderContext {
public:
    RenderContext(daxa::Window& window) 
		: device{ daxa::gpu::Device::create() }
		, queue{ this->device->createQueue() }
		, swapchain{ this->device->createSwapchain({
			.surface = window.getSurface(),
			.width = window.getWidth(),
			.height = window.getHeight(),
			.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
		})}
		, swapchainImage{ this->swapchain->aquireNextImage() }
		, presentSignal{ this->device->createSignal() }
    {
		auto cmd = device->getEmptyCommandList();
		cmd->begin();
		recreateFramebuffer(cmd, window.getWidth(), window.getHeight());
		cmd->end();
		queue->submitBlocking({.commandLists = {cmd}});
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
		this->depthImage = device->createImage2d({
			.width = width,
			.height = height,
			.format = VK_FORMAT_D32_SFLOAT,
			.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			.imageAspekt = VK_IMAGE_ASPECT_DEPTH_BIT,
		});

		this->depthImageCopy = device->createImage2d({
			.width = width,
			.height = height,
			.format = VK_FORMAT_D32_SFLOAT,
			.imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.imageAspekt = VK_IMAGE_ASPECT_DEPTH_BIT,
			.sampler = device->createSampler({}),
		});

		this->normalsBuffer = device->createImage2d({
			.width = width,
			.height = height,
			.format = VK_FORMAT_R16G16_SFLOAT,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			.imageAspekt = VK_IMAGE_ASPECT_COLOR_BIT,
		});

		this->normalsBufferCopy = device->createImage2d({
			.width = width,
			.height = height,
			.format = VK_FORMAT_R16G16_SFLOAT,
			.imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.imageAspekt = VK_IMAGE_ASPECT_COLOR_BIT,
			.sampler = device->createSampler({}),
		});

		cmd->insertImageBarriers(std::array{
			daxa::gpu::ImageBarrier{
				.image = depthImage,
				.layoutAfter = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			},
			daxa::gpu::ImageBarrier{
				.image = depthImageCopy,
				.layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			},
			daxa::gpu::ImageBarrier{
				.image = normalsBuffer,
				.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			},
			daxa::gpu::ImageBarrier{
				.image = normalsBufferCopy,
				.layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
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
	daxa::gpu::QueueHandle queue = {};
	daxa::gpu::SwapchainHandle swapchain = {};
	daxa::gpu::SwapchainImage swapchainImage = {};
	daxa::gpu::SignalHandle presentSignal = {};
	daxa::gpu::ImageHandle depthImage = {};
	daxa::gpu::ImageHandle depthImageCopy = {};
	daxa::gpu::ImageHandle normalsBuffer = {};
	daxa::gpu::ImageHandle normalsBufferCopy = {};
};