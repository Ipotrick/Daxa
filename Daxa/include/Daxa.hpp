#pragma once

#include "../src/Application.hpp"
#include "../src/gpu/Device.hpp"
#include "../src/gpu/Instance.hpp"
#include "../src/DearImGuiImpl.hpp"
#include "../src/rendering/Cameras.hpp"
#include "../src/rendering/Mesh.hpp"
#include "../src/entity/EntityComponentManager.hpp"

namespace daxa {
	void initialize();

	void cleanup();
}