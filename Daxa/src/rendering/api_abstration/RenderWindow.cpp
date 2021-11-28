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
			presentationSemaphore = device->getVkDevice().createSemaphore({});
			swapchainImageFormat = (vk::Format)vkbSwapchain.image_format;
		}

		RenderWindow::~RenderWindow() {
			if (device) {
				device->getVkDevice().destroySwapchainKHR(swapchain);
				for (int i = 0; i < imagesInFlight; i++) {
					device->getVkDevice().destroySemaphore(presentationSemaphore);
				}
				swapchainImages.clear();
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
			si.presentSemaphore = presentationSemaphore;
			return si;
		}
	}
}
