#pragma once

#include <mutex>
#include <array>
#include <string>

#include "../DaxaCore.hpp"

namespace daxa {
	/**
	 * Window abstraction class.
	 * Handles Input.
	 * threadsave (every functions locks).
	 */
	class Window {
	public:
		Window(std::string name, std::array<u32,2> size);
		~Window();

		void setSize(std::array<u32, 2> size);
		std::array<u32,2> getSize() const;

		void setName(std::string name);
		const std::string& getName();

		bool update(f32 deltaTime);

		bool isFocused() const;

		void swapBuffers();

		void* getNativeHandle();

	private:
		std::string name;
		std::array<u32, 2> size;

		void* sdlWindowHandle{ nullptr };
		u32 sdlWindowId{ 0xFFFFFFFF };
	};
}
