#pragma once

#include <mutex>
#include <array>
#include <string>
#include <vector>

#include "../DaxaCore.hpp"
#include "../gpu/Instance.hpp"

#include "Scancodes.hpp"

struct SDL_Window;

namespace daxa {

	enum class MouseButton : u8 {
		Left     = 0,
		Middle   = 1,
		Right    = 2,
		X1       = 3,
		X2       = 4
	};

	struct KeyEvent {
		Scancode scancode;
		enum class Type : u8 {
			JustPressed,
			Pressed,
			Released,
			JustReleased,
			Repeat
		};
		Type type;
	};

	class Window {
	public:
		Window(std::string name, u32 width, u32 height);
		~Window();
		Window(Window&&) = delete;

		bool update(f32 deltaTime);

		bool keyPressed(Scancode key) const;
		bool keyJustPressed(Scancode key) const;
		bool keyReleased(Scancode key) const;
		bool keyJustReleased(Scancode key) const;
		void hideKey(Scancode key);

		bool buttonPressed(MouseButton button) const;
		bool buttonJustPressed(MouseButton button) const;
		bool buttonReleased(MouseButton button) const;
		bool buttonJustReleased(MouseButton button) const;
		void hideButton(MouseButton button);

		bool buttonPressedAndHide(MouseButton button);
		bool buttonJustPressedAndHide(MouseButton button);
		bool buttonReleasedAndHide(MouseButton button);
		bool buttonJustReleasedAndHide(MouseButton button);

		f32 scrollX() const;
		f32 scrollY() const;
		void hideScrollX();
		void hideScrollY();
		f32 scrollXAndHide();
		f32 scrollYAndHide();

		i32 getCursorPosX() const;
		i32 getCursorPosY() const;
		i32 getCursorPosChangeX() const;
		i32 getCursorPosChangeY() const;
		f32 getRelativeCursorPosX() const;
		f32 getRelativeCursorPosY() const;
		f32 getRelativeCursorPosChangeX() const;
		f32 getRelativeCursorPosChangeY() const;
		bool isCursorOverWindow() const;
		void captureCursor();
		void releaseCursor();
		bool isCursorCaptured() const;

		std::vector<KeyEvent> getKeyEventsInOrder() const;

		bool isFocused() const;

		void setSize(u32 width, u32 height) { this->width = width; this->height = height; bChangedSize = true; }
		u32 getWidth() const { return width; }
		u32 getHeight() const { return height; }

		void setName(std::string name);
		std::string const& getName();

		void* getWindowHandleSDL();
		VkSurfaceKHR getSurface();
	private:
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		std::string name	= "";
		bool bChangedSize 	= false;
		u32 width 			= 100;
		u32 height 			= 100;

		SDL_Window* sdlWindowHandle	= {};
		u32 sdlWindowId				= 0xFFFFFFFF;
		bool bCursorCaptured		= false;
		bool bFocused 				= false;

		std::vector<KeyEvent> eventQ							= {};
		std::vector<KeyEvent> keyEvents							= {};
		std::unique_ptr<std::array<bool, 512>> keyStates 		= std::make_unique<std::array<bool, 512>>();
		std::unique_ptr < std::array<bool, 512>> prevKeyStates	= std::make_unique<std::array<bool, 512>>();
		std::array<bool, 512> keyHidden							= { false };
		std::unique_ptr < std::array<bool, 5>> buttonStates		= std::make_unique<std::array<bool, 5>>();
		std::unique_ptr < std::array<bool, 5>> prevButtonStates	= std::make_unique<std::array<bool, 5>>();
		std::array<bool, 5> buttonHidden						= { false };

		f32 m_scrollX		= 0.0f;
		f32 m_scrollY		= 0.0f;
		bool scrollXHidden	= false;
		bool scrollYHidden	= false;

		i32 cursorPosX 			= 0;
		i32 cursorPosY			= 0;
		i32 cursorPosChangeX 	= 0;
		i32 cursorPosChangeY 	= 0;
	};
}
