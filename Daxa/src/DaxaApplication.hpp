#pragma once

#include "threading/OwningMutex.hpp"
#include "threading/Jobs.hpp"
#include "platform/Window.hpp"

namespace daxa {

	class Application {
	public:
		Application(std::string name, u32 width, u32 height);

		std::unique_ptr<OwningMutex<Window>> windowMutex; 
	private:
	};
}
