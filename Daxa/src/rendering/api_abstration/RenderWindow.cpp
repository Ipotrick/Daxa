#include "RenderWindow.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <VkBootstrap.hpp>

namespace daxa {
	namespace gpu {

		RenderWindow::RenderWindow(std::shared_ptr<Device> device, std::shared_ptr<vkb::Instance> instance, void* sdl_window_handle, u32 width, u32 height, vk::PresentModeKHR presentmode)
			: device{device}
			, instance{ instance }
		{
			SDL_Vulkan_CreateSurface((SDL_Window*)sdl_window_handle, instance->instance, (VkSurfaceKHR*)&surface);

			vkb::SwapchainBuilder swapchainBuilder{ device->getVkPhysicalDevice(), device->getVkDevice(), surface };

			vkb::Swapchain vkbSwapchain = swapchainBuilder
				.use_default_format_selection()
				.set_desired_present_mode((VkPresentModeKHR)presentmode)
				.set_desired_extent(width, height)
				.build()
				.value();
			
			//store swapchain and its related images
			swapchain = (vk::SwapchainKHR)vkbSwapchain.swapchain;
			auto vkImages = vkbSwapchain.get_images().value();
			for (VkImage& img : vkImages) {
				swapchainImages.push_back((vk::Image)img);
			}
			auto vkImageViews = vkbSwapchain.get_image_views().value();
			for (VkImageView& img : vkImageViews) {
				swapchainImageViews.push_back((vk::ImageView)img);
			}
			presentationSemaphore = device->getVkDevice().createSemaphore({});
			swapchainImageFormat = (vk::Format)vkbSwapchain.image_format;
		}

		RenderWindow::~RenderWindow() {
			if (device) {
				device->getVkDevice().destroySwapchainKHR(swapchain);
				for (int i = 0; i < imagesInFlight; i++) {
					device->getVkDevice().destroyImage(swapchainImages[i]);
					device->getVkDevice().destroyImageView(swapchainImageViews[i]);
					device->getVkDevice().destroySemaphore(presentationSemaphore);
				}
				swapchainImages.clear();
				swapchainImageViews.clear();
				vkDestroySurfaceKHR(instance->instance, surface, nullptr);
				instance = nullptr;
				surface = nullptr;
				device = nullptr;
			}
		}

		SwapchainImage RenderWindow::getNextImage() {
			u32 index{ 0 };
			auto err = device->getVkDevice().acquireNextImageKHR(swapchain, 10000000000, presentationSemaphore, nullptr, &index);
			assert(err == vk::Result::eSuccess);
			SwapchainImage si{};
			si.swapchain = swapchain;
			si.imageIndex = index;
			si.image = swapchainImages[index];
			si.imageView = swapchainImageViews[index];
			si.presentSemaphore = presentationSemaphore;
			return si;
		}
	}
}
