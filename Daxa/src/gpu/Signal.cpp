#include "Signal.hpp"
#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"

namespace daxa {
	Signal::Signal(std::shared_ptr<DeviceBackend> deviceBackend, SignalCreateInfo const& ci) 
		: deviceBackend{ std::move(deviceBackend) }
	{
		VkSemaphoreCreateInfo semaphoreCI{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
		};
		DAXA_CHECK_VK_RESULT_M(vkCreateSemaphore(this->deviceBackend->device.device, &semaphoreCI, nullptr, &semaphore), "failed to create signal");

		if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
			this->debugName = ci.debugName;

			VkDebugUtilsObjectNameInfoEXT imageNameInfo{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_SEMAPHORE,
				.objectHandle = (uint64_t)semaphore,
				.pObjectName = ci.debugName,
			};
			instance->pfnSetDebugUtilsObjectNameEXT(this->deviceBackend->device.device, &imageNameInfo);
		}
	}

	Signal::~Signal() {
		if (deviceBackend->device.device) {
			vkDestroySemaphore(deviceBackend->device.device, semaphore, nullptr);
			deviceBackend = {};
		}
	}
}