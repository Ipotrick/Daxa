#pragma once

#include "../DaxaCore.hpp"

#include <vulkan/vulkan.h>

#include <VkBootstrap.h>

#include "Device.hpp"

namespace daxa {
	namespace gpu {

		class Instance {
		public:
			Instance(char const* AppName = "Daxa Application", char const* EngineName = "Daxa", bool enableValidationLayer = true);
			~Instance();

			Device createDevice();
		private:
			vkb::Instance instance;
		};

	}
}