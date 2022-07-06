#include "DeviceBackend.hpp"
#include <array>

namespace daxa {
	DeviceBackend::DeviceBackend(vkb::Instance& instance, PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT) {
#if USE_NSIGHT_AFTERMATH
		gpuCrashTracker.Initialize();
#endif
		vkb::PhysicalDeviceSelector selector{ instance };
		vkb::PhysicalDevice physicalDevice = selector
			.set_minimum_version(1, 2)
			.defer_surface_initialization()
			.add_desired_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
			.add_desired_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
#if USE_NSIGHT_AFTERMATH
			.add_required_extension(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME)
			.add_required_extension(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME)
#endif
			.select()
			.value();

		auto physicslDevice = physicalDevice.physical_device;

		VkPhysicalDeviceSubgroupProperties subgroupProperties;
		subgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
		subgroupProperties.pNext = NULL;

		this->properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		this->properties.pNext = &subgroupProperties;

		vkGetPhysicalDeviceProperties2(physicslDevice, &this->properties);

		VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeature{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
			.pNext = nullptr,
			.shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,	// amd has limited support
			.shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE,
			.shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE,
			.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE,
			.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
			.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
			.shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
			.shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE,	// amd has limited support
			.shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE,
			.shaderStorageTexelBufferArrayNonUniformIndexing = VK_TRUE,
			.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE,	// uniform buffers are specially optimized on nvidia hw
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
			.storageBuffer16BitAccess = VK_TRUE,
			.uniformAndStorageBuffer16BitAccess = VK_TRUE,
			.storagePushConstant16 = VK_FALSE,	// AMD YOU BOZO WHY NO SUPPORT
			.storageInputOutput16 = VK_FALSE,
		};
		VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
			.pNext = nullptr,
			.nullDescriptor = VK_TRUE,
		};
		VkPhysicalDeviceFeatures2 deviceFeatures{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = nullptr,
			.features = {
				.samplerAnisotropy = VK_TRUE,
				.shaderSampledImageArrayDynamicIndexing = VK_TRUE,
				.shaderStorageImageArrayDynamicIndexing = VK_TRUE,
			}
		};
		//VK_KHR_shader_non_semantic_info 
		


		vkb::DeviceBuilder deviceBuilder{ physicalDevice };
		deviceBuilder.add_pNext(&deviceFeatures);
		deviceBuilder.add_pNext(&descriptorIndexingFeature);
		deviceBuilder.add_pNext(&dynamicRenderingFeature);
		deviceBuilder.add_pNext(&timelineSemaphoreFeatures);
		deviceBuilder.add_pNext(&synchronization2Features);
		deviceBuilder.add_pNext(&atomicI64Features);
		deviceBuilder.add_pNext(&device16BitFeatures);
		deviceBuilder.add_pNext(&robustness2Features);

#if USE_NSIGHT_AFTERMATH
		VkPhysicalDeviceDiagnosticsConfigFeaturesNV aftermathFeatures{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV,
			.pNext = nullptr,
			.diagnosticsConfig = VK_TRUE,
		};
		VkDeviceDiagnosticsConfigCreateInfoNV aftermathCI{
			.sType = VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV,
			.pNext = nullptr,
			.flags = VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_RESOURCE_TRACKING_BIT_NV |
				VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_AUTOMATIC_CHECKPOINTS_BIT_NV |
				VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_SHADER_DEBUG_INFO_BIT_NV,
		};
		deviceBuilder.add_pNext(&aftermathFeatures);
		deviceBuilder.add_pNext(&aftermathCI);
#endif

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

		createGPURessourceTable(vkbDevice.device, gpuRessources);
		
		if (pfnSetDebugUtilsObjectNameEXT) {
			VkDebugUtilsObjectNameInfoEXT nameInfo {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET,
				.objectHandle = (uint64_t)gpuRessources.bindAllSet,
				.pObjectName = "Bind all set",
			};
			pfnSetDebugUtilsObjectNameEXT(device, &nameInfo);
		}
	}

	BufferHandle DeviceBackend::createBuffer(BufferInfo const& info) {
		return createBufferHandleAndInsertIntoTable(device.device, allocator, gpuRessources, graphicsQFamilyIndex, info);
	}

	ImageViewHandle DeviceBackend::createImageView(ImageViewInfo const& info) {
		return createImageViewHandleAndInsertIntoTable(device.device, gpuRessources, info);
	}

	SamplerHandle DeviceBackend::createSampler(SamplerInfo const& info) {
		return createSamplerHandleAndInsertIntoTable(device.device, gpuRessources, info);
	}

	DeviceBackend::~DeviceBackend() {
		if (device) {
			destroyGPURessourceTable(device.device, gpuRessources);
			vkDeviceWaitIdle(device.device);
			vmaDestroyAllocator(allocator);
			vkb::destroy_device(device);
		}
	}
}