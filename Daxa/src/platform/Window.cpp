#include "Window.hpp"

#include <iostream>

#include <VkBootstrap.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace daxa {
	Window::Window(std::string name, std::array<u32, 2> size, VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice) :
		name{ name }, size{ size }, vulkanInstance{instance}, vulkanDevice{device}, vulkanPhysicalDevice{physicalDevice}
	{
		//create blank SDL window for our application
		sdlWindowHandle = SDL_CreateWindow(
			name.c_str(),				//window title
			SDL_WINDOWPOS_UNDEFINED,	//window position x (don't care)
			SDL_WINDOWPOS_UNDEFINED,	//window position y (don't care)
			size[0],					//window width in pixels
			size[1],					//window height in pixels
			SDL_WINDOW_VULKAN
		);

		if (!sdlWindowHandle) {
			std::cerr << SDL_GetError() << std::endl;
			exit(-1);
		}

		sdlWindowId = SDL_GetWindowID(sdlWindowHandle);

		SDL_Vulkan_CreateSurface(sdlWindowHandle, instance, &surface);

		vkb::SwapchainBuilder swapchainBuilder{ vulkanPhysicalDevice, vulkanDevice, surface };

		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.use_default_format_selection()
			//use vsync present mode
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(size[0], size[1])
			.build()
			.value();

		//store swapchain and its related images
		swapchain = vkbSwapchain.swapchain;
		swapchainImages = vkbSwapchain.get_images().value();
		swapchainImageViews = vkbSwapchain.get_image_views().value();

		swapchainImageFormat = vkbSwapchain.image_format;
	}
	Window::~Window()
	{
		vkDestroySwapchainKHR(vulkanDevice, swapchain, nullptr);

		//destroy swapchain resources
		for (int i = 0; i < swapchainImageViews.size(); i++) {

			vkDestroyImageView(vulkanDevice, swapchainImageViews[i], nullptr);
		}
		
		vkDestroySurfaceKHR(vulkanInstance, surface, nullptr);
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
		}
		return close;
	}
	bool Window::isFocused() const
	{
		return false;
	}
}
