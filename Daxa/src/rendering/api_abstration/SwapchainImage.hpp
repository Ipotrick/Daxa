#pragma once

#include <memory>

#include <deque>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

namespace daxa {
	namespace gpu {

		class SwapchainImage {
		public:
			vk::Image getVkImage() const { return image; }
			vk::ImageView getVkImageView() const { return imageView; }
			u32 getImageIndex() const { return imageIndex; }
			vk::SwapchainKHR getVkSwapchain() const { return swapchain; }
			vk::Semaphore getVkSemaphore() const { return presentSemaphore; }
		private:
			friend class Device;
			friend class RenderWindow;

			vk::Image image;
			vk::ImageView imageView;
			u32 imageIndex;
			vk::SwapchainKHR swapchain;
			vk::Semaphore presentSemaphore;
		};
	}
}