#include "Window.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace daxa {
	Window::Window(std::string name, std::array<u32, 2> size) :
		name{ name }, size{ size }
	{
		SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_SHOWN);
		sdlWindowHandle = SDL_CreateWindow(
			this->name.c_str(), //window title
			SDL_WINDOWPOS_UNDEFINED, //window position x (don't care)
			SDL_WINDOWPOS_UNDEFINED, //window position y (don't care)
			this->size[0],  //window width in pixels
			this->size[1], //window height in pixels
			window_flags
		);
		sdlWindowId = SDL_GetWindowID(reinterpret_cast<SDL_Window*>(sdlWindowHandle));
	}
	Window::~Window()
	{
		if (sdlWindowHandle) {
			SDL_DestroyWindow(reinterpret_cast<SDL_Window*>(sdlWindowHandle));
		}
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
			if (event.window.windowID == sdlWindowId) {
				if (event.type = SDL_QUIT) {
					close = true;
				}
			}
		}
		return close;
	}
	bool Window::isFocused() const
	{
		return false;
	}
	void Window::swapBuffers()
	{
	}
	void* Window::getNativeHandle()
	{
		return sdlWindowHandle;
	}
}
