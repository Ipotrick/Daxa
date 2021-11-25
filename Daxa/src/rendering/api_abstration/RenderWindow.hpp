#pragma once

#include <memory>

#include <deque>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

#include "Device.hpp"

namespace daxa {
	namespace gpu {

		class RenderWindow {
		public:
			RenderWindow(std::shared_ptr<Device> device, std::shared_ptr<vkb::Instance> instance, void* sdl_window_handle, u32 width, u32 height, vk::PresentModeKHR presentmode);
			~RenderWindow();

			std::tuple<vk::Image, vk::ImageView> getNextImage();
		private:

			std::shared_ptr<Device> device;
			std::shared_ptr<vkb::Instance> instance;

			vk::PresentModeKHR presentMode{ vk::PresentModeKHR::eFifo };
			vk::SurfaceKHR surface;
			vk::SwapchainKHR swapchain; 
			vk::Format swapchainImageFormat;
			std::vector<vk::Image> swapchainImages; 
			std::vector<vk::ImageView> swapchainImageViews;
			std::vector<vk::Fence> presentationFences;
			u32 imageIndex{ 0 };
			u32 imagesInFlight{ 2 };
		};

	}
}
