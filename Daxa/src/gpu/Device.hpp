#pragma once

#include "../DaxaCore.hpp"

#include <deque>
#include <memory>
#include <span>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>
#include <VkBootstrap.h>

#include "DeviceBackend.hpp"
#include "Handle.hpp"
#include "Graveyard.hpp"
#include "CommandList.hpp"
#include "Image.hpp"
#include "Buffer.hpp"
#include "ShaderModule.hpp"
#include "Pipeline.hpp"
#include "SwapchainImage.hpp"
#include "TimelineSemaphore.hpp"
#include "Swapchain.hpp"
#include "Signal.hpp"
#include "StagingBufferPool.hpp"
#include "CommandQueue.hpp"
#include "PipelineCompiler.hpp"

namespace daxa {
	namespace gpu {

		class Instance;

		class DeviceHandle;

		class Device {
		public:
			Device(Instance& instance);
			Device(Device const& other)				= delete;
			Device& operator=(Device const&)		= delete;
			Device(Device&&) noexcept				= delete;
			Device& operator=(Device&&) noexcept	= delete;

			CommandQueueHandle createCommandQueue(CommandQueueCreateInfo const& ci);

			static DeviceHandle create();

			SamplerHandle createSampler(SamplerCreateInfo ci);

			ImageHandle createImage(ImageCreateInfo const& ci);

			ImageViewHandle createImageView(ImageViewCreateInfo const& ci);

			/**
			 * \param ci all information defining the buffer
			 * \return a reference counted buffer handle of the created buffer ressource.
			 */
			BufferHandle createBuffer(BufferCreateInfo ci);

			TimelineSemaphoreHandle createTimelineSemaphore(TimelineSemaphoreCreateInfo const& ci);

			SignalHandle createSignal(SignalCreateInfo const& ci);

			SwapchainHandle createSwapchain(SwapchainCreateInfo swapchainCI);

			BindingSetLayout const& getBindingSetLayout(BindingSetDescription const& description);
			
			std::shared_ptr<BindingSetLayout const> getBindingSetLayoutShared(BindingSetDescription const& description);

			Result<ShaderModuleHandle> createShaderModule(ShaderModuleCreateInfo const& ci);

			daxa::Result<PipelineHandle> createComputePipeline(ComputePipelineCreateInfo const& ci);

			BindingSetAllocatorHandle createBindingSetAllocator(BindingSetAllocatorCreateInfo const& ci);

			PipelineCompilerHandle createPipelineCompiler();

			/**
			 * Waits for the device to complete all submitted operations and the gpu to idle.
			 */
			void waitIdle();

			const VkPhysicalDevice getVkPhysicalDevice() const { return backend->device.physical_device; }
			const VkDevice getVkDevice() const { return backend->device.device; }
			const VmaAllocator getVma() const { return backend->allocator; }
			const u32 getVkGraphicsQueueFamilyIndex() const { return backend->graphicsQFamilyIndex; }
		private:
			friend class Instance;

			CommandListHandle getNextCommandList();

			shaderc::Compiler 						shaderCompiler 				= {};
			shaderc::CompileOptions 				shaderCompileOptions 		= {};
			std::shared_ptr<DeviceBackend> 			backend 					= {};
			std::shared_ptr<StagingBufferPool> 		stagingBufferPool 			= {};
			std::shared_ptr<BindingSetLayoutCache>	bindingSetDescriptionCache 	= {};
		};

		class DeviceHandle : public SharedHandle<Device>{};
	}
}