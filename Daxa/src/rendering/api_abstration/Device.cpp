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
			ret->initFrameContexts();
			return std::move(ret);
		}

		Device::~Device() {
			for (int i = 0; i < 3; i++) {
				nextFrameContext();
			}
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

		void Device::submit(CommandList&& cmdList, SubmitInfo const& submitInfo) {
			CommandList list{ std::move(cmdList) };
			assert(list.operationsInProgress == 0);
			auto& frame = *this->frameContexts.front();

			auto thisSubmitFence = getNextFence();

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
			vkQueueSubmit(graphicsQ, 1, &si, thisSubmitFence);

			frame.usedCommandLists.push_back(std::move(list));
		}

		void Device::submit(std::vector<CommandList>& cmdLists, SubmitInfo const& submitInfo) {
			auto& frame = *this->frameContexts.front();
			auto thisSubmitFence = getNextFence();

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
			vkQueueSubmit(graphicsQ, 1, &si, thisSubmitFence);

			while (!cmdLists.empty()) {
				auto list = std::move(cmdLists.back());
				cmdLists.pop_back();
				frame.usedCommandLists.push_back(std::move(list));
			}
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

		void Device::nextFrameContext() {
			// get new frame context
			auto lastFrame = std::move(frameContexts.back());
			frameContexts.pop_back();
			frameContexts.push_front(std::move(lastFrame));

			auto& frame = *frameContexts.front();

			if (!frame.usedFences.empty()) {
				vkWaitForFences(device, frame.usedFences.size(), frame.usedFences.data(), VK_TRUE, UINT64_MAX);
				vkResetFences(device, frame.usedFences.size(), frame.usedFences.data());
			}
			while (!frame.usedFences.empty()) {
				auto back = frame.usedFences.back();
				frame.usedFences.pop_back();
				unusedFences.push_back(back);
			}
			// we reset the command buffers individually as there is generally no big performance cost to that
			//device.resetCommandPool(*frame.cmdPool, VkCommandPoolResetFlagBits::eReleaseResources);
			// reset and recycle CommandLists
			while (!frame.usedCommandLists.empty()) {
				auto list = std::move(frame.usedCommandLists.back());
				frame.usedCommandLists.pop_back();
				list.reset();
				unusedCommandLists.push_back(std::move(list));
			}
		}

		void Device::waitIdle() {
			for (int i = 0; i < 3; i++) {
				nextFrameContext();
			}
			vkDeviceWaitIdle(device);
		}

		void Device::initFrameContexts() {
			// The amount of contexts is currently hardcoded to two wich is fine for now.
			// A later extention to adapt the amount of contexts may be a nice feature.
			for (int i = 0; i < 3; i++) {
				auto fc = std::make_unique<FrameContext>();
				this->frameContexts.push_back(std::move(fc));
			}
		}

		CommandList Device::getNextCommandList() {
			auto& frame = *frameContexts.front();
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

		VkSemaphore Device::getNextSemaphore() {
			auto& frame = *frameContexts.front();
			if (unusedSemaphores.empty()) {
				unusedSemaphores.push_back({});

				VkSemaphoreCreateInfo semaphoreCreateInfo{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
					.pNext = nullptr,
				};
				vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &unusedSemaphores.back());
			}
			auto ret = unusedSemaphores.back();
			unusedSemaphores.pop_back();
			frame.usedSemaphores.push_back(ret);
			return ret;
		}

		VkFence Device::getNextFence() {
			auto& frame = *frameContexts.front();
			if (unusedFences.empty()) {
				unusedFences.push_back({});

				VkFenceCreateInfo fenceCreateInfo{
					.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
					.pNext = nullptr,
				};
				vkCreateFence(device, &fenceCreateInfo, nullptr, &unusedFences.back());
			}
			auto ret = unusedFences.back();
			unusedFences.pop_back();
			frame.usedFences.push_back(ret);
			return ret;
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