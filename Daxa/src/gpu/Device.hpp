#pragma once

#include "../DaxaCore.hpp"

#include <deque>
#include <memory>
#include <span>

#include <vulkan/vulkan.h>

#include <VkBootstrap.h>

#include "Handle.hpp"
#include "CommandList.hpp"
#include "Image.hpp"
#include "ShaderModule.hpp"
#include "Pipeline.hpp"
#include "SwapchainImage.hpp"
#include "TimelineSemaphore.hpp"
#include "Swapchain.hpp"
#include "Signal.hpp"
#include "StagingBufferPool.hpp"
#include "CommandQueue.hpp"
#include "PipelineCompiler.hpp"
#include "GPUHandles.hpp"

namespace daxa {
	class Instance;
	class DeviceHandle;

	class Device {
	public:
		Device(Instance& instance);
		Device(Device const& other)				= delete;
		Device& operator=(Device const&)		= delete;
		Device(Device&&) noexcept				= delete;
		Device& operator=(Device&&) noexcept	= delete;

		static DeviceHandle create();

		ImageHandle createImage(ImageCreateInfo const& info);

		BufferHandle createBuffer(BufferInfo const& info);
		ImageViewHandle createImageView(ImageViewInfo const& info);
		SamplerHandle createSampler(SamplerInfo const& info);

		void destroyBuffer(BufferHandle handle);
		void destroyImageView(ImageViewHandle handle);
		void destroySampler(SamplerHandle handle);

		BufferInfo const& info(BufferHandle handle);
		ImageViewInfo const& info(ImageViewHandle handle);
		SamplerInfo const& info(SamplerHandle handle);

		MappedMemory mapMemory(BufferHandle handle);

		CommandQueueHandle createCommandQueue(CommandQueueCreateInfo const& ci);

		TimelineSemaphoreHandle createTimelineSemaphore(TimelineSemaphoreCreateInfo const& ci);

		SignalHandle createSignal(SignalCreateInfo const& ci);

		SwapchainHandle createSwapchain(SwapchainCreateInfo swapchainCI);

		PipelineCompilerHandle createPipelineCompiler();

		/**
		 * Waits for the device to complete all submitted operations and the gpu to idle.
		 */
		void waitIdle();

		VkPhysicalDevice getVkPhysicalDevice() const;
		VkDevice getVkDevice() const;
		u32 getVkGraphicsQueueFamilyIndex() const;
	private:
		friend class Instance;

		std::shared_ptr<void> 				backend 					= {};
		std::shared_ptr<StagingBufferPool> 	uploadStagingBufferPool 	= {};
		std::shared_ptr<StagingBufferPool> 	downloadStagingBufferPool 	= {};
	};

	class DeviceHandle : public SharedHandle<Device>{};
}