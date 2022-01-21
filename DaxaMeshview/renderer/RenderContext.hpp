#pragma once

#include "Daxa.hpp"

class RenderContext {
public:
    RenderContext(daxa::Window& window) 
		: device{ daxa::gpu::Device::create() }
		, queue{ this->device->createQueue({}) }
		, swapchain{ this->device->createSwapchain({
			.surface = window.getSurface(),
			.width = window.getWidth(),
			.height = window.getHeight(),
			.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
		})}
		, swapchainImage{ this->swapchain->aquireNextImage() }
		, presentSignal{ this->device->createSignal({}) }
    {
		defaultSampler = device->createSampler({});
		auto cmd = device->getCommandList();
		recreateFramebuffer(cmd, window.getWidth(), window.getHeight());
		cmd->finalize();
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
			.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			.imageAspekt = VK_IMAGE_ASPECT_DEPTH_BIT,
			.sampler = defaultSampler,
			.debugName = "depth image",
		});

		this->normalsImage = device->createImage2d({
			.width = width,
			.height = height,
			.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			.imageAspekt = VK_IMAGE_ASPECT_COLOR_BIT,
			.sampler = defaultSampler,
			.debugName = "normals image",
		});

		cmd->insertImageBarriers(std::array{
			daxa::gpu::ImageBarrier{
				.image = depthImage,
				.layoutAfter = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			},
			daxa::gpu::ImageBarrier{
				.image = normalsImage,
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
	daxa::gpu::QueueHandle queue = {};
	daxa::gpu::SwapchainHandle swapchain = {};
	daxa::gpu::SwapchainImage swapchainImage = {};
	daxa::gpu::SignalHandle presentSignal = {};
	daxa::gpu::ImageHandle depthImage = {};
	daxa::gpu::ImageHandle normalsImage = {};
	daxa::gpu::SamplerHandle defaultSampler = {};
};