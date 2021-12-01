#include "Device.hpp"

#include <iostream>
#include <algorithm>

#include <VkBootstrap.hpp>
#include "../dependencies/vulkanhelper.hpp"

namespace daxa {
	namespace gpu {
		std::shared_ptr<vkb::Instance> instance;
		bool initialized = false;

		void initGlobals() {
			vkb::InstanceBuilder instanceBuilder;
			instanceBuilder
				.set_app_name("Daxa Application")
				.set_engine_name("Daxa")
				.request_validation_layers(true)
				.require_api_version(1, 2, 0)
				.enable_layer("VK_LAYER_LUNARG_monitor")
				.use_default_debug_messenger();
			auto instanceBuildReturn = instanceBuilder.build();

			instance = std::make_shared<vkb::Instance>(std::move(instanceBuildReturn.value()));
		}


		std::shared_ptr<vkb::Instance> Device::getInstance() {
			if (!initialized) {
				initGlobals();
				initialized = true;
			}
			return instance;
		}

		std::shared_ptr<Device> Device::createNewDevice() {
			if (!initialized) {
				initGlobals();
				initialized = true;
			}
			vkb::PhysicalDeviceSelector selector{ *instance };
			vkb::PhysicalDevice physicalDevice = selector
				.set_minimum_version(1, 2)
				.defer_surface_initialization()
				.require_separate_compute_queue()
				.require_separate_transfer_queue()
				.add_desired_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
				.select()
				.value();

			auto physicslDevice = physicalDevice.physical_device;

			VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_feature{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
				.pNext = nullptr,
				.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
				.descriptorBindingPartiallyBound = VK_TRUE,
				.descriptorBindingVariableDescriptorCount = VK_TRUE,
				.runtimeDescriptorArray = VK_TRUE,
			};

			VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
				.pNext = nullptr,
				.dynamicRendering = VK_TRUE,
			};

			vkb::DeviceBuilder deviceBuilder{ physicalDevice };
			deviceBuilder.add_pNext(&descriptor_indexing_feature);
			deviceBuilder.add_pNext(&dynamic_rendering_feature);

			vkb::Device vkbDevice = deviceBuilder.build().value();

			auto device = vkbDevice.device;

			auto mainGraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
			auto mainGraphicsQueueFamiltyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

			VmaAllocatorCreateInfo allocatorInfo = {
				.physicalDevice = physicslDevice,
				.device = device,
				.instance = instance->instance,
			};
			VmaAllocator allocator;
			vmaCreateAllocator(&allocatorInfo, &allocator);

			auto fnPtrvkCmdBeginRenderingKHR = (void(*)(VkCommandBuffer, const VkRenderingInfoKHR*))vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR");
			auto fnPtrvkCmdEndRenderingKHR = (void(*)(VkCommandBuffer))vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR");
			assert(fnPtrvkCmdBeginRenderingKHR != nullptr && fnPtrvkCmdEndRenderingKHR != nullptr, "ERROR: could not load VK_KHR_DYNAMIC_RENDERING_EXTENSION");

			std::shared_ptr<Device> ret = std::make_shared<Device>();
			ret->device = device;
			ret->descriptorLayoutCache = { device };
			ret->physicalDevice = physicalDevice.physical_device;
			ret->graphicsQ = mainGraphicsQueue;
			ret->graphicsQFamilyIndex = mainGraphicsQueueFamiltyIndex;
			ret->allocator = allocator;
			ret->vkCmdBeginRenderingKHR = fnPtrvkCmdBeginRenderingKHR;
			ret->vkCmdEndRenderingKHR = fnPtrvkCmdEndRenderingKHR;
			return std::move(ret);
		}

		Device::~Device() {
			waitIdle();
			for (auto& fence : unusedFences) {
				fence.disableRecycling();
			}
			unusedFences.clear();
		}

		ImageHandle Device::createImage2d(Image2dCreateInfo ci) { 
			return ImageHandle{ std::make_shared<Image>(std::move(Image::create2dImage(device, allocator, ci))) };
		}

		BufferHandle Device::createBuffer(BufferCreateInfo ci) {
			return BufferHandle{ std::make_shared<Buffer>(std::move(Buffer(device, graphicsQFamilyIndex, allocator, ci))) };
		}

		CommandList Device::getEmptyCommandList() {
			return std::move(getNextCommandList());
		}

		FenceHandle Device::submit(CommandList&& cmdList, SubmitInfo const& submitInfo) {
			CommandList list{ std::move(cmdList) };
			assert(list.operationsInProgress == 0);

			auto thisSubmitFence = getNextFenceHandle();

			VkPipelineStageFlags pipelineStages = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			VkCommandBuffer commandBuffers[] = { list.cmd };
			VkSubmitInfo si{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.pNext = nullptr,
				.waitSemaphoreCount = (u32)submitInfo.waitOnSemaphores.size(),
				.pWaitSemaphores = (VkSemaphore*)submitInfo.waitOnSemaphores.data(),
				.pWaitDstStageMask = &pipelineStages,
				.commandBufferCount = 1,
				.pCommandBuffers = commandBuffers,
				.signalSemaphoreCount = (u32)submitInfo.signalSemaphores.size(),
				.pSignalSemaphores = (VkSemaphore*)submitInfo.signalSemaphores.data(),
			};
			vkQueueSubmit(graphicsQ, 1, &si, thisSubmitFence.fence->fence);

			PendingSubmit pendingSubmit{
				.fence = thisSubmitFence,
			};
			pendingSubmit.cmdLists.push_back(std::move(list));
			unfinishedSubmits.push_back(std::move(pendingSubmit));

			return thisSubmitFence;
		}

		FenceHandle Device::submit(std::vector<CommandList>& cmdLists, SubmitInfo const& submitInfo) {
			auto thisSubmitFence = getNextFenceHandle();

			submitCommandBufferBuffer.clear();
			for (auto& cmdList : cmdLists) {
				assert(cmdList.operationsInProgress == 0);
				submitCommandBufferBuffer.push_back(cmdList.cmd);
			}

			VkPipelineStageFlags pipelineStages = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			VkSubmitInfo si{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.pNext = nullptr,
				.waitSemaphoreCount = (u32)submitInfo.waitOnSemaphores.size(),
				.pWaitSemaphores = (VkSemaphore*)submitInfo.waitOnSemaphores.data(),
				.pWaitDstStageMask = &pipelineStages,
				.commandBufferCount = static_cast<u32>(submitCommandBufferBuffer.size()),
				.pCommandBuffers = submitCommandBufferBuffer.data(),
				.signalSemaphoreCount = (u32)submitInfo.signalSemaphores.size(),
				.pSignalSemaphores = (VkSemaphore*)submitInfo.signalSemaphores.data(),
			};
			vkQueueSubmit(graphicsQ, 1, &si, thisSubmitFence.fence->fence);

			PendingSubmit pendingSubmit{
				.fence = thisSubmitFence
			};
			while (!cmdLists.empty()) {
				auto list = std::move(cmdLists.back());
				cmdLists.pop_back();
				pendingSubmit.cmdLists.push_back(std::move(list));
			}
			unfinishedSubmits.push_back(std::move(pendingSubmit));

			return thisSubmitFence;
		}

		void Device::present(SwapchainImage const& sImage, std::span<VkSemaphore> waitOn) {
			VkPresentInfoKHR presentInfo{
				.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.pNext = nullptr,
				.waitSemaphoreCount = static_cast<u32>(waitOn.size()),
				.pWaitSemaphores = (VkSemaphore*)waitOn.data(),
				.swapchainCount = 1,
				.pSwapchains = &sImage.swapchain,
				.pImageIndices = &sImage.imageIndex,
			};
			vkQueuePresentKHR(graphicsQ, &presentInfo);
		}

		void Device::recycle() {
			for (auto iter = unfinishedSubmits.begin(); iter != unfinishedSubmits.end();) {
				if (iter->fence.checkStatus() == VK_SUCCESS) {
					while (!iter->cmdLists.empty()) {
						auto list = std::move(iter->cmdLists.back());
						iter->cmdLists.pop_back();
						list.reset();
						unusedCommandLists.push_back(std::move(list));
					}
					iter->fence.reset();
					iter = unfinishedSubmits.erase(iter);
				}
				else {
					++iter;
				}
			}
		}

		void Device::waitIdle() {
			for (auto iter = unfinishedSubmits.begin(); iter != unfinishedSubmits.end(); ++iter) {
				iter->fence.wait(UINT64_MAX);
			}
			vkDeviceWaitIdle(device);
		}

		CommandList Device::getNextCommandList() {
			if (unusedCommandLists.empty()) {
				// we have no command lists left, we need to create new ones:
				CommandList list;
				list.device = device;
				list.vkCmdBeginRenderingKHR = this->vkCmdBeginRenderingKHR;
				list.vkCmdEndRenderingKHR = this->vkCmdEndRenderingKHR;

				VkCommandPoolCreateInfo commandPoolCI{
					.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
					.pNext = nullptr,
					.queueFamilyIndex = graphicsQFamilyIndex,
				};
				vkCreateCommandPool(device, &commandPoolCI, nullptr, &list.cmdPool);

				VkCommandBufferAllocateInfo commandBufferAllocateInfo{
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
					.pNext = nullptr,
					.commandPool = list.cmdPool,
					.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
					.commandBufferCount = 1,
				};
				vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &list.cmd);

				unusedCommandLists.push_back(std::move(list));
			} 
			auto ret = std::move(unusedCommandLists.back());
			unusedCommandLists.pop_back();
			return std::move(ret);
		}

		FenceHandle Device::getNextFenceHandle() {
			if (unusedFences.empty()) {
				unusedFences.push_back(Fence{ device, &unusedFences });
			}

			auto fence = std::move(unusedFences.back());
			unusedFences.pop_back();
			return FenceHandle{ std::move(fence) };
		}

		std::optional<ShaderModuleHandle> Device::tryCreateShderModuleFromGLSL(std::string const& glslSource, VkShaderStageFlagBits stage, std::string const& entrypoint) {
			return ShaderModuleHandle::tryCreateDAXAShaderModule(device, glslSource, entrypoint, stage);
		}

		std::optional<ShaderModuleHandle> Device::tryCreateShderModuleFromGLSL(std::filesystem::path const& pathToGlsl, VkShaderStageFlagBits stage, std::string const& entrypoint) {
			return ShaderModuleHandle::tryCreateDAXAShaderModule(device, pathToGlsl, entrypoint, stage);
		}

		GraphicsPipelineHandle Device::createGraphicsPipeline(GraphicsPipelineBuilder& pipelineBuilder) {
			return pipelineBuilder.build(device, *descriptorLayoutCache);
		}
	}
}