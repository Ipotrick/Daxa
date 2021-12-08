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

		RenderWindow::RenderWindow(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance, void* sdl_window_handle, u32 width, u32 height, VkPresentModeKHR presentMode, VkSwapchainKHR oldSwapchain, VkSurfaceKHR oldSurface)
			: device{ device }
			, physicalDevice{ physicalDevice }
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
				auto handle = ImageHandle{ std::make_shared<Image>() };
				swapchainImages.push_back(std::move(handle));
				auto& img = *swapchainImages.back();
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
				img.arrayLayers = 1;
				img.mipmapLevels = 1;
				printf("swapchain image ptr: %p\n", swapchainImages.back().image.get());
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
			printf("destroy render window\n");
			if (device) {
				printf("destroy render window REAL\n");
				for (auto& img : swapchainImages) {
					printf("ref count: %i\n", img.getRefCount());
				}
				swapchainImages.clear();
				vkDestroySwapchainKHR(device, swapchain, nullptr);
				if (surface) {
					vkDestroySurfaceKHR(instance, surface, nullptr);
				}
				vkDestroyFence(device, aquireFence, nullptr);
				device = nullptr;
				instance = nullptr;
				surface = nullptr;
				physicalDevice = nullptr;
				aquireFence = nullptr;
			}
		}

		SwapchainImage RenderWindow::aquireNextImage() {

			u32 index{ 0 };
			auto err = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, nullptr, aquireFence, &index);
			DAXA_ASSERT_M(err == VK_SUCCESS, "could not aquire next image from swapchain");
			SwapchainImage si{};
			si.swapchain = swapchain;
			si.imageIndex = index;
			si.image = swapchainImages[index];

			vkWaitForFences(device, 1, &aquireFence, VK_TRUE, UINT64_MAX);
			vkResetFences(device, 1, &aquireFence);

			return si;
		}

		void RenderWindow::resize(VkExtent2D newSize) {
			auto surface = this->surface;
			this->surface = VK_NULL_HANDLE;
			*this = RenderWindow{ device, physicalDevice, instance, sdl_window_handle, newSize.width, newSize.height, presentMode, this->swapchain, surface };
		}

		void RenderWindow::setPresentMode(VkPresentModeKHR newPresentMode) {
			auto surface = this->surface;
			this->surface = VK_NULL_HANDLE;
			*this = RenderWindow{ device, physicalDevice, instance, sdl_window_handle, size.width,  size.height, newPresentMode, this->swapchain, surface };
		}
	}
}
