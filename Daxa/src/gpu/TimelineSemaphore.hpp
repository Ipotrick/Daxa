#pragma once

#include "../DaxaCore.hpp"

#include <atomic>
#include <memory>

#include <vulkan/vulkan.h>

#include "Handle.hpp"

namespace daxa {
	struct TimelineSemaphoreCreateInfo {
		u64 		initialValue	= 0;
		char const* debugName 		= {};
	};

	/**
	* TimelineSemaphore is a persistent datastructure. 
	* Meaning that it shoould exist for the whole duration of the program and not be created and destroyed in between frames.
	*/
	class TimelineSemaphore {
	public:
		TimelineSemaphore(std::shared_ptr<DeviceBackend> deviceBackend, TimelineSemaphoreCreateInfo const& ci);
		TimelineSemaphore(TimelineSemaphore&&) noexcept				= delete;
		TimelineSemaphore& operator=(TimelineSemaphore&&) noexcept	= delete;
		TimelineSemaphore(TimelineSemaphore const&)					= delete;
		TimelineSemaphore& operator=(TimelineSemaphore const&) 		= delete;
		~TimelineSemaphore();

		VkSemaphore getVkSemaphore() const { return timelineSema; }
		u64 getCounter() const;
		void setCounter(u64 newCounterValue);
		VkResult wait(u64 counter, u64 timeout = UINT64_MAX);

		std::string const& getDebugName() const { return debugName; }
	private:
		friend class Device;
		friend class Queue;

		std::shared_ptr<DeviceBackend> deviceBackend	= VK_NULL_HANDLE;
		VkSemaphore timelineSema 						= VK_NULL_HANDLE;
		std::string debugName 							= {};
	};

	class TimelineSemaphoreHandle : public SharedHandle<TimelineSemaphore>{};
}