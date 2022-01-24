#include "TimelineSemaphore.hpp"
#include "Instance.hpp"

namespace daxa {
	namespace gpu {

		TimelineSemaphore::TimelineSemaphore(std::shared_ptr<DeviceBackend> deviceBackend, TimelineSemaphoreCreateInfo const& ci) 
			: deviceBackend{ std::move(deviceBackend) }
		{

			VkSemaphoreTypeCreateInfo semaphoreTypeCI{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
				.pNext = nullptr,
				.semaphoreType = VkSemaphoreType::VK_SEMAPHORE_TYPE_TIMELINE,
				.initialValue = ci.initialValue,
			};

			VkSemaphoreCreateInfo timelineSemaphoreCI{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = &semaphoreTypeCI,
				.flags = 0,
			};

			vkCreateSemaphore(this->deviceBackend->device.device, &timelineSemaphoreCI, nullptr, &this->timelineSema);

			if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
				this->debugName = ci.debugName;

				VkDebugUtilsObjectNameInfoEXT imageNameInfo{
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = NULL,
					.objectType = VK_OBJECT_TYPE_SEMAPHORE,
					.objectHandle = (uint64_t)timelineSema,
					.pObjectName = ci.debugName,
				};
				instance->pfnSetDebugUtilsObjectNameEXT(this->deviceBackend->device.device, &imageNameInfo);
			}
		}

		TimelineSemaphore::~TimelineSemaphore() {
			if (this->deviceBackend->device.device && timelineSema) {
				vkDestroySemaphore(this->deviceBackend->device.device, timelineSema, nullptr);
				this->deviceBackend = {};
			}
		}

		u64 TimelineSemaphore::getCounter() const {
			u64 counter = 0;
			vkGetSemaphoreCounterValue(this->deviceBackend->device.device, timelineSema, &counter);
			return counter;
		}

		void TimelineSemaphore::setCounter(u64 newCounterValue) {
			VkSemaphoreSignalInfo semaphoreSI{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
				.pNext = nullptr,
				.semaphore = timelineSema,
				.value = newCounterValue,
			};

			vkSignalSemaphore(this->deviceBackend->device.device, &semaphoreSI);
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

			return vkWaitSemaphores(this->deviceBackend->device.device, &semaphoreWI, timeout);
		}
	}
}