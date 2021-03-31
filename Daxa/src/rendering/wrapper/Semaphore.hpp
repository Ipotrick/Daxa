#pragma once

#include "../Vulkan.hpp"

namespace daxa{
	namespace vk {
		class Semaphore {
		public:
			Semaphore(VkDevice device = vk::mainDevice);

			Semaphore(Semaphore&& other) noexcept;

			~Semaphore();

			operator const VkSemaphore&();

			const VkSemaphore& get();
		private:
			VkDevice device{ VK_NULL_HANDLE };
			VkSemaphore semaphore{ VK_NULL_HANDLE };
		};
	}
}
