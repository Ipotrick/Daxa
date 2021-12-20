#include "Swapchain.hpp"

#include <iostream>
#include <chrono>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <VkBootstrap.h>

namespace daxa {
	namespace gpu {
		VkSurfaceKHR createSurface(void* sdlWindowHandle, VkInstance instance) {
			VkSurfaceKHR surface;
			SDL_Vulkan_CreateSurface((SDL_Window*)sdlWindowHandle, instance, &surface);
			return surface;
		}

		void Swapchain::construct(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance, SwapchainCreateInfo ci) {
			this->device = device;
			this->physicalDevice = physicalDevice;
			this->instance = instance;
			this->size = { .width = ci.width, .height = ci.height };
			this->surface = ci.surface;
			this->presentMode = ci.presentMode;
			this->additionalimageUses = ci.additionalUses;

			vkb::SwapchainBuilder swapchainBuilder{ physicalDevice, device, surface };

			if (swapchain != VK_NULL_HANDLE) {
				swapchainBuilder.set_old_swapchain(swapchain);
			}

			vkb::Swapchain vkbSwapchain = swapchainBuilder
				//.use_default_format_selection()
				.set_desired_format({ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT })
				.set_desired_present_mode(presentMode)
				.set_desired_extent(ci.width, ci.height)
				.add_image_usage_flags(additionalimageUses)
				.build()
				.value();
			
			//store swapchain and its related images
			this->swapchain = vkbSwapchain.swapchain;
			auto vkImages = vkbSwapchain.get_images().value();
			auto vkImageViews = vkbSwapchain.get_image_views().value();
			for (int i = 0; i < vkImages.size(); i++) {
				auto handle = ImageHandle{ std::make_shared<Image>() };
				this->swapchainImages.push_back(std::move(handle));
				auto& img = *this->swapchainImages.back();
				img.extent.width = ci.width;
				img.extent.height = ci.height;
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
			}
			this->swapchainImageFormat = vkbSwapchain.image_format;

			VkFenceCreateInfo fenceCI{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
			};

			if (aquireFence == VK_NULL_HANDLE) {
				vkCreateFence(device, &fenceCI, nullptr, &this->aquireFence);
			}
		}

		Swapchain::~Swapchain() {
			swapchainImages.clear();
			if (device) {
				vkDestroySwapchainKHR(device, swapchain, nullptr);
				vkDestroyFence(device, aquireFence, nullptr);
				device = VK_NULL_HANDLE;
			}
		}

		SwapchainImage Swapchain::aquireNextImage() {
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

		void Swapchain::resize(VkExtent2D newSize) {
			swapchainImages.clear();
			construct(device, physicalDevice, instance, {surface, newSize.width, newSize.height, presentMode, additionalimageUses});
		}

		void Swapchain::setPresentMode(VkPresentModeKHR newPresentMode) {
			swapchainImages.clear();
			construct(device, physicalDevice, instance, {surface, size.width, size.height, newPresentMode, additionalimageUses});
		}
	}
}
