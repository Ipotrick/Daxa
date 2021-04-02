#pragma once

#include "../Vulkan.hpp"

namespace daxa {
	namespace vkh {
		VkFence makeFence(VkDevice device = vkh::mainDevice);

		VkSemaphore makeSemaphore(VkDevice device = vkh::mainDevice);

		VkRenderPass makeRenderPass(VkRenderPassCreateInfo info, VkDevice device = vkh::mainDevice);
	}
}
