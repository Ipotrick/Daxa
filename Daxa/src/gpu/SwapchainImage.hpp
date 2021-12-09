#pragma once

#include <memory>

#include <deque>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include "../DaxaCore.hpp"

#include "Image.hpp"

namespace daxa {
	namespace gpu {

		class SwapchainImage {
		public:
			u32 getImageIndex() const { return imageIndex; }
			VkSwapchainKHR getVkSwapchain() const { return swapchain; }
			ImageHandle& getImageHandle() { return image; }
			ImageHandle const& getImageHandle() const { return image; }
		private:
			friend class RenderWindow;
			friend class Queue;

			ImageHandle image = {};
			u32 imageIndex = {};
			VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		};

	}
}