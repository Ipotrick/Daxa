#pragma once

#include "../../DaxaCore.hpp"
#include "Backend.hpp"
#include "Backend.hpp"

#include <vector>
#include <mutex>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <VkBootstrap.h>

#include "GPUHandleGraveyard.hpp"
#include "GPURessources.hpp"

#if USE_NSIGHT_AFTERMATH
#if VK_HEADER_VERSION < 135
#error Minimum requirement for the Aftermath application integration is the Vulkan 1.2.135 SDK
#endif
#include "NsightAftermathHelpers.h"
#include "NsightAftermathGpuCrashTracker.h"
#include "NsightAftermathShaderDatabase.h"
#endif

namespace daxa {
	struct DeviceBackend {
		DeviceBackend(vkb::Instance& instance, PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT);
		~DeviceBackend();

		vkb::Device device                                                              = {};
		
		void (*vkCmdBeginRenderingKHR)(VkCommandBuffer, const VkRenderingInfoKHR*) 		= nullptr;
		void (*vkCmdEndRenderingKHR)(VkCommandBuffer) 									= nullptr;
		void (*vkCmdPipelineBarrier2KHR)(VkCommandBuffer, VkDependencyInfoKHR const*) 	= nullptr;
		VmaAllocator allocator 															= {};
		u32 graphicsQFamilyIndex 														= 0xffffffff;
		u32 transferQFamilyIndex														= 0xffffffff;
		u32 computeQFamilyIndex															= 0xffffffff;
		std::vector<u32> allQFamilyIndices 												= {};
		GPUHandleGraveyard graveyard                                                    = {};
		GPURessourceTable gpuRessources 												= {};
		GPUHandleGraveyard gpuHandleGraveyard											= {};
		VkPhysicalDeviceProperties2 properties 											= {};

		BufferHandle createBuffer(BufferInfo const& info);
		ImageViewHandle createImageView(ImageViewInfo const& info);
		SamplerHandle createSampler(SamplerInfo const& info);

#if USE_NSIGHT_AFTERMATH
	// GPU crash dump tracker using Nsight Aftermath instrumentation
	GpuCrashTracker gpuCrashTracker;
#endif

	};

	void deviceBackendDestructor(void* deviceBackend) {
		DeviceBackend* backend = reinterpret_cast<DeviceBackend*>(deviceBackend);
		backend->~DeviceBackend();
	}
}
