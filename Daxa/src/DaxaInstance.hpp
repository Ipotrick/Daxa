#pragma once

#include "DaxaCore.hpp"

#include "threading/OwningMutex.hpp"
#include "threading/Jobs.hpp"
#include "platform/Window.hpp"
#include "rendering/Rendering.hpp"
#include "rendering/Renderer.hpp"

namespace daxa {
	class Instance {
	public:
		OwningMutex<Window> windowMtx;
		Renderer renderer{ &windowMtx };
	private:
	};
}
