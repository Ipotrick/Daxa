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
			//.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = 1,
			.poolSizeCount = (u32)bindAllPoolSizes.size(),
			.pPoolSizes = bindAllPoolSizes.data(),
		};
		DAXA_CHECK_VK_RESULT_M(vkCreateDescriptorPool(vkbDevice.device, &poolCI, nullptr, &bindAllSetPool), "failed to create bind all set pool");
		auto bindAllSetDescriptorSetLayoutBindings = std::array {
			BIND_ALL_SAMPLER_SET_LAYOUT_BINDING,
			BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING,
			BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING,
			BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING,
			BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING,
		};
		auto bindAllSetLayoutBindingFlags = std::array<VkDescriptorBindingFlags, 5>{
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
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
			//.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = (u32)bindAllSetDescriptorSetLayoutBindings.size(),
			.pBindings = bindAllSetDescriptorSetLayoutBindings.data()
		};
		DAXA_CHECK_VK_RESULT_M(vkCreateDescriptorSetLayout(vkbDevice.device, &bindAllSetLayoutCI, nullptr, &bindAllSetLayout), "failed to create bind all set layout");
		VkDescriptorSetAllocateInfo bindAllSetAI {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = bindAllSetPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &bindAllSetLayout,
		};
		DAXA_CHECK_VK_RESULT_M(vkAllocateDescriptorSets(vkbDevice.device, &bindAllSetAI, &bindAllSet), "failed to create bind all set");
		createDummies();
		
		if (pfnSetDebugUtilsObjectNameEXT) {
			VkDebugUtilsObjectNameInfoEXT nameInfo {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET,
				.objectHandle = (uint64_t)bindAllSet,
				.pObjectName = "Bind all set",
			};
			pfnSetDebugUtilsObjectNameEXT(device, &nameInfo);
		}
	}

	DeviceBackend::~DeviceBackend() {
		if (device) {
			destroyDummies();
			vkResetDescriptorPool(device.device, bindAllSetPool, 0);
			vkDestroyDescriptorPool(device.device, bindAllSetPool, nullptr);
			vkDestroyDescriptorSetLayout(device.device, bindAllSetLayout, nullptr);
			vkDeviceWaitIdle(device.device);
			vmaDestroyAllocator(allocator);
			vkb::destroy_device(device);
		}
	}
	
	void DeviceBackend::createDummies() {
		VkSamplerCreateInfo samplerCI {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_NEAREST,
			.minFilter = VK_FILTER_NEAREST,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE,
			.maxAnisotropy = 1,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0,
			.maxLod = 0,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE,
		};
		vkCreateSampler(device.device, &samplerCI, nullptr, &dummySampler);
	}

	void DeviceBackend::destroyDummies() {
		vkDestroySampler(device.device, dummySampler, nullptr);
	}

	const char* getVkResultString(VkResult result) {
		switch (result) {
			case VK_SUCCESS: return "VK_SUCCESS";
			case VK_NOT_READY: return "VK_NOT_READY";
			case VK_TIMEOUT: return "VK_TIMEOUT";
			case VK_EVENT_SET: return "VK_EVENT_SET";
			case VK_EVENT_RESET: return "VK_EVENT_RESET";
			case VK_INCOMPLETE: return "VK_INCOMPLETE";
			case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
			case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
			case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
			case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
			case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
			case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
			case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
			case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
			case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
			case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
			case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_INITIVK_ERROR_FORMAT_NOT_SUPPORTEDALIZATION_FAILED";
			case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
			case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
			case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
			case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
			case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
			case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
			case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
			case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
			case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
			case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
			case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
			case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
			case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
			case VK_ERROR_NOT_PERMITTED_EXT: return "VK_ERROR_NOT_PERMITTED_EXT";
			case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
			case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
			case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
			case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
			case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
			case VK_PIPELINE_COMPILE_REQUIRED_EXT: return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
			default: return "unknown result";
		}
	}
}