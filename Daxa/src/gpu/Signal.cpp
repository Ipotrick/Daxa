#include "Signal.hpp"

namespace daxa {
	namespace gpu {
		Signal::Signal(VkDevice device) 
			: device{ device }
		{
			VkSemaphoreCreateInfo semaphoreCI{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
			};
			vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore);
		}

		Signal::~Signal() {
			if (device) {
				vkDestroySemaphore(device, semaphore, nullptr);
				device = VK_NULL_HANDLE;
			}
		}
	}
}