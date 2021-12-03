#include "Device.hpp"

#include <iostream>
#include <algorithm>
#include <chrono>

#include <VkBootstrap.hpp>
#include "../dependencies/vulkanhelper.hpp"

#include "common.hpp"

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

			VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
				.pNext = nullptr,
				.timelineSemaphore = VK_TRUE,
			};

			vkb::DeviceBuilder deviceBuilder{ physicalDevice };
			deviceBuilder.add_pNext(&descriptor_indexing_feature);
			deviceBuilder.add_pNext(&dynamic_rendering_feature);
			deviceBuilder.add_pNext(&timeline_semaphore_features);

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

			auto ret = std::make_shared<Device>();
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
		}

		ImageHandle Device::createImage2d(Image2dCreateInfo ci) { 
			return ImageHandle{ std::make_shared<Image>(std::move(Image::create2dImage(device, allocator, ci))) };
		}

		BufferHandle Device::createBuffer(BufferCreateInfo ci) {
			return BufferHandle{ std::move(Buffer(device, graphicsQFamilyIndex, allocator, ci)) };
		}

		TimelineSemaphore Device::createTimelineSemaphore() {
			return TimelineSemaphore{ device };
		}

		RenderWindow Device::createRenderWindow(void* sdlWindowHandle, u32 width, u32 height, VkPresentModeKHR presentMode) {
			return RenderWindow{ device, physicalDevice, instance->instance, graphicsQ, sdlWindowHandle, width, height, presentMode };
		}

		CommandList Device::getEmptyCommandList() {
			return std::move(getNextCommandList());
		}

		void Device::submit(SubmitInfo&& si) {

			submitCommandBufferBuffer.clear();
			for (auto& cmdList : si.commandLists) {
				assert(cmdList.operationsInProgress == 0);
				submitCommandBufferBuffer.push_back(cmdList.cmd);
			}

			for (auto [timelineSema, waitValue] : si.waitOnTimelines) {
				this->submitSemaphoreWaitOnBuffer.push_back(timelineSema->getVkSemaphore());
				this->submitSemaphoreWaitOnValueBuffer.push_back(waitValue);
			}
			for (auto binarySemaphore : si.waitOnSemaphores) {
				this->submitSemaphoreWaitOnBuffer.push_back(binarySemaphore);
				this->submitSemaphoreWaitOnValueBuffer.push_back(0);
			}
			for (auto [timelineSema, signalValue] : si.signalTimelines) {
				this->submitSemaphoreSignalBuffer.push_back(timelineSema->getVkSemaphore());
				this->submitSemaphoreSignalValueBuffer.push_back(signalValue);
			}
			for (auto binarySema : si.signalSemaphores) {
				this->submitSemaphoreSignalBuffer.push_back(binarySema);
				this->submitSemaphoreSignalValueBuffer.push_back(0);
			}

			auto thisSubmitTimelineSema = getNextTimeline();
			auto thisSubmitTimelineFinishCounter = thisSubmitTimelineSema.getCounter() + 1;
			this->submitSemaphoreSignalBuffer.push_back(thisSubmitTimelineSema.getVkSemaphore());
			this->submitSemaphoreSignalValueBuffer.push_back(thisSubmitTimelineFinishCounter);

			VkTimelineSemaphoreSubmitInfo timelineSI{
				.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
				.pNext = nullptr,
				.waitSemaphoreValueCount = (u32)submitSemaphoreWaitOnValueBuffer.size(),
				.pWaitSemaphoreValues = submitSemaphoreWaitOnValueBuffer.data(),
				.signalSemaphoreValueCount = (u32)submitSemaphoreSignalValueBuffer.size(),
				.pSignalSemaphoreValues = submitSemaphoreSignalValueBuffer.data(),
			};

			VkPipelineStageFlags pipelineStages = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;	// TODO maybe remove this
			VkSubmitInfo submitInfo{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.pNext = &timelineSI,
				.waitSemaphoreCount = (u32)submitSemaphoreWaitOnBuffer.size(),
				.pWaitSemaphores = submitSemaphoreWaitOnBuffer.data(),
				.pWaitDstStageMask = &pipelineStages,
				.commandBufferCount = (u32)submitCommandBufferBuffer.size(),
				.pCommandBuffers = submitCommandBufferBuffer.data() ,
				.signalSemaphoreCount = (u32)submitSemaphoreSignalBuffer.size(),
				.pSignalSemaphores = submitSemaphoreSignalBuffer.data(),
			};
			vkQueueSubmit(graphicsQ, 1, &submitInfo, nullptr);

			PendingSubmit pendingSubmit{
				.cmdLists = std::move(si.commandLists),
				.timelineSema = std::move(thisSubmitTimelineSema),
				.finishCounter = thisSubmitTimelineFinishCounter,
			};
			unfinishedSubmits.push_back(std::move(pendingSubmit));

			submitSemaphoreWaitOnBuffer.clear();
			submitSemaphoreWaitOnBuffer.clear();
			submitSemaphoreSignalBuffer.clear();
			submitSemaphoreSignalValueBuffer.clear();
		}

		void Device::recycle() {
			for (auto iter = unfinishedSubmits.begin(); iter != unfinishedSubmits.end();) {
				if (iter->timelineSema.getCounter() >= iter->finishCounter) {
					while (!iter->cmdLists.empty()) {
						auto list = std::move(iter->cmdLists.back());
						iter->cmdLists.pop_back();
						for (auto& buffer : list.usedBuffers) {
							buffer->bInUseOnGPU = false;
						}
						list.reset();
						unusedCommandLists.push_back(std::move(list));
					}
					unusedTimelines.push_back(std::move(iter->timelineSema));
					iter = unfinishedSubmits.erase(iter);
				}
				else {
					++iter;
				}
			}
		}

		void Device::waitIdle() {
			for (auto iter = unfinishedSubmits.begin(); iter != unfinishedSubmits.end(); ++iter) {
				iter->timelineSema.wait(iter->finishCounter);
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

		TimelineSemaphore Device::getNextTimeline() {
			if (unusedTimelines.empty()) {
				unusedTimelines.push_back(TimelineSemaphore{ device });
				printf("create new timeline\n");
			}
			auto timeline = std::move(unusedTimelines.back());
			unusedTimelines.pop_back();
			return timeline;
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