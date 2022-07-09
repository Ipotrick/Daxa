#include "Device.hpp"

#include <iostream>
#include <algorithm>
#include <chrono>

#include <VkBootstrap.h>

#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"
#include "backend/BufferBackend.hpp"
#include "backend/ImageViewBackend.hpp"

namespace daxa {

	VkPhysicalDevice Device::getVkPhysicalDevice() const { 
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		return deviceBackend->device.physical_device; 
	}

	VkDevice Device::getVkDevice() const { 
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		return deviceBackend->device.device;
	}

	u32 Device::getVkGraphicsQueueFamilyIndex() const { 
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		return deviceBackend->graphicsQFamilyIndex;
	}

	DeviceHandle Device::create() {
		return DeviceHandle{ std::make_shared<Device>(*instance) };
	}

	ImageHandle Device::createImage(ImageCreateInfo const& info) {
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		return ImageHandle{ std::make_shared<Image>(deviceBackend, info) };
	}

	BufferHandle Device::createBuffer(BufferInfo const& info) {
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		return deviceBackend->createBuffer(info);
	}

	ImageViewHandle Device::createImageView(ImageViewInfo const& info) {
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		return deviceBackend->createImageView(info);
	}

	SamplerHandle Device::createSampler(SamplerInfo const& info) {
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		return deviceBackend->createSampler(info);
	}

	void Device::destroyBuffer(BufferHandle handle) {
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		deviceBackend->gpuHandleGraveyard.zombifyBuffer(deviceBackend->gpuRessources, handle);
	}

	void Device::destroyImageView(ImageViewHandle handle) {
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		deviceBackend->gpuHandleGraveyard.zombifyImageView(deviceBackend->gpuRessources, handle);
	}

	void Device::destroySampler(SamplerHandle handle) {
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		deviceBackend->gpuHandleGraveyard.zombifySampler(deviceBackend->gpuRessources, handle);
	}

	BufferInfo const& Device::info(BufferHandle handle) {
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		return deviceBackend->gpuRessources.buffers.get(handle).info;
	}

	ImageViewInfo const& Device::info(ImageViewHandle handle) {
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		return deviceBackend->gpuRessources.imageViews.get(handle).info;
	}

	SamplerInfo const& Device::info(SamplerHandle handle) {
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		return deviceBackend->gpuRessources.samplers.get(handle).info;
	}

	
	MappedMemory Device::mapMemory(BufferHandle handle) {
		return std::move(MappedMemory{ backend, handle, this->info(handle).size, 0 });
	}

	Device::Device(Instance& instance) 
		: backend{ std::make_shared<DeviceBackend>(instance.getVKBInstance(), instance.pfnSetDebugUtilsObjectNameEXT) }
		, uploadStagingBufferPool{ std::make_shared<StagingBufferPool>(backend) }
		, downloadStagingBufferPool{ std::make_shared<StagingBufferPool>(backend, 1 << 14/*, VK_BUFFER_USAGE_TRANSFER_DST_BIT*/, MemoryType::GPU_TO_CPU) }
	{ }

	CommandQueueHandle Device::createCommandQueue(CommandQueueCreateInfo const& ci) {
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		return CommandQueueHandle{ std::make_shared<CommandQueue>(
			backend, 
			deviceBackend->device.get_queue(vkb::QueueType::graphics).value(), 
			deviceBackend->graphicsQFamilyIndex, 
			uploadStagingBufferPool, 
			downloadStagingBufferPool, 
			ci
		)};
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

	void Device::waitIdle() {		
		std::shared_ptr<DeviceBackend> deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		vkDeviceWaitIdle(deviceBackend->device.device);
	}

	PipelineCompilerHandle Device::createPipelineCompiler() {
		return PipelineCompilerHandle{ std::make_shared<PipelineCompiler>(backend) };
	}
}