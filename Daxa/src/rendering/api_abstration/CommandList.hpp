#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "../dependencies/vk_mem_alloc.hpp"
#include "../dependencies/vulkanhelper.hpp"

#include "../../DaxaCore.hpp"

namespace gpu {

	class CommandList {
	public:

		vk::CommandBuffer& operator * () {
			return cmd;
		}
	private:
		friend class Device;
		vk::CommandBuffer cmd;
	};

}
