#include "Signal.hpp"

#include "common.hpp"

namespace daxa {
	namespace gpu {
		DAXA_DEFINE_TRIVIAL_MOVE(Signal)

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
				std::memset(this, 0, sizeof(Signal));
			}
		}

		SignalHandle::SignalHandle(std::shared_ptr<Signal> signal)
			: signal{ std::move(signal) }
		{

		}

	}
}