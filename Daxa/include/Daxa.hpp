#pragma once

#include <Daxa/Application.hpp>
#include <Daxa/gpu/Device.hpp>
#include <Daxa/gpu/Instance.hpp>
#include <Daxa/DearImGuiImpl.hpp>
#include <Daxa/rendering/Cameras.hpp>
#include <Daxa/rendering/Mesh.hpp>
#include <Daxa/rendering/ImageCache.hpp>
#include <Daxa/entity/EntityComponentManager.hpp>
#include <Daxa/entity/DefaultComponents.hpp>

namespace daxa {
	void initialize();

	void cleanup();
}