#include "RenderWindow.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <VkBootstrap.hpp>

namespace daxa {
	namespace gpu {

		RenderWindow::RenderWindow(std::shared_ptr<Device> device, std::shared_ptr<vkb::Instance> instance, void* sdl_window_handle, u32 width, u32 height, VkPresentModeKHR presentMode, VkSwapchainKHR oldSwapchain, VkSurfaceKHR oldSurface)
			: device{device}
			, instance{ instance }
			, sdl_window_handle{ sdl_window_handle }
			, size{ .width = width, .height = height }
			, presentMode{ presentMode }
		{
			if (!oldSwapchain) {
				SDL_Vulkan_CreateSurface((SDL_Window*)sdl_window_handle, instance->instance, (VkSurfaceKHR*)&surface);
			}
			else {
				this->surface = oldSurface;
			}

			vkb::SwapchainBuilder swapchainBuilder{ device->getVkPhysicalDevice(), device->getVkDevice(), surface };

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
				img.device = device->getVkDevice();
				img.type = VK_IMAGE_TYPE_2D;
				img.viewFormat = vkbSwapchain.image_format;
				img.tiling = VK_IMAGE_TILING_OPTIMAL;
				img.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				img.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			swapchainImageFormat = vkbSwapchain.image_format;
		}

		RenderWindow::RenderWindow(RenderWindow&& other) {
			// trivial move
			std::memcpy(this, &other, sizeof(RenderWindow));
			std::memset(&other, 0, sizeof(RenderWindow));
		}

		RenderWindow& RenderWindow::operator=(RenderWindow&& other) {
			// trivial move
			std::memcpy(this, &other, sizeof(RenderWindow));
			std::memset(&other, 0, sizeof(RenderWindow));
			return *this;
		}

		RenderWindow::~RenderWindow() {
			if (device) {
				vkDestroySwapchainKHR(device->getVkDevice(), swapchain, nullptr);
				swapchainImages.clear();
				vkDestroySurfaceKHR(instance->instance, surface, nullptr);
				instance = nullptr;
				surface = nullptr;
				device = nullptr;
				printf("real destruction");
			}
		}

		SwapchainImage RenderWindow::getNextImage(VkSemaphore presentSemaphore) {
			VkExtent2D realSize;
			SDL_GetWindowSize((SDL_Window*)sdl_window_handle, (int*)&realSize.width, (int*)&realSize.height);
			if (realSize.width != size.width || realSize.height != size.height) {
				this->resize(realSize);
			}

			u32 index{ 0 };
			auto err = vkAcquireNextImageKHR(device->getVkDevice(), swapchain, UINT64_MAX, presentSemaphore, nullptr, &index);
			assert(err == VK_SUCCESS);
			SwapchainImage si{};
			si.swapchain = swapchain;
			si.imageIndex = index;
			si.image = swapchainImages[index];
			return si;
		}

		void RenderWindow::resize(VkExtent2D newSize) {
			device->waitIdle();
			*this = RenderWindow{ device, instance, sdl_window_handle, newSize.width, newSize.height, presentMode, this->swapchain, this->surface };
		}

		void RenderWindow::setPresentMode(VkPresentModeKHR newPresentMode) {
			device->waitIdle();
			*this = RenderWindow{ device, instance, sdl_window_handle, size.width,  size.height, newPresentMode, this->swapchain, this->surface };
		}
	}
}
