#pragma once

#include <Daxa/DaxaCore.hpp>

#include <deque>
#include <memory>
#include <span>

#include <vulkan/vulkan.h>

#include <Daxa/dependencies/VkBootstrap.h>

#include <Daxa/gpu/Handle.hpp>
#include <Daxa/gpu/Graveyard.hpp>
#include <Daxa/gpu/CommandList.hpp>
#include <Daxa/gpu/Image.hpp>
#include <Daxa/gpu/Buffer.hpp>
#include <Daxa/gpu/ShaderModule.hpp>
#include <Daxa/gpu/Pipeline.hpp>
#include <Daxa/gpu/SwapchainImage.hpp>
#include <Daxa/gpu/TimelineSemaphore.hpp>
#include <Daxa/gpu/Swapchain.hpp>
#include <Daxa/gpu/Signal.hpp>
#include <Daxa/gpu/StagingBufferPool.hpp>
#include <Daxa/gpu/CommandQueue.hpp>
#include <Daxa/gpu/PipelineCompiler.hpp>

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

		BindingSetAllocatorHandle createBindingSetAllocator(BindingSetAllocatorCreateInfo const& ci);

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

		CommandListHandle getNextCommandList();

		shaderc::Compiler 						shaderCompiler 				= {};
		shaderc::CompileOptions 				shaderCompileOptions 		= {};
		std::shared_ptr<DeviceBackend> 			backend 					= {};
		std::shared_ptr<StagingBufferPool> 		uploadStagingBufferPool 	= {};
		std::shared_ptr<StagingBufferPool> 		downloadStagingBufferPool 	= {};
		std::shared_ptr<BindingSetLayoutCache>	bindingSetDescriptionCache 	= {};
	};

	class DeviceHandle : public SharedHandle<Device>{};
}