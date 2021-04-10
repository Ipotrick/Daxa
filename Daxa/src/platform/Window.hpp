#pragma once

#include <mutex>
#include <array>
#include <string>
#include <vector>

#include "../DaxaCore.hpp"
#include "../rendering/Rendering.hpp"
#include "../math/Vec2.hpp"

struct SDL_Window;

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
			vk::Device device,
			vk::PhysicalDevice physicalDevice
		);
		~Window();
		Window(Window&&) = delete;

		void setSize(std::array<u32, 2> size);
		std::array<u32,2> getSize() const;
		Vec2 getSizeVec() const;
		vk::Extent2D getExtent() const;

		void setName(std::string name);
		const std::string& getName();

		bool update(f32 deltaTime);

		bool isFocused() const;

		void swapBuffers();

		std::string name;
		std::array<u32, 2> size;

		SDL_Window* sdlWindowHandle{ nullptr };
		u32 sdlWindowId{ 0xFFFFFFFF };

		bool bSpacePressed{ false };
		vk::PresentModeKHR presentMode{ vk::PresentModeKHR::eFifo };
		vk::Device vulkanDevice;
		vk::PhysicalDevice vulkanPhysicalDevice;
		vk::SurfaceKHR surface;
		vk::SwapchainKHR swapchain; // from other articles
		vk::Format swapchainImageFormat; // image format expected by the windowing system
		std::vector<vk::Image> swapchainImages; //array of images from the swapchain
		std::vector<vk::ImageView> swapchainImageViews; //array of image-views from the swapchain
		Image depthImage;
		vk::UniqueImageView depthImageView;
		vk::Format depthImageFormat;
	};
}
