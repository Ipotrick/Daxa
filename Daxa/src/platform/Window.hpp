#pragma once

#include <mutex>
#include <array>
#include <string>
#include <vector>

#include "../DaxaCore.hpp"
#include "../rendering/Rendering.hpp"
#include "../math/Vec2.hpp"

#include "Scancodes.hpp"

struct SDL_Window;

namespace daxa {

	struct KeyEvent {
		Scancode scancode;
		enum class Type : u8 {
			Up,
			Down,
			Repeat
		};
		Type type;
	};

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

		bool isKeyPressed(Scancode key) const;
		bool isKeyJustPressed(Scancode key) const;
		bool isKeyReleased(Scancode key) const;
		bool isKeyJustReleased(Scancode key) const;
		void hideKey(Scancode key);

		std::array<i32,2> getCursorPosition() const; 
		Vec2 getCursorPositionVec() const;
		Vec2 getCursorPositionRelative() const;
		std::array<i32, 2> getCursorPositionChange() const;
		Vec2 getCursorPositionChangeVec() const;
		Vec2 getCursorPositionChangeRelative() const;
		void captureCursor();
		void releaseCursor();

	private:

		std::string name;
		std::array<u32, 2> size;

		SDL_Window* sdlWindowHandle{ nullptr };
		u32 sdlWindowId{ 0xFFFFFFFF };
		bool bCursorCaptured{ false };

		std::vector<KeyEvent> keyEvents;
		std::array<bool, 512> keyStates;
		std::array<bool, 512> prevKeyStates;
		std::array<bool, 512> keyHidden;

		std::array<i32, 2> cursorPos;
		std::array<i32, 2> prevCursorPos;
	public:

		bool bSpacePressed{ false };
		vk::PresentModeKHR presentMode{ vk::PresentModeKHR::eFifo };
		vk::Device vulkanDevice;
		vk::PhysicalDevice vulkanPhysicalDevice;
		vk::SurfaceKHR surface;
		vk::SwapchainKHR swapchain; // from other articles
		vk::Format swapchainImageFormat; // image format expected by the windowing system
		std::vector<vk::Image> swapchainImages; //array of images from the swapchain
		std::vector<vk::ImageView> swapchainImageViews; //array of image-views from the swapchain
		vk::Format depthImageFormat;
		Image depthImage;
	};
}
