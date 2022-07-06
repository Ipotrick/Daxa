#pragma once

#include "../../DaxaCore.hpp"
#include "Backend.hpp"

#include <memory>
#include <span>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "../GPUHandles.hpp"

namespace daxa {

	struct SamplerBackend {
		size_t zombieReferences = {};
		SamplerInfo info = {};
		VkSampler sampler = {};

		void create(VkDevice device, SamplerInfo const& info);
		void destroy(VkDevice device);
	};
}