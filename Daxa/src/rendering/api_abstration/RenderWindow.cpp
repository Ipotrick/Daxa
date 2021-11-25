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
			for (int i = 0; i < imagesInFlight; i++) {
				presentationFences.push_back(device->getVkDevice().createFence({}));
			}
			
			swapchainImageFormat = (vk::Format)vkbSwapchain.image_format;
		}

		RenderWindow::~RenderWindow() {
			if (device) {
				device->getVkDevice().destroySwapchainKHR(swapchain);
				for (int i = 0; i < imagesInFlight; i++) {
					device->getVkDevice().destroyImage(swapchainImages[i]);
					device->getVkDevice().destroyImageView(swapchainImageViews[i]);
					device->getVkDevice().destroyFence(presentationFences[i]);
				}
				swapchainImages.clear();
				swapchainImageViews.clear();
				presentationFences.clear();
				vkDestroySurfaceKHR(instance->instance, surface, nullptr);
				instance = nullptr;
				surface = nullptr;
				device = nullptr;
			}
		}

		std::tuple<vk::Image, vk::ImageView> RenderWindow::getNextImage() {
			auto index = imageIndex++ % 2;
			u32 dummy;
			device->getVkDevice().waitForFences(presentationFences[index], VK_TRUE, 10000000000);
			auto err = device->getVkDevice().acquireNextImageKHR(swapchain, 10000000000, nullptr, presentationFences[index], &dummy);
			assert(err == vk::Result::eSuccess);
			return { swapchainImages[index], swapchainImageViews[index] };
		}
	}
}
