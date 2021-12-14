#pragma once

#include <mutex>
#include <array>
#include <string>
#include <vector>

#include "../DaxaCore.hpp"
#include "../math/Vec2.hpp"
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
		Window(
			std::string name,
			std::array<u32,2> size
		);
		~Window();
		Window(Window&&) = delete;

		void setSize(std::array<u32, 2> size);
		std::array<u32,2> getSize() const;
		Vec2 getSizeVec() const;

		void setName(std::string name);
		std::string const& getName();

		/**
		 * Intercepts certain window related Events like window close wich are abstracted in this class.
		 * 
		 * \param deltaTime time that passed since the last call.
		 * \return if the window should be closed (Exit pas pressed)
		 */
		bool update(f32 deltaTime);

		bool isFocused() const;

		bool isCursorCaptured() const;

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

		std::array<i32,2> getCursorPosition() const; 
		std::array<i32,2> getPrevCursorPosition() const; 
		Vec2 getCursorPositionVec() const;
		Vec2 getPrevCursorPositionVec() const;
		Vec2 getCursorPositionRelative() const;
		std::array<i32, 2> getCursorPositionChange() const;
		Vec2 getCursorPositionChangeVec() const;
		Vec2 getCursorPositionChangeRelative() const;
		void captureCursor();
		void releaseCursor();

		f32 scrollX() const;
		f32 scrollY() const;
		void hideScrollX();
		void hideScrollY();
		f32 scrollXAndHide();
		f32 scrollYAndHide();

		std::vector<KeyEvent> getKeyEventsInOrder() const;

		void* getWindowHandleSDL();
		VkSurfaceKHR getSurface();
	private:
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		std::string name;
		bool bChangedSize{ false };
		std::array<u32, 2> size;

		SDL_Window* sdlWindowHandle{ nullptr };
		u32 sdlWindowId{ 0xFFFFFFFF };
		bool bCursorCaptured{ false };

		std::vector<KeyEvent> keyEvents;
		std::unique_ptr<std::array<bool, 512>> keyStates{ std::make_unique<std::array<bool, 512>>() };
		std::unique_ptr < std::array<bool, 512>> prevKeyStates{ std::make_unique<std::array<bool, 512>>() };
		std::array<bool, 512> keyHidden;
		std::unique_ptr < std::array<bool, 5>> buttonStates{ std::make_unique<std::array<bool, 5>>() };
		std::unique_ptr < std::array<bool, 5>> prevButtonStates{ std::make_unique<std::array<bool, 5>>() };
		std::array<bool, 5> buttonHidden;
		f32 m_scrollX{ 0.0f };
		f32 m_scrollY{ 0.0f };
		bool scrollXHidden{ false };
		bool scrollYHidden{ false };
		std::vector<KeyEvent> eventQ;

		std::array<i32, 2> cursorPos;
		std::array<i32, 2> prevCursorPos;
	};
}
