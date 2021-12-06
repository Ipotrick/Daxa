#pragma once

#include <memory>

#include <deque>

#include <vulkan/vulkan.h>

#include "../dependencies/vk_mem_alloc.hpp"

#include "../../DaxaCore.hpp"

#include "Image.hpp"

namespace daxa {
	namespace gpu {

		class SwapchainImage {
		public:
			SwapchainImage() = default;
			SwapchainImage(SwapchainImage const&) = default;
			SwapchainImage& operator=(SwapchainImage const&) = default;
			SwapchainImage(SwapchainImage&& other) noexcept {
				std::memcpy(this, &other, sizeof(SwapchainImage));
				std::memset(&other, 0, sizeof(SwapchainImage));
			}
			SwapchainImage& operator=(SwapchainImage&& other) {
				std::memcpy(this, &other, sizeof(SwapchainImage));
				std::memset(&other, 0, sizeof(SwapchainImage));
				return *this;
			}

			u32 getImageIndex() const { return imageIndex; }
			VkSwapchainKHR getVkSwapchain() const { return swapchain; }
			ImageHandle& getImageHandle() { return image; }
			ImageHandle const& getImageHandle() const { return image; }
		private:
			friend class RenderWindow;
			friend class Queue;

			ImageHandle image;
			u32 imageIndex;
			VkSwapchainKHR swapchain;
		};

	}
}