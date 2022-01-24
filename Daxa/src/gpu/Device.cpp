#include "Device.hpp"

#include <iostream>
#include <algorithm>
#include <chrono>

#include <VkBootstrap.h>

#include "Instance.hpp"

namespace daxa {
	namespace gpu {

		DeviceHandle Device::create() {
			return DeviceHandle{ std::make_shared<Device>(gpu::instance->getVKBInstance()) };
		}

		Device::Device(vkb::Instance& instance) {
			this->backend = std::make_shared<DeviceBackend>(instance);
			this->bindingSetDescriptionCache = std::make_unique<BindingSetDescriptionCache>(backend->device.device);
			
			this->stagingBufferPool = std::make_shared<StagingBufferPool>(StagingBufferPool{ 
				backend->device.device, 
				&backend->graveyard, 
				std::span<u32>{ backend->allQFamilyIndices.data(), backend->allQFamilyIndices.size() }, 
				backend->allocator
			});
			this->cmdListRecyclingSharedData = std::make_shared<CommandListRecyclingSharedData>();
		}

		QueueHandle Device::createQueue(QueueCreateInfo const& ci) {
			return QueueHandle{ std::make_shared<Queue>(backend->device.device, backend->device.get_queue(vkb::QueueType::graphics).value(), ci) };
		}

		SamplerHandle Device::createSampler(SamplerCreateInfo ci) {
			return SamplerHandle{ std::make_shared<Sampler>(backend->device.device, &backend->graveyard, ci) };
		}

		ImageHandle Device::createImage2d(Image2dCreateInfo ci) { 
			auto handle = ImageHandle{ std::make_shared<Image>() };
			Image::construct2dImage(backend->device.device, &backend->graveyard, backend->allocator, backend->graphicsQFamilyIndex, ci, *handle);
			return std::move(handle);
		}

		BufferHandle Device::createBuffer(BufferCreateInfo ci) {
			return BufferHandle{ std::make_shared<Buffer>(backend->device.device, &backend->graveyard, backend->allocator, std::span<u32>{ backend->allQFamilyIndices.data(), backend->allQFamilyIndices.size() }, ci) };
		}

		TimelineSemaphoreHandle Device::createTimelineSemaphore(TimelineSemaphoreCreateInfo const& ci) {
			return TimelineSemaphoreHandle{ std::make_shared<TimelineSemaphore>(backend->device.device, ci) };
		}

		SignalHandle Device::createSignal(SignalCreateInfo const& ci) {
			return SignalHandle{ std::make_shared<Signal>(backend->device.device, ci) };
		}

		SwapchainHandle Device::createSwapchain(SwapchainCreateInfo ci) {
			auto handle = SwapchainHandle{ std::make_shared<Swapchain>() };
			handle->construct(backend->device.device, &backend->graveyard, backend->device.physical_device, gpu::instance->getVkInstance(), ci);
			return std::move(handle);
		}

		BindingSetDescription const* Device::createBindingSetDescription(std::span<VkDescriptorSetLayoutBinding> bindings) {
			return bindingSetDescriptionCache->getSetDescription(bindings);
		}

		CommandListHandle Device::getCommandList(char const* debugName) {
			auto list = std::move(getNextCommandList());
			list->setDebugName(debugName);
			{
				std::unique_lock lock(backend->graveyard.mtx);
				backend->graveyard.activeZombieLists.push_back(list->zombies);
			}
			return std::move(list);
		}

		void Device::waitIdle() {
			vkDeviceWaitIdle(backend->device.device);
		}

		CommandListHandle Device::getNextCommandList() {
			if (unusedCommandLists.empty()) {
				auto lock = std::unique_lock(cmdListRecyclingSharedData->mut);
				while (!cmdListRecyclingSharedData->zombies.empty()) {
					unusedCommandLists.push_back(CommandListHandle{std::move(cmdListRecyclingSharedData->zombies.back())});
					cmdListRecyclingSharedData->zombies.pop_back();
				}
			}

			if (unusedCommandLists.empty()) {
				// we have no command lists left, we need to create new ones:
				unusedCommandLists.push_back(CommandListHandle{ std::make_shared<CommandList>() });
				CommandList& list = *unusedCommandLists.back();
				list.device = backend->device.device;
				list.vkCmdBeginRenderingKHR = backend->vkCmdBeginRenderingKHR;
				list.vkCmdEndRenderingKHR = backend->vkCmdEndRenderingKHR;
				list.vkCmdPipelineBarrier2KHR = backend->vkCmdPipelineBarrier2KHR;
				list.stagingBufferPool = stagingBufferPool;
				list.recyclingData = cmdListRecyclingSharedData;
				list.graveyard = &backend->graveyard;

				VkCommandPoolCreateInfo commandPoolCI{
					.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
					.pNext = nullptr,
					.queueFamilyIndex = backend->graphicsQFamilyIndex,
				};
				vkCreateCommandPool(backend->device.device, &commandPoolCI, nullptr, &list.cmdPool);

				VkCommandBufferAllocateInfo commandBufferAllocateInfo{
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
					.pNext = nullptr,
					.commandPool = list.cmdPool,
					.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
					.commandBufferCount = 1,
				};
				vkAllocateCommandBuffers(backend->device.device, &commandBufferAllocateInfo, &list.cmd);
				list.begin();
			} 
			auto ret = std::move(unusedCommandLists.back());
			unusedCommandLists.pop_back();
			return std::move(ret);
		}

		daxa::Result<PipelineHandle> Device::createGraphicsPipeline(GraphicsPipelineBuilder& pipelineBuilder) {
			return pipelineBuilder.build(backend->device.device, *bindingSetDescriptionCache);
		}

		daxa::Result<PipelineHandle> Device::createComputePipeline(ComputePipelineCreateInfo const& ci) {
			return gpu::createComputePipeline(backend->device.device, *bindingSetDescriptionCache, ci);
		}

		BindingSetAllocatorHandle Device::createBindingSetAllocator(BindingSetAllocatorCreateInfo const& ci) {
			return BindingSetAllocatorHandle{ std::make_shared<BindingSetAllocator>(backend->device.device, ci) };
		}

		Result<ShaderModuleHandle> Device::createShaderModule(ShaderModuleCreateInfo const& ci) {
			return ShaderModuleHandle::tryCreateDAXAShaderModule(backend->device.device, ci);
		}
	}
}