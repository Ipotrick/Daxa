#include "RenderWindow.hpp"

#include <iostream>
#include <chrono>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <VkBootstrap.hpp>

#include "common.hpp"

namespace daxa {
	namespace gpu {

		DAXA_DEFINE_TRIVIAL_MOVE(RenderWindow)

		RenderWindow::RenderWindow(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance, VkQueue graphicsQueue, void* sdl_window_handle, u32 width, u32 height, VkPresentModeKHR presentMode, VkSwapchainKHR oldSwapchain, VkSurfaceKHR oldSurface)
			: device{device}
			, physicalDevice{ physicalDevice }
			, graphicsQueue{ graphicsQueue }
			, instance{ instance }
			, sdl_window_handle{ sdl_window_handle }
			, size{ .width = width, .height = height }
			, presentMode{ presentMode }
		{
			if (!oldSwapchain) {
				SDL_Vulkan_CreateSurface((SDL_Window*)sdl_window_handle, instance, (VkSurfaceKHR*)&surface);
			}
			else {
				this->surface = oldSurface;
			}

			vkb::SwapchainBuilder swapchainBuilder{ physicalDevice, device, surface };

			if (oldSwapchain) {
				swapchainBuilder.set_old_swapchain(oldSwapchain);
			}

			vkb::Swapchain vkbSwapchain = swapchainBuilder
				.use_default_format_selection()
				.set_desired_present_mode(presentMode)
				.set_desired_extent(width, height)
				.build()
				.value();
			
			//store swapchain and its related images
			swapchain = vkbSwapchain.swapchain;
			auto vkImages = vkbSwapchain.get_images().value();
			auto vkImageViews = vkbSwapchain.get_image_views().value();
			for (int i = 0; i < vkImages.size(); i++) {
				swapchainImages.push_back(ImageHandle{ std::make_shared<Image>(Image{}) });
				auto& img = *swapchainImages.back();
				img.layout = VK_IMAGE_LAYOUT_UNDEFINED;
				img.extent.width = width;
				img.extent.height = height;
				img.extent.depth = 1;
				img.allocation = nullptr;
				img.allocator = nullptr;
				img.image = vkImages[i];
				img.view = vkImageViews[i];
				img.device = device;
				img.type = VK_IMAGE_TYPE_2D;
				img.viewFormat = vkbSwapchain.image_format;
				img.tiling = VK_IMAGE_TILING_OPTIMAL;
				img.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				img.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			swapchainImageFormat = vkbSwapchain.image_format;

			VkFenceCreateInfo fenceCI{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
			};
			vkCreateFence(device, &fenceCI, nullptr, &aquireFence);
		}

		RenderWindow::~RenderWindow() {
			if (device) {
				vkDestroySwapchainKHR(device, swapchain, nullptr);
				swapchainImages.clear();
				vkDestroySurfaceKHR(instance, surface, nullptr);
				vkDestroyFence(device, aquireFence, nullptr);
				instance = nullptr;
				surface = nullptr;
				device = nullptr;
				physicalDevice = nullptr;
				aquireFence = nullptr;
				graphicsQueue = nullptr;
			}
		}

		SwapchainImage RenderWindow::aquireNextImage() {

			u32 index{ 0 };
			auto err = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, nullptr, aquireFence, &index);
			assert(err == VK_SUCCESS);
			SwapchainImage si{};
			si.swapchain = swapchain;
			si.imageIndex = index;
			si.image = swapchainImages[index];

			vkWaitForFences(device, 1, &aquireFence, VK_TRUE, UINT64_MAX);
			vkResetFences(device, 1, &aquireFence);

			return si;
		}

		void RenderWindow::present(SwapchainImage&& image, SignalHandle waitOn) {
			assert(image.swapchain == swapchain);
			auto sema = waitOn->getVkSemaphore();
			VkPresentInfoKHR presentInfo{
				.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.pNext = nullptr,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &sema,
				.swapchainCount = 1,
				.pSwapchains = &image.swapchain,
				.pImageIndices = &image.imageIndex,
			};
			vkQueuePresentKHR(graphicsQueue, &presentInfo);
		}

		SwapchainImage RenderWindow::presentAquireNextImage(SwapchainImage&& image, SignalHandle waitOn) {
			present(std::move(image), waitOn);
			return aquireNextImage();
		}

		void RenderWindow::resize(VkExtent2D newSize) {
			*this = RenderWindow{ device, physicalDevice, instance, graphicsQueue, sdl_window_handle, newSize.width, newSize.height, presentMode, this->swapchain, this->surface };
		}

		void RenderWindow::setPresentMode(VkPresentModeKHR newPresentMode) {
			*this = RenderWindow{ device, physicalDevice, instance, graphicsQueue, sdl_window_handle, size.width,  size.height, newPresentMode, this->swapchain, this->surface };
		}
	}
}
