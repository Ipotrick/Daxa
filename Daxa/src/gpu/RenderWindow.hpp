#pragma once

#include <memory>
#include <vector>
#include <deque>
#include <span>

#include <vulkan/vulkan.h>

#include "../dependencies/vk_mem_alloc.hpp"

#include "../DaxaCore.hpp"

#include "Image.hpp"
#include "SwapchainImage.hpp"
#include "Signal.hpp"

namespace daxa {
	namespace gpu {

		class RenderWindow {
		public:
			RenderWindow() = default;
			RenderWindow(RenderWindow&&) noexcept;
			RenderWindow& operator=(RenderWindow&&) noexcept;
			RenderWindow(RenderWindow const&) = delete;
			RenderWindow& operator=(RenderWindow const&) = delete;
			~RenderWindow();

			void resize(VkExtent2D newSize);

			void setPresentMode(VkPresentModeKHR newPresentMode);

			SwapchainImage aquireNextImage();

			VkExtent2D getSize() const { return size; }

			VkFormat getVkFormat() const { return swapchainImageFormat; }
		private:
			friend class Device;

			RenderWindow(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance, void* sdl_window_handle, u32 width, u32 height, VkPresentModeKHR presentmode, VkSwapchainKHR = nullptr, VkSurfaceKHR oldSurface = nullptr);

			VkDevice device = VK_NULL_HANDLE;
			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			VkInstance instance = VK_NULL_HANDLE;
			VkFence aquireFence = VK_NULL_HANDLE;
			VkPresentModeKHR presentMode{ VK_PRESENT_MODE_FIFO_KHR };
			VkSurfaceKHR surface;
			VkSwapchainKHR swapchain; 
			VkFormat swapchainImageFormat;
			std::vector<ImageHandle> swapchainImages;					// TODO REPLACE WITH STACK ALLOCATED VECTOR TYPE
			VkExtent2D size;
			void* sdl_window_handle = nullptr;
		};

	}
}
