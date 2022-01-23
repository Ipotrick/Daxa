#include "DeviceBackend.hpp"

namespace daxa {
    namespace gpu {
		DeviceBackend::DeviceBackend(vkb::Instance& instance) {vkb::PhysicalDeviceSelector selector{ instance };
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

			this->device = vkbDevice;
			this->graphicsQFamilyIndex = mainQueueFamilyIndex;
			auto transferIndexOpt = vkbDevice.get_queue_index(vkb::QueueType::transfer);
			if (transferIndexOpt.has_value()) {
				this->transferQFamilyIndex = transferIndexOpt.value();
			}
			auto computeIndexOpt = vkbDevice.get_queue_index(vkb::QueueType::compute);
			if (computeIndexOpt.has_value()) {
				this->computeQFamilyIndex = computeIndexOpt.value();
			}
			if (graphicsQFamilyIndex != -1) {
				allQFamilyIndices.push_back(graphicsQFamilyIndex);
			}
			if (transferQFamilyIndex != -1) {
				allQFamilyIndices.push_back(transferQFamilyIndex);
			}
			if (computeQFamilyIndex != -1) {
				allQFamilyIndices.push_back(computeQFamilyIndex);
			}
			this->allocator = allocator;
			this->vkCmdBeginRenderingKHR = fnPtrvkCmdBeginRenderingKHR;
			this->vkCmdEndRenderingKHR = fnPtrvkCmdEndRenderingKHR;
			this->vkCmdPipelineBarrier2KHR = fnPtrvkCmdPipelineBarrier2KHR;
        }

		DeviceBackend::~DeviceBackend() {
			if (device) {
			    vkDeviceWaitIdle(device);
				vmaDestroyAllocator(allocator);
                vkb::destroy_device(device);
			}
        }
        
    }
}