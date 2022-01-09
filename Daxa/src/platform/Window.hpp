#pragma once

#include <mutex>
#include <array>
#include <string>
#include <vector>

#include "../DaxaCore.hpp"

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

namespace daxa {

	struct WindowState {
		bool bCloseRequested						= false;
		std::array<bool, 5> mouseButtonDownOld 		= {};
		std::array<bool, 5> mouseButtonDown 		= {};
		std::array<bool, 512> KeyDown				= {};
		std::array<bool, 512> KeyDownOld			= {};
		i32 oldCursorPosX							= {};
		i32 oldCursorPosY							= {};
		i32 cursorChangeX							= {};
		i32 cursorChangeY							= {};
	};

	using Key = i32;
	using Button = i32;

	class Window {
	public:
		Window(std::string name, u32 width, u32 height);
		~Window();
		Window(Window&&) = delete;

		bool update(f32 deltaTime);

		bool keyPressed(Key key) const;
		bool keyJustPressed(Key key) const;
		bool keyJustReleased(Key key) const;

		bool buttonPressed(Button button) const;
		bool buttonJustPressed(Button button) const;
		bool buttonJustReleased(Button button) const;

		f32 scrollX() const;
		f32 scrollY() const;

		i32 getCursorPosX() const;
		i32 getCursorPosY() const;
		i32 getCursorPosChangeX() const;
		i32 getCursorPosChangeY() const;
		bool isCursorOverWindow() const;
		void captureCursor();
		void releaseCursor();
		bool isCursorCaptured() const;

		bool isFocused() const;

		void setWidth(u32 width);
		void setHeight(u32 height);
		u32 getWidth() const;
		u32 getHeight() const;

		void setName(std::string name);
		std::string const& getName();

		GLFWwindow* getGLFWWindow();
		VkSurfaceKHR_T* getSurface();
	private:
		VkSurfaceKHR_T* surface = 0;

		GLFWwindow* glfwWindow		= {};
		u32 glfwWindowId			= {};
		bool bCursorCaptured		= false;
		bool bFocused 				= false;
		std::string name 			= {};
		std::unique_ptr<WindowState> windowState = std::make_unique<WindowState>();

		i32 cursorPosChangeX 	= 0;
		i32 cursorPosChangeY 	= 0;
	};
}
