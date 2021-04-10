#include "Window.hpp"

#include <iostream>

#include <VkBootstrap.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace daxa {
	Window::Window(std::string name, std::array<u32, 2> size, vk::Device device, vk::PhysicalDevice physicalDevice) :
		name{ name }, size{ size }, vulkanDevice{device}, vulkanPhysicalDevice{physicalDevice}
	{
		//create blank SDL window for our application
		sdlWindowHandle = SDL_CreateWindow(
			name.c_str(),				// window title
			SDL_WINDOWPOS_UNDEFINED,	// window position x (don't care)
			SDL_WINDOWPOS_UNDEFINED,	// window position y (don't care)
			size[0],					// window width in pixels
			size[1],					// window height in pixels
			SDL_WINDOW_VULKAN
		);

		depthImageFormat = vk::Format::eD32Sfloat;
		
		depthImage = makeImage(
			vk::ImageCreateInfo{
				.imageType = vk::ImageType::e2D,
				.format = depthImageFormat,
				.extent = vk::Extent3D{.width=size[0],.height=size[1],.depth=1},
				.mipLevels = 1,
				.arrayLayers = 1,
				.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
			}
		);
		
		depthImageView = device.createImageViewUnique(vk::ImageViewCreateInfo{
			.image = depthImage.image,
			.viewType = vk::ImageViewType::e2D,
			.format = depthImageFormat,
			.subresourceRange = vk::ImageSubresourceRange {
				.aspectMask = vk::ImageAspectFlagBits::eDepth,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		});

		if (!sdlWindowHandle) {
			std::cerr << SDL_GetError() << std::endl;
			exit(-1);
		}

		sdlWindowId = SDL_GetWindowID(sdlWindowHandle);

		SDL_Vulkan_CreateSurface(sdlWindowHandle, vkh_old::instance, (VkSurfaceKHR*)&surface);

		vkb::SwapchainBuilder swapchainBuilder{ vulkanPhysicalDevice, vulkanDevice, surface };

		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.use_default_format_selection()
			//use vsync present mode
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(size[0], size[1])
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

		swapchainImageFormat = (vk::Format)vkbSwapchain.image_format;
	}
	Window::~Window()
	{
		vkDestroySwapchainKHR(vulkanDevice, swapchain, nullptr);

		//destroy swapchain resources
		for (int i = 0; i < swapchainImageViews.size(); i++) {

			vkDestroyImageView(vulkanDevice, swapchainImageViews[i], nullptr);
		}
		
		vkDestroySurfaceKHR(vkh_old::instance, surface, nullptr);
		SDL_DestroyWindow(sdlWindowHandle);
		sdlWindowHandle = nullptr;
	}
	void Window::setSize(std::array<u32, 2> size)
	{
		this->size = size;
	}
	std::array<u32, 2> Window::getSize() const
	{
		return size;
	}
	Vec2 Window::getSizeVec() const
	{
		return { static_cast<f32>(size[0]), static_cast<f32>(size[1]) };
	}
	vk::Extent2D Window::getExtent() const
	{
		return vk::Extent2D{size[0],size[1]};
	}
	void Window::setName(std::string name)
	{
		this->name = std::move(name);
	}
	const std::string& Window::getName()
	{
		return name;
	}
	bool Window::update(f32 deltaTime)
	{
		bool close{ false };
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				close = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_SPACE) {
					bSpacePressed ^= 1;
				}
			}
		}
		return close;
	}
	bool Window::isFocused() const
	{
		return false;
	}
}
