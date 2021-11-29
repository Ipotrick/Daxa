#pragma once

#include <memory>

#include <deque>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.h>

#include "../dependencies/vk_mem_alloc.hpp"

#include "../../DaxaCore.hpp"

#include "Device.hpp"
#include "Image.hpp"
#include "SwapchainImage.hpp"

namespace daxa {
	namespace gpu {

		class RenderWindow {
		public:
			RenderWindow(std::shared_ptr<Device> device, std::shared_ptr<vkb::Instance> instance, void* sdl_window_handle, u32 width, u32 height, VkPresentModeKHR presentmode, VkSwapchainKHR = nullptr, VkSurfaceKHR oldSurface = nullptr);
			RenderWindow(RenderWindow&& other);
			RenderWindow& operator=(RenderWindow&& other);
			~RenderWindow();

			void setPresentMode(VkPresentModeKHR newPresentMode);

			SwapchainImage getNextImage(VkSemaphore presentSemaphore);

			VkExtent2D getSize() const { return size; }
		private:
			void resize(VkExtent2D newSize);

			std::shared_ptr<Device> device;
			std::shared_ptr<vkb::Instance> instance;
			VkPresentModeKHR presentMode{ VK_PRESENT_MODE_FIFO_KHR };
			VkSurfaceKHR surface;
			VkSwapchainKHR swapchain; 
			VkFormat swapchainImageFormat;
			std::vector<ImageHandle> swapchainImages;
			u32 imagesInFlight{ 2 };
			VkExtent2D size;
			void* sdl_window_handle = nullptr;
		};

	}
}
