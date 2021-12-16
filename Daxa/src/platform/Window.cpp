#include "Window.hpp"

#include "../DaxaCore.hpp"

#include <iostream>

#include <VkBootstrap.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace daxa {
	Window::Window(std::string name, u32 width, u32 height) 
		: name{ name }
		, width{ width }
		, height{ height }
	{
		sdlWindowHandle = SDL_CreateWindow(
			name.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width,
			height,	
			SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
		);

		SDL_CaptureMouse(SDL_TRUE);

		if (!sdlWindowHandle) {
			std::cerr << SDL_GetError() << std::endl;
			exit(-1);
		}

		sdlWindowId = SDL_GetWindowID(sdlWindowHandle);

		auto ret = SDL_Vulkan_CreateSurface((SDL_Window*)sdlWindowHandle, gpu::instance->getVkInstance(), &surface);
		DAXA_ASSERT_M(ret == SDL_TRUE, "could not create window surface");
	}

	Window::~Window() 	{
		vkDestroySurfaceKHR(gpu::instance->getVkInstance(), surface, nullptr);
		SDL_DestroyWindow(sdlWindowHandle);
		sdlWindowHandle = nullptr;
	}

	const std::string& Window::getName() {
		return name;
	}

	bool Window::isFocused() const {
		auto mask = SDL_GetWindowFlags(sdlWindowHandle);
		return mask & SDL_WINDOW_INPUT_FOCUS;
	}

	bool Window::keyPressed(Scancode key) const {
		return !keyHidden[u32(key)] && (*keyStates)[u32(key)];
	}

	bool Window::keyJustPressed(Scancode key) const {
		return !keyHidden[u32(key)] && (*keyStates)[u32(key)] && !(*prevKeyStates)[u32(key)];
	}

	bool Window::keyReleased(Scancode key) const {
		return !keyHidden[u32(key)] && !(*keyStates)[u32(key)];
	}

	bool Window::keyJustReleased(Scancode key) const {
		return !keyHidden[u32(key)] && !(*keyStates)[u32(key)] && (*prevKeyStates)[u32(key)];
	}

	void Window::hideKey(Scancode key) {
		keyHidden[u32(key)] = true;
	}

	bool Window::buttonPressed(MouseButton button) const {
		return (*buttonStates)[u8(button)] && !buttonHidden[u8(button)];
	}

	bool Window::buttonJustPressed(MouseButton button) const {
		return !(*prevButtonStates)[u8(button)] && (*buttonStates)[u8(button)] && !buttonHidden[u8(button)];
	}

	bool Window::buttonReleased(MouseButton button) const {
		return !(*buttonStates)[u8(button)] && !buttonHidden[u8(button)];
	}

	bool Window::buttonJustReleased(MouseButton button) const {
		return (*prevButtonStates)[u8(button)] && !(*buttonStates)[u8(button)] && !buttonHidden[u8(button)];
	}

	void Window::hideButton(MouseButton button) {
		buttonHidden[u8(button)] = true;
	}

	bool Window::buttonPressedAndHide(MouseButton button) {
		auto ret = buttonPressed(button);
		hideButton(button);
		return ret;
	}

	bool Window::buttonJustPressedAndHide(MouseButton button) {
		auto ret = buttonJustPressed(button);
		hideButton(button);
		return ret;
	}

	bool Window::buttonReleasedAndHide(MouseButton button) {
		auto ret = buttonReleased(button);
		hideButton(button);
		return ret;
	}

	bool Window::buttonJustReleasedAndHide(MouseButton button) {
		auto ret = buttonJustReleased(button);
		hideButton(button);
		return ret;
	}

	i32 Window::getCursorPosX() const{
		return cursorPosX;
	}

	i32 Window::getCursorPosY() const {
		return cursorPosY;
	}

	f32 Window::getRelativeCursorPosX() const {
		return f32(cursorPosX) / f32(width) * 2.0f - 1.0f;
	}

	f32 Window::getRelativeCursorPosY() const {
		return f32(cursorPosY) / f32(height) * 2.0f - 1.0f;
	}

	i32 Window::getCursorPosChangeX() const {
		return (bCursorCaptured || isCursorOverWindow()) ? cursorPosChangeX : 0;
	}

	i32 Window::getCursorPosChangeY() const {
		return (bCursorCaptured || isCursorOverWindow()) ? cursorPosChangeY : 0;
	}

	f32 Window::getRelativeCursorPosChangeX() const {
		return (bCursorCaptured || isCursorOverWindow()) ? f32(cursorPosChangeX) * 2.0f - 1.0f : 0.f;
	}

	f32 Window::getRelativeCursorPosChangeY() const {
		return (bCursorCaptured || isCursorOverWindow()) ? f32(cursorPosChangeY) * 2.0f - 1.0f : 0.f;
	}

	bool Window::isCursorOverWindow() const {
		return cursorPosX >= 0 && cursorPosX < width && cursorPosY >= 0 && cursorPosY < height;
	}

	void Window::captureCursor() {
		SDL_SetWindowGrab(sdlWindowHandle, SDL_TRUE);
		SDL_ShowCursor(SDL_DISABLE);
		bCursorCaptured = true;
	}

	void Window::releaseCursor() {
		SDL_SetWindowGrab(sdlWindowHandle, SDL_FALSE);
		SDL_ShowCursor(SDL_ENABLE);
		bCursorCaptured = false;
	}

	bool Window::isCursorCaptured() const {
		return bCursorCaptured;
	}

	f32 Window::scrollX() const {
		return scrollXHidden ? 0.0f : m_scrollX;
	}

	f32 Window::scrollY() const {
		return scrollYHidden ? 0.0f : m_scrollY;
	}

	void Window::hideScrollX() {
		scrollXHidden = true;
	}

	void Window::hideScrollY() {
		scrollYHidden = true;
	}

	f32 Window::scrollXAndHide() {
		auto ret = scrollXHidden ? 0.0f : m_scrollX;
		hideScrollX();
		return ret;
	}

	f32 Window::scrollYAndHide() {
		auto ret = scrollYHidden ? 0.0f : m_scrollY;
		hideScrollY();
		return ret;
	}

	bool Window::update(f32 deltaTime) 	{
		bool close{ false };
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				close = true;
				break;
			case SDL_KEYDOWN:
				break;
			case SDL_WINDOWEVENT:
				{
				}
				break;
			}
		}

		scrollXHidden = false;
		scrollYHidden = false;

		// Keyboard
		std::swap(keyStates, prevKeyStates);
		const u8* keystate = SDL_GetKeyboardState(nullptr /* its 512 */);
		for (i32 i = 0; i < 512; ++i) {
			if (keystate[i]) {
			}
			(*keyStates)[i] = keystate[i];
		}
		for (i32 i = 0; i < 512; ++i) {
			keyHidden[i] = false;
		}

		// Mouse
		auto prevCursorPosX = cursorPosX;
		auto prevCursorPosY = cursorPosY;
		std::swap(buttonStates, prevButtonStates);
		const u32 buttonMask = SDL_GetMouseState(&cursorPosX, &cursorPosY);
		for (i32 i = 0; i < 5; ++i) {
			(*buttonStates)[i] = buttonMask & SDL_BUTTON(i);
		}
		for (i32 i = 0; i < 5; ++i) {
			buttonHidden[i] = false;
		}
		cursorPosChangeX = cursorPosX - prevCursorPosX;
		cursorPosChangeY = cursorPosY - prevCursorPosY;
		if (bCursorCaptured) {
			SDL_WarpMouseInWindow(sdlWindowHandle, width/2, height/2);
			cursorPosX = (i32)width/2;
			cursorPosY = (i32)height/2;
		}

		if (bChangedSize) {
			bChangedSize = false;
			SDL_SetWindowSize(sdlWindowHandle, width, height);
		}

		// Events
		SDL_GetWindowSize(sdlWindowHandle, (int*)&this->width, (int*)&this->height);

		if (!isFocused() && bCursorCaptured) {
			releaseCursor();
		}

		return close;
	}

	std::vector<KeyEvent> Window::getKeyEventsInOrder() const {
		std::vector<KeyEvent> ret;
		for (auto e : eventQ) {
			if (keyHidden[u32(e.scancode)]) {
				ret.push_back(e);
			}
		}
		return ret;
	}

	void* Window::getWindowHandleSDL() {
		return sdlWindowHandle;
	}

	VkSurfaceKHR Window::getSurface() {
		return surface;
	}
}