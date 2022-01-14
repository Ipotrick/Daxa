#include "Signal.hpp"
#include "Instance.hpp"

namespace daxa {
	namespace gpu {
		Signal::Signal(VkDevice device, SignalCreateInfo const& ci) 
			: device{ device }
		{
			VkSemaphoreCreateInfo semaphoreCI{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
			};
			vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore);

			if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
				this->debugName = ci.debugName;

				VkDebugUtilsObjectNameInfoEXT imageNameInfo{
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = NULL,
					.objectType = VK_OBJECT_TYPE_SEMAPHORE,
					.objectHandle = (uint64_t)semaphore,
					.pObjectName = ci.debugName,
				};
				instance->pfnSetDebugUtilsObjectNameEXT(device, &imageNameInfo);
			}
		}

		Signal::~Signal() {
			if (device) {
				vkDestroySemaphore(device, semaphore, nullptr);
				device = VK_NULL_HANDLE;
			}
		}
	}
}