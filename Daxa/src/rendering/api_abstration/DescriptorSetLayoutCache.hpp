#pragma once

#include "../../DaxaCore.hpp"

#include <unordered_map>

#include <vulkan/vulkan.h>
#include "../dependencies/vk_mem_alloc.hpp"

namespace daxa {
	namespace gpu {
		struct DescriptorLayoutHash {
			std::size_t operator()(std::vector<VkDescriptorSetLayoutBinding> const& bindings) const;
		};

		class DescriptorSetLayoutCache {
		public:
			DescriptorSetLayoutCache(VkDevice);
			~DescriptorSetLayoutCache();

			VkDescriptorSetLayout getLayout(std::vector<VkDescriptorSetLayoutBinding>& bindings);
		private:
			VkDevice device;
			std::unordered_map<std::vector<VkDescriptorSetLayoutBinding>, VkDescriptorSetLayout, DescriptorLayoutHash> bindingsToLayout;
		};
	}
}

bool operator ==(std::vector<VkDescriptorSetLayoutBinding> const& a, std::vector<VkDescriptorSetLayoutBinding> const& b);