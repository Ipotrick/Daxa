#pragma once

#include <memory>

#include <deque>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include "../DaxaCore.hpp"

#include "Image.hpp"

namespace daxa {
	class SwapchainImage {
	public:
		u32 getImageIndex() const { return imageIndex; }
		VkSwapchainKHR getVkSwapchain() const { return swapchain; }
		ImageViewHandle& getImageViewHandle() { return image; }
		ImageViewHandle const& getImageViewHandle() const { return image; }
	private:
		friend class Swapchain;
		friend class CommandQueue;

		ImageViewHandle image 		= {};
		u32 			imageIndex 	= {};
		VkSwapchainKHR 	swapchain 	= {};
	};
}