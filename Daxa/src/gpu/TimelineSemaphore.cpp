#include "TimelineSemaphore.hpp"
#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"

namespace daxa {

	TimelineSemaphore::TimelineSemaphore(std::shared_ptr<void> backend, TimelineSemaphoreCreateInfo const& ci) 
		: backend{ std::move(backend) }
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
		auto deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);

		DAXA_CHECK_VK_RESULT_M(vkCreateSemaphore(deviceBackend->device.device, &timelineSemaphoreCI, nullptr, &this->timelineSema), "could not create timeline semaphore");

		if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
			this->debugName = ci.debugName;

			VkDebugUtilsObjectNameInfoEXT imageNameInfo{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_SEMAPHORE,
				.objectHandle = (uint64_t)timelineSema,
				.pObjectName = ci.debugName,
			};
			instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &imageNameInfo);
		}
	}

	TimelineSemaphore::~TimelineSemaphore() {
		if (this->backend && timelineSema) {
			auto deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
			vkDestroySemaphore(deviceBackend->device.device, timelineSema, nullptr);
			deviceBackend = {};
		}
	}

	u64 TimelineSemaphore::getCounter() const {
		u64 counter = 0;
		auto deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		DAXA_CHECK_VK_RESULT_M(vkGetSemaphoreCounterValue(deviceBackend->device.device, timelineSema, &counter), "could not aquire timeline semaphore counter");
		return counter;
	}

	void TimelineSemaphore::setCounter(u64 newCounterValue) {
		VkSemaphoreSignalInfo semaphoreSI{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
			.pNext = nullptr,
			.semaphore = timelineSema,
			.value = newCounterValue,
		};

		auto deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		DAXA_CHECK_VK_RESULT_M(vkSignalSemaphore(deviceBackend->device.device, &semaphoreSI), "could not signal timeline semaphore");
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

		auto deviceBackend = std::static_pointer_cast<DeviceBackend>(this->backend);
		return vkWaitSemaphores(deviceBackend->device.device, &semaphoreWI, timeout);
	}
}