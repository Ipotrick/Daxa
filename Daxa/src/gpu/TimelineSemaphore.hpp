#pragma once

#include "../DaxaCore.hpp"

#include <atomic>
#include <memory>

#include <vulkan/vulkan.h>

namespace daxa {
	namespace gpu {

		/**
		* TimelineSemaphore is a persistent datastructure. 
		* Meaning that it shoould exist for the whole duration of the program and not be created and destroyed in between frames.
		*/
		class TimelineSemaphore {
		public:
			TimelineSemaphore(VkDevice);
			TimelineSemaphore(TimelineSemaphore&&) noexcept				= delete;
			TimelineSemaphore& operator=(TimelineSemaphore&&) noexcept	= delete;
			TimelineSemaphore(TimelineSemaphore const&)					= delete;
			TimelineSemaphore& operator=(TimelineSemaphore const&) 		= delete;
			~TimelineSemaphore();

			VkSemaphore getVkSemaphore() const { return timelineSema; }
			u64 getCounter() const;
			void setCounter(u64 newCounterValue);
			VkResult wait(u64 counter, u64 timeout = UINT64_MAX);
		private:
			friend class Device;
			friend class Queue;

			VkDevice device 			= VK_NULL_HANDLE;
			VkSemaphore timelineSema 	= VK_NULL_HANDLE;
		};

		class TimelineSemaphoreHandle {
		public:
			TimelineSemaphoreHandle(std::shared_ptr<TimelineSemaphore> timeline) 
				: timeline{ std::move(timeline) }
			{}
			TimelineSemaphoreHandle() = default;

			TimelineSemaphore const& operator*() const { return *timeline; }
			TimelineSemaphore& operator*() { return *timeline; }
			TimelineSemaphore const* operator->() const { return timeline.get(); }
			TimelineSemaphore* operator->() { return timeline.get(); }

			size_t getRefCount() const { return timeline.use_count(); }

			operator bool() const { return timeline.operator bool(); }
			bool operator!() const { return !timeline; }
			bool valid() const { return *this; }
		private:
			std::shared_ptr<TimelineSemaphore> timeline = {};
		};
	}
}