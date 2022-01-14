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
			vkb::PhysicalDeviceSelector selector{ instance };
			vkb::PhysicalDevice physicalDevice = selector
				.set_minimum_version(1, 2)
				.defer_surface_initialization()
				.add_desired_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
				.add_desired_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
				.select()
				.value();

			auto physicslDevice = physicalDevice.physical_device;

			VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeature{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
				.pNext = nullptr,
				.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
				.descriptorBindingPartiallyBound = VK_TRUE,
				.descriptorBindingVariableDescriptorCount = VK_TRUE,
				.runtimeDescriptorArray = VK_TRUE,
			};

			VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
				.pNext = nullptr,
				.dynamicRendering = VK_TRUE,
			};

			VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
				.pNext = nullptr,
				.timelineSemaphore = VK_TRUE,
			};

			VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2Features{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
				.pNext = nullptr,
				.synchronization2 = VK_TRUE,
			};
			VkPhysicalDeviceShaderAtomicInt64Features atomicI64Features{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES,
				.pNext = nullptr,
				.shaderBufferInt64Atomics = VK_TRUE,
				.shaderSharedInt64Atomics = VK_TRUE,
			};

			vkb::DeviceBuilder deviceBuilder{ physicalDevice };
			deviceBuilder.add_pNext(&descriptorIndexingFeature);
			deviceBuilder.add_pNext(&dynamicRenderingFeature);
			deviceBuilder.add_pNext(&timelineSemaphoreFeatures);
			deviceBuilder.add_pNext(&synchronization2Features);
			deviceBuilder.add_pNext(&atomicI64Features);

			vkb::Device vkbDevice = deviceBuilder.build().value();

			auto device = vkbDevice.device;

			auto mainQueueFamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

			VmaAllocatorCreateInfo allocatorInfo = {
				.physicalDevice = physicslDevice,
				.device = device,
				.instance = instance.instance,
			};
			VmaAllocator allocator;
			vmaCreateAllocator(&allocatorInfo, &allocator);

			auto fnPtrvkCmdBeginRenderingKHR = (void(*)(VkCommandBuffer, const VkRenderingInfoKHR*))vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR");
			auto fnPtrvkCmdEndRenderingKHR = (void(*)(VkCommandBuffer))vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR");
			DAXA_ASSERT_M(fnPtrvkCmdBeginRenderingKHR != nullptr && fnPtrvkCmdEndRenderingKHR != nullptr, "could not load VK_KHR_DYNAMIC_RENDERING_EXTENSION");

			auto fnPtrvkCmdPipelineBarrier2KHR = (void(*)(VkCommandBuffer, VkDependencyInfoKHR const*))vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier2KHR");
			DAXA_ASSERT_M(fnPtrvkCmdPipelineBarrier2KHR != nullptr, "could not load VK_KHR_SYNCHRONIZATION_2_EXTENSION");

			this->instance = instance.instance;
			this->device = device;
			this->bindingSetDescriptionCache = std::make_unique<BindingSetDescriptionCache>(device);
			this->physicalDevice = physicalDevice.physical_device;
			this->graphicsQFamilyIndex = mainQueueFamilyIndex;
			this->allocator = allocator;
			this->vkCmdBeginRenderingKHR = fnPtrvkCmdBeginRenderingKHR;
			this->vkCmdEndRenderingKHR = fnPtrvkCmdEndRenderingKHR;
			this->vkCmdPipelineBarrier2KHR = fnPtrvkCmdPipelineBarrier2KHR;
			this->stagingBufferPool = std::make_shared<StagingBufferPool>(StagingBufferPool{ device, mainQueueFamilyIndex, allocator });
			this->vkbDevice = vkbDevice;
			this->cmdListRecyclingSharedData = std::make_shared<CommandListRecyclingSharedData>();
		}

		Device::~Device() {
			if (device) {
				waitIdle();
				cmdListRecyclingSharedData.reset();
				unusedCommandLists.clear();
				stagingBufferPool.reset();
				bindingSetDescriptionCache.reset();
				vmaDestroyAllocator(allocator);
				vkDestroyDevice(device, nullptr);
				device = VK_NULL_HANDLE;
			}
		}

		QueueHandle Device::createQueue(QueueCreateInfo const& ci) {
			return QueueHandle{ std::make_shared<Queue>(device, vkbDevice.get_queue(vkb::QueueType::graphics).value(), ci) };
		}

		SamplerHandle Device::createSampler(SamplerCreateInfo ci) {
			return SamplerHandle{ std::make_shared<Sampler>(device, ci) };
		}

		ImageHandle Device::createImage2d(Image2dCreateInfo ci) { 
			auto handle = ImageHandle{ std::make_shared<Image>() };
			Image::construct2dImage(device, allocator,graphicsQFamilyIndex, ci, *handle);
			return std::move(handle);
		}

		BufferHandle Device::createBuffer(BufferCreateInfo ci) {
			return BufferHandle{ std::make_shared<Buffer>(device, graphicsQFamilyIndex, allocator, ci) };
		}

		TimelineSemaphoreHandle Device::createTimelineSemaphore(TimelineSemaphoreCreateInfo const& ci) {
			return TimelineSemaphoreHandle{ std::make_shared<TimelineSemaphore>(device, ci) };
		}

		SignalHandle Device::createSignal(SignalCreateInfo const& ci) {
			return SignalHandle{ std::make_shared<Signal>(device, ci) };
		}

		SwapchainHandle Device::createSwapchain(SwapchainCreateInfo ci) {
			auto handle = SwapchainHandle{ std::make_shared<Swapchain>() };
			handle->construct(device, physicalDevice, gpu::instance->getVkInstance(), ci);
			return std::move(handle);
		}

		BindingSetDescription const* Device::createBindingSetDescription(std::span<VkDescriptorSetLayoutBinding> bindings) {
			return bindingSetDescriptionCache->getSetDescription(bindings);
		}

		CommandListHandle Device::getCommandList(char const* debugName) {
			auto list = std::move(getNextCommandList());
			list->setDebugName(debugName);
			return std::move(list);
		}

		void Device::waitIdle() {
			vkDeviceWaitIdle(device);
		}

		CommandListHandle Device::getNextCommandList() {
			if (unusedCommandLists.empty()) {
				auto lock = std::unique_lock(cmdListRecyclingSharedData->mut);
				while (!cmdListRecyclingSharedData->zombies.empty()) {
					unusedCommandLists.push_back(std::move(cmdListRecyclingSharedData->zombies.back()));
					cmdListRecyclingSharedData->zombies.pop_back();
				}
			}

			if (unusedCommandLists.empty()) {
				// we have no command lists left, we need to create new ones:
				unusedCommandLists.push_back(CommandListHandle{ std::make_shared<CommandList>() });
				CommandList& list = *unusedCommandLists.back();
				list.device = device;
				list.vkCmdBeginRenderingKHR = this->vkCmdBeginRenderingKHR;
				list.vkCmdEndRenderingKHR = this->vkCmdEndRenderingKHR;
				list.vkCmdPipelineBarrier2KHR = this->vkCmdPipelineBarrier2KHR;
				list.stagingBufferPool = stagingBufferPool;
				list.recyclingData = cmdListRecyclingSharedData;

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
				list.begin();
			} 
			auto ret = std::move(unusedCommandLists.back());
			unusedCommandLists.pop_back();
			return std::move(ret);
		}

		PipelineHandle Device::createGraphicsPipeline(GraphicsPipelineBuilder& pipelineBuilder) {
			return pipelineBuilder.build(device, *bindingSetDescriptionCache);
		}

		PipelineHandle Device::createComputePipeline(ComputePipelineCreateInfo const& ci) {
			return gpu::createComputePipeline(device, *bindingSetDescriptionCache, ci);
		}

		BindingSetAllocatorHandle Device::createBindingSetAllocator(BindingSetAllocatorCreateInfo const& ci) {
			return BindingSetAllocatorHandle{ std::make_shared<BindingSetAllocator>(device, ci) };
		}

		Result<ShaderModuleHandle> Device::createShaderModule(ShaderModuleCreateInfo const& ci) {
			return ShaderModuleHandle::tryCreateDAXAShaderModule(device, ci);
		}
	}
}