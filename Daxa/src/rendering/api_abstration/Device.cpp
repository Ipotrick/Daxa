#include "Device.hpp"

#include <iostream>
#include <algorithm>

#include <VkBootstrap.hpp>

namespace gpu {
	vkb::Instance instance;
	vk::DebugUtilsMessengerEXT debugMessenger;
	bool initialized = false;

	void initGlobals() {
		vkb::InstanceBuilder instanceBuilder;
		instanceBuilder
			.set_app_name("Daxa Application")
			.set_engine_name("Daxa")
			.request_validation_layers(true)
			.require_api_version(1, 2, 0)
			.use_default_debug_messenger();
		auto instanceBuildReturn = instanceBuilder.build();
		if (!instanceBuildReturn) {
			std::cerr << "could not create vulkan instance!\n";
		}

		instance = instanceBuildReturn.value();
		debugMessenger = instance.debug_messenger;
	}

	Device Device::createNewDevice() {
		if (!initialized) {
			initGlobals();
			initialized = true;
		}
		vkb::PhysicalDeviceSelector selector{ instance };
		vkb::PhysicalDevice physicalDevice = selector
			.set_minimum_version(1, 2)
			.defer_surface_initialization()
			.require_separate_compute_queue()
			.require_separate_transfer_queue()
			.select()
			.value();

		auto physicslDevice = physicalDevice.physical_device;

		vk::PhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features{
			.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
			.descriptorBindingPartiallyBound = VK_TRUE,
			.descriptorBindingVariableDescriptorCount = VK_TRUE,
			.runtimeDescriptorArray = VK_TRUE,
		};

		vkb::DeviceBuilder deviceBuilder{ physicalDevice };
		deviceBuilder.add_pNext(&descriptor_indexing_features);

		vkb::Device vkbDevice = deviceBuilder.build().value();

		auto device = vkbDevice.device;

		auto mainGraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
		auto mainGraphicsQueueFamiltyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physicslDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance.instance;
		VmaAllocator allocator;
		vmaCreateAllocator(&allocatorInfo, &allocator);

		Device ret;
		ret.device = device;
		ret.physicalDevice = physicalDevice.physical_device;
		ret.graphicsQ = mainGraphicsQueue;
		ret.graphicsQFamilyIndex = mainGraphicsQueueFamiltyIndex;
		ret.allocator = allocator;
		ret.initFrameContexts();
		return ret;
	}

	ImageHandle Device::createImage2d(ImageCreateInfo ci) {
		ImageHandle ret;
		ret.image = std::make_shared<Image>();
	
		vk::ImageCreateInfo ici{};
		ici.imageType = vk::ImageType::e2D;
		ici.format = ci.format;
		ici.extent = vk::Extent3D{ ci.width, ci.height,1 };
		ici.mipLevels = 1;
		ici.arrayLayers = 1;
		ici.usage = ci.imageUsage;
		ici.tiling = vk::ImageTiling::eOptimal;
		ici.initialLayout = vk::ImageLayout::eUndefined;
		ici.sharingMode = vk::SharingMode::eExclusive;
		ici.samples = vk::SampleCountFlagBits::e1;

		VmaAllocationCreateInfo aci{};
		aci.usage = ci.memoryUsage;
		aci.requiredFlags = (VkMemoryPropertyFlags)ci.memoryPropertyFlags;

		vmaCreateImage(allocator, (VkImageCreateInfo*)&ici, &aci, (VkImage*)&*ret->image, &ret->allocation, nullptr);

		vk::ImageViewCreateInfo ivci{};
		ivci.image = ret->image;
		ivci.format = ci.format;
		ivci.subresourceRange.baseMipLevel = 0;
		ivci.subresourceRange.levelCount = 1;
		ivci.subresourceRange.baseArrayLayer = 0;
		ivci.subresourceRange.layerCount = 1;
		ivci.subresourceRange.aspectMask = ci.imageAspekt;

		ret->view = device.createImageViewUnique(ivci);

		ret.image->allocator = allocator;
		ret->createInfo = ici;
		ret->viewCreateInfo = ivci;

		return ret;
	}

	BufferHandle Device::createBuffer(BufferCreateInfo ci) {
		return BufferHandle{ std::make_shared<Buffer>(std::move(Buffer(device, graphicsQFamilyIndex, allocator, ci))) };
	}

	BufferHandle Device::createFramedBuffer(BufferCreateInfo ci) {
		auto bufferHandle = createBuffer(ci);
		this->frameContexts.front()->framedBuffers.push_back(bufferHandle);
		return bufferHandle;
	}

	CommandList Device::createFramedCommandList() {
		CommandList ret;

		vk::CommandBufferAllocateInfo cbai{};
		cbai.commandPool = *this->frameContexts.front()->cmdPool;
		cbai.level = vk::CommandBufferLevel::ePrimary;
		ret.cmd = device.allocateCommandBuffers(cbai).front();

		return ret;
	}

	void Device::submit(CommandList& cmdList) {
		auto prevSubmitSema = this->frameContexts.front()->usedSemaphores.back();
		auto thisSubmitSema = getNextSemaphore();
		auto thisSubmitFence = getNextFence();

		vk::SubmitInfo si{};

		vk::PipelineStageFlags pipelineStages = vk::PipelineStageFlagBits::eAllCommands;
		si.pWaitDstStageMask = &pipelineStages;

		vk::CommandBuffer commandBuffers[] = { cmdList.cmd };
		si.pCommandBuffers = commandBuffers;
		si.commandBufferCount = 1;

		vk::Semaphore previousSemaphores[] = { prevSubmitSema };
		si.pSignalSemaphores = previousSemaphores;
		si.signalSemaphoreCount = 1;

		graphicsQ.submit(si, thisSubmitFence);
	}

	void Device::nextFrameContext() {
		// get new frame context
		auto lastFrame = std::move(frameContexts.front());
		frameContexts.pop_front();
		frameContexts.push_back(std::move(lastFrame));

		// shortcut for frame
		auto& frame = *frameContexts.front();

		// wait on frame context to finish on gpu:
		if (!frame.usedFences.empty())
			device.waitForFences(frame.usedFences, VK_TRUE, 100000000);

		// reset frame ressources:
		if (!frame.usedFences.empty())
			device.resetFences(frame.usedFences);
		while (!frame.usedFences.empty()) {
			auto back = frame.usedFences.back();
			frame.usedFences.pop_back();
			frame.unusedFences.push_back(back);
		}
		device.resetCommandPool(*frame.cmdPool, vk::CommandPoolResetFlagBits::eReleaseResources);
		frame.framedBuffers.clear();
	}

	void Device::initFrameContexts() {
		for (int i = 0; i < 2; i++) {
			std::unique_ptr<FrameContext> fc = std::make_unique<FrameContext>();

			vk::CommandPoolCreateInfo cpci{};
			cpci.queueFamilyIndex = graphicsQFamilyIndex;
			cpci.flags |= vk::CommandPoolCreateFlagBits::eTransient;
			fc->cmdPool = device.createCommandPoolUnique(cpci);

			this->frameContexts.push_back(std::move(fc));
		}
	}

	vk::Semaphore Device::getNextSemaphore() {
		auto& frame = *frameContexts.front();
		if (frame.unusedSemaphores.empty()) {
			frame.unusedSemaphores.push_back(device.createSemaphore({}));
		}
		auto ret = frame.unusedSemaphores.back();
		frame.usedSemaphores.push_back(ret);
		frame.unusedSemaphores.pop_back();
		return ret;
	}

	vk::Fence Device::getNextFence() {
		auto& frame = *frameContexts.front();
		if (frame.unusedFences.empty()) {
			frame.unusedFences.push_back(device.createFence({}));
		}
		auto ret = this->frameContexts.front()->unusedFences.back();
		frame.usedFences.push_back(ret);
		frame.unusedFences.pop_back();
		return ret;
	}
}
