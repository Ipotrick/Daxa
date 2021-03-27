#pragma once

#include <mutex>

#include "threading/OwningMutex.hpp"
#include "platform/Window.hpp"

namespace daxa {

	class Application {
	public:
		Application(std::string name, u32 width, u32 height):
			windowMutex{ name, std::array<u32,2>{width, height} }
		{ }

		OwningMutex<Window> windowMutex; 
	private:
	};
}
