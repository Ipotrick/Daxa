#pragma once

#include <memory>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

namespace gpu {

	class Pipeline {
	public:

	private:
		vkh::Pipeline pipeline;
	};

}
