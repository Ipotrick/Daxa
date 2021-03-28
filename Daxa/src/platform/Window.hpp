#pragma once

#include <mutex>
#include <array>
#include <string>
#include <vector>

#include "../DaxaCore.hpp"
#include "../rendering/Vulkan.hpp"

namespace daxa {
	/**
	 * Window abstraction class.
	 * Handles Input.
	 * threadsave (every functions locks).
	 */
	class Window {
	public:
		Window(
			std::string name,
			std::array<u32,2> size,
			VkInstance instance,
			VkDevice device,
			VkPhysicalDevice physicalDevice
		);
		~Window();
		Window(Window&&) = delete;

		void setSize(std::array<u32, 2> size);
		std::array<u32,2> getSize() const;

		void setName(std::string name);
		const std::string& getName();

		bool update(f32 deltaTime);

		bool isFocused() const;

		void swapBuffers();

	private:
		std::string name;
		std::array<u32, 2> size;

		SDL_Window* sdlWindowHandle{ nullptr };
		u32 sdlWindowId{ 0xFFFFFFFF };

		VkPresentModeKHR presentMode{ VK_PRESENT_MODE_FIFO_KHR };
		VkInstance vulkanInstance{ VK_NULL_HANDLE };
		VkDevice vulkanDevice{ VK_NULL_HANDLE };
		VkPhysicalDevice vulkanPhysicalDevice{ VK_NULL_HANDLE };
		VkSurfaceKHR surface{ VK_NULL_HANDLE };
		VkSwapchainKHR swapchain{ VK_NULL_HANDLE }; // from other articles
		VkFormat swapchainImageFormat; // image format expected by the windowing system
		std::vector<VkImage> swapchainImages; //array of images from the swapchain
		std::vector<VkImageView> swapchainImageViews; //array of image-views from the swapchain
	};
}
