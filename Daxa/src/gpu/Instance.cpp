#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"

#include <iostream>

namespace daxa {
	Instance::Instance(char const* AppName, char const* EngineName) {
		vkb::InstanceBuilder instanceBuilder;
		instanceBuilder
			.set_app_name(AppName)
			.set_engine_name(EngineName)
			.require_api_version(1, 2, 0); 

		//#if defined(_DEBUG)
			instanceBuilder
				.enable_validation_layers(true)
				.enable_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
				//.add_validation_feature_enable(VkValidationFeatureEnableEXT::VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT)
				//.enable_layer("VK_LAYER_LUNARG_monitor")
				.use_default_debug_messenger();
		//#endif

		auto ret = instanceBuilder.build();

		DAXA_ASSERT_M(ret.has_value(), "could not create Vulkan instance");

		instance = ret.value();

		//#if defined(_DEBUG)
			pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
			DAXA_ASSERT_M(pfnSetDebugUtilsObjectNameEXT != nullptr, "could not load function ptr for PFN_vkSetDebugUtilsObjectNameEXT");
		//#else
		//	pfnSetDebugUtilsObjectNameEXT = nullptr;
		//#endif
	}

	Instance::~Instance() {
		if (instance.instance) {
			vkb::destroy_instance(instance);
		}
	}

	std::unique_ptr<Instance> instance = nullptr;
}