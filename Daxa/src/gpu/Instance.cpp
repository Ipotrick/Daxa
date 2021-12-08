#include "Instance.hpp"

#include <iostream>

namespace daxa {
	namespace gpu {
		Instance::Instance(char const* AppName, char const* EngineName, bool enableValidationLayer) {
			vkb::InstanceBuilder instanceBuilder;
			instanceBuilder
				.set_app_name(AppName)
				.set_engine_name(EngineName)
				.require_api_version(1, 2, 0);

			if (enableValidationLayer) {
				instanceBuilder
					.request_validation_layers(true)
					.enable_layer("VK_LAYER_LUNARG_monitor")
					.use_default_debug_messenger();
			}

			auto ret = instanceBuilder.build();

			DAXA_ASSERT_M(ret.has_value(), "could not create Vulkan instance.");

			instance = ret.value();
		}

		Instance::~Instance() {
			vkDestroyInstance(instance.instance, nullptr);
		}

		Device Instance::createDevice() {
			return Device(instance);
		}
	}
}