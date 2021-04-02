#pragma once

#include "../Vulkan.hpp"

namespace daxa {
	namespace vkh {
		struct Buffer {
			VkBuffer buffer;
			VmaAllocation allocation;
		};
	}
}
