#pragma once

#include "../../DaxaCore.hpp"
#include "Backend.hpp"

#include <memory>
#include <span>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "../GPUHandles.hpp"

namespace daxa {

	struct ImageViewBackend {
		size_t zombieReferences = {};
		ImageViewInfo info = {};
		VkImageView imageView = {};
		bool externView = false;

		void create(VkDevice device, ImageViewInfo const& info, VkImageView preCreatedView = VK_NULL_HANDLE);
		void destroy(VkDevice device);
	};
}