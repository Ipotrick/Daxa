#pragma once

#include "../Vulkan.hpp"

namespace daxa {
	namespace vk {
		class Fence {
		public:
			Fence(VkDevice device = vk::mainDevice);

			Fence(Fence&& other) noexcept;

			~Fence();

			operator const VkFence& ();

			const VkFence& get();
		private:

			VkDevice device{ VK_NULL_HANDLE };
			VkFence fence{ VK_NULL_HANDLE };
		};
	}
}
