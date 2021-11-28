#pragma once

#include <memory>

#include <deque>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

#include "Device.hpp"
#include "Image.hpp"
#include "SwapchainImage.hpp"

namespace daxa {
	namespace gpu {

		class RenderWindow {
		public:
			RenderWindow(std::shared_ptr<Device> device, std::shared_ptr<vkb::Instance> instance, void* sdl_window_handle, u32 width, u32 height, vk::PresentModeKHR presentmode);
			~RenderWindow();

			SwapchainImage getNextImage();

			std::shared_ptr<Device> device;
			std::shared_ptr<vkb::Instance> instance;

			vk::PresentModeKHR presentMode{ vk::PresentModeKHR::eFifo };
			vk::SurfaceKHR surface;
			vk::SwapchainKHR swapchain; 
			vk::Format swapchainImageFormat;
			std::vector<ImageHandle> swapchainImages;
			vk::Semaphore presentationSemaphore;
			u32 imagesInFlight{ 2 };
		};

	}
}
