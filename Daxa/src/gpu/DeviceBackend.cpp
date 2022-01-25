#include "DeviceBackend.hpp"
#include <array>

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
				.shaderInputAttachmentArrayDynamicIndexing = VK_TRUE,
				.shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE,
				.shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE,
				.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE,
				.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
				.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
				.shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
				.shaderInputAttachmentArrayNonUniformIndexing = VK_TRUE,
				.shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE,
				.shaderStorageTexelBufferArrayNonUniformIndexing = VK_TRUE,
				//.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,	// uniform buffers are specially optimized on nvidia hw
				.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
				.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
				.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
				.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE,
				.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
				.descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
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
			VkPhysicalDevice16BitStorageFeatures device16BitFeatures{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES,
				.pNext = nullptr,
				.storageBuffer16BitAccess = VK_FALSE,
				.uniformAndStorageBuffer16BitAccess = VK_TRUE,
				.storagePushConstant16 = VK_FALSE,
				.storageInputOutput16 = VK_FALSE,
			};

			vkb::DeviceBuilder deviceBuilder{ physicalDevice };
			deviceBuilder.add_pNext(&descriptorIndexingFeature);
			deviceBuilder.add_pNext(&dynamicRenderingFeature);
			deviceBuilder.add_pNext(&timelineSemaphoreFeatures);
			deviceBuilder.add_pNext(&synchronization2Features);
			//deviceBuilder.add_pNext(&atomicI64Features);
			deviceBuilder.add_pNext(&device16BitFeatures);

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

			auto bindAllPoolSizes = std::array{
				BIND_ALL_SAMPLER_POOL_SIZE,
				BIND_ALL_COMBINED_IMAGE_SAMPLER_POOL_SIZE,
				BIND_ALL_SAMPLED_IMAGE_POOL_SIZE,
				BIND_ALL_STORAGE_IMAGE_POOL_SIZE,
				BIND_ALL_STORAGE_BUFFER_POOL_SIZE,
			};
			VkDescriptorPoolCreateInfo poolCI {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.pNext = nullptr,
				.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
				.maxSets = 1,
				.poolSizeCount = (u32)bindAllPoolSizes.size(),
				.pPoolSizes = bindAllPoolSizes.data(),
			};
			DAXA_CHECK_VK_RESULT(vkCreateDescriptorPool(vkbDevice.device, &poolCI, nullptr, &bindAllSetPool));
			auto bindAllSetDescriptorSetLayoutBindings = std::array {
				BIND_ALL_SAMPLER_SET_LAYOUT_BINDING,
				BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING,
				BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING,
				BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING,
				BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING,
			};
			auto bindAllSetLayoutBindingFlags = std::array<VkDescriptorBindingFlags, 5>{
				VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
				VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
				VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
				VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
				VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			};
			VkDescriptorSetLayoutBindingFlagsCreateInfo bindAllSetLayoutBindingFlagsCI {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
				.pNext = nullptr,
				.bindingCount = (u32)bindAllSetLayoutBindingFlags.size(),
				.pBindingFlags = bindAllSetLayoutBindingFlags.data(),
			};
			VkDescriptorSetLayoutCreateInfo bindAllSetLayoutCI {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.pNext = &bindAllSetLayoutBindingFlagsCI,
				.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
				.bindingCount = (u32)bindAllSetDescriptorSetLayoutBindings.size(),
				.pBindings = bindAllSetDescriptorSetLayoutBindings.data()
			};
			DAXA_CHECK_VK_RESULT(vkCreateDescriptorSetLayout(vkbDevice.device, &bindAllSetLayoutCI, nullptr, &bindAllSetLayout));
			VkDescriptorSetAllocateInfo bindAllSetAI {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.pNext = nullptr,
				.descriptorPool = bindAllSetPool,
				.descriptorSetCount = 1,
				.pSetLayouts = &bindAllSetLayout,
			};
			DAXA_CHECK_VK_RESULT(vkAllocateDescriptorSets(vkbDevice.device, &bindAllSetAI, &bindAllSet));
        }

		DeviceBackend::~DeviceBackend() {
			if (device) {
				vkResetDescriptorPool(device.device, bindAllSetPool, 0);
				vkDestroyDescriptorPool(device.device, bindAllSetPool, nullptr);
				vkDestroyDescriptorSetLayout(device.device, bindAllSetLayout, nullptr);
			    vkDeviceWaitIdle(device.device);
				vmaDestroyAllocator(allocator);
                vkb::destroy_device(device);
			}
        }
        
    }
}