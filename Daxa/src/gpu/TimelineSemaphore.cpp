#include "TimelineSemaphore.hpp"

namespace daxa {
	namespace gpu {

		TimelineSemaphore::TimelineSemaphore(VkDevice device) 
			: device{ device }
		{

			VkSemaphoreTypeCreateInfo semaphoreTypeCI{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
				.pNext = nullptr,
				.semaphoreType = VkSemaphoreType::VK_SEMAPHORE_TYPE_TIMELINE,
				.initialValue = 0,
			};

			VkSemaphoreCreateInfo timelineSemaphoreCI{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = &semaphoreTypeCI,
				.flags = 0,
			};

			vkCreateSemaphore(this->device, &timelineSemaphoreCI, nullptr, &this->timelineSema);
		}

		TimelineSemaphore::~TimelineSemaphore() {
			if (device && timelineSema) {
				vkDestroySemaphore(device, timelineSema, nullptr);
			}
			std::memset(this, 0, sizeof(TimelineSemaphore));
		}

		u64 TimelineSemaphore::getCounter() const {
			u64 counter = 0;
			vkGetSemaphoreCounterValue(device, timelineSema, &counter);
			return counter;
		}

		void TimelineSemaphore::setCounter(u64 newCounterValue) {
			VkSemaphoreSignalInfo semaphoreSI{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
				.pNext = nullptr,
				.semaphore = timelineSema,
				.value = newCounterValue,
			};

			vkSignalSemaphore(device, &semaphoreSI);
		}

		VkResult TimelineSemaphore::wait(u64 counter, u64 timeout) {
			VkSemaphoreWaitInfo semaphoreWI{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
				.pNext = nullptr,
				.flags = 0,
				.semaphoreCount = 1,
				.pSemaphores = &timelineSema,
				.pValues = &counter,
			};

			return vkWaitSemaphores(device, &semaphoreWI, timeout);
		}
	}
}