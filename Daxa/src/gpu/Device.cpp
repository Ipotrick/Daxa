#include "Device.hpp"

#include <iostream>
#include <algorithm>
#include <chrono>

#include <VkBootstrap.h>

#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"
#include "backend/BufferBackend.hpp"

namespace daxa {

	VkPhysicalDevice Device::getVkPhysicalDevice() const { 
		return backend->device.physical_device; 
	}

	VkDevice Device::getVkDevice() const { 
		return backend->device.device;
	}

	u32 Device::getVkGraphicsQueueFamilyIndex() const { 
		return backend->graphicsQFamilyIndex;
	}

	DeviceHandle Device::create() {
		return DeviceHandle{ std::make_shared<Device>(*instance) };
	}

	Device::Device(Instance& instance) 
		: backend{ std::make_shared<DeviceBackend>(instance.getVKBInstance(), instance.pfnSetDebugUtilsObjectNameEXT) }
		, bindingSetDescriptionCache{ std::make_shared<BindingSetLayoutCache>(backend) }
		, uploadStagingBufferPool{ std::make_shared<StagingBufferPool>(backend) }
		, downloadStagingBufferPool{ std::make_shared<StagingBufferPool>(backend, 1 << 14/*, VK_BUFFER_USAGE_TRANSFER_DST_BIT*/, MemoryType::GPU_TO_CPU) }
	{ }

	CommandQueueHandle Device::createCommandQueue(CommandQueueCreateInfo const& ci) {
		return CommandQueueHandle{ std::make_shared<CommandQueue>(
			backend, 
			backend->device.get_queue(vkb::QueueType::graphics).value(), 
			backend->graphicsQFamilyIndex, 
			uploadStagingBufferPool, 
			downloadStagingBufferPool, 
			ci
		)};
	}

	SamplerHandle Device::createSampler(SamplerCreateInfo ci) {
		return SamplerHandle{ std::make_shared<Sampler>(backend, ci) };
	}

	ImageHandle Device::createImage(ImageCreateInfo const& ci) {
		return ImageHandle{ std::make_shared<Image>(backend, ci) };
	}

	ImageViewHandle Device::createImageView(ImageViewCreateInfo const& ci) {
		return ImageViewHandle{ std::make_shared<ImageView>(backend, ci) };
	}

	BufferHandle Device::createBuffer(BufferCreateInfo ci) {
		return BufferHandle{ std::make_shared<BufferBackend>(backend, ci) };
	}

	TimelineSemaphoreHandle Device::createTimelineSemaphore(TimelineSemaphoreCreateInfo const& ci) {
		return TimelineSemaphoreHandle{ std::make_shared<TimelineSemaphore>(backend, ci) };
	}

	SignalHandle Device::createSignal(SignalCreateInfo const& ci) {
		return SignalHandle{ std::make_shared<Signal>(backend, ci) };
	}

	SwapchainHandle Device::createSwapchain(SwapchainCreateInfo ci) {
		auto handle = SwapchainHandle{ std::make_shared<Swapchain>() };
		handle->construct(backend, ci);
		return std::move(handle);
	}

	BindingSetLayout const& Device::getBindingSetLayout(BindingSetDescription const& description) {
		return bindingSetDescriptionCache->getLayout(description);
	}
	std::shared_ptr<BindingSetLayout const> Device::getBindingSetLayoutShared(BindingSetDescription const& description) {
		return std::move(bindingSetDescriptionCache->getLayoutShared(description));
	}

	void Device::waitIdle() {
		vkDeviceWaitIdle(backend->device.device);
	}

	PipelineCompilerHandle Device::createPipelineCompiler() {
		return PipelineCompilerHandle{ std::make_shared<PipelineCompiler>(backend, bindingSetDescriptionCache) };
	}

	BindingSetAllocatorHandle Device::createBindingSetAllocator(BindingSetAllocatorCreateInfo const& ci) {
		return BindingSetAllocatorHandle{ std::make_shared<BindingSetAllocator>(backend, ci) };
	}
}