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
			TimelineSemaphore(TimelineSemaphore&&) noexcept;
			TimelineSemaphore& operator=(TimelineSemaphore&&) noexcept;
			TimelineSemaphore(TimelineSemaphore const&) = delete;
			TimelineSemaphore& operator=(TimelineSemaphore const&) = delete;
			~TimelineSemaphore();

			VkSemaphore getVkSemaphore() const { return timelineSema; }
			u64 getCounter() const;
			void setCounter(u64 newCounterValue);
			VkResult wait(u64 counter, u64 timeout = UINT64_MAX);
		private:
			friend class Device;
			friend class Queue;

			TimelineSemaphore(VkDevice);

			VkDevice device = VK_NULL_HANDLE;
			VkSemaphore timelineSema = VK_NULL_HANDLE;
		};
	}
}