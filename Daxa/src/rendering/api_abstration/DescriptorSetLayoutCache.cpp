#include "DescriptorSetLayoutCache.hpp"

namespace daxa {
	namespace gpu {


		DescriptorSetLayoutCache::DescriptorSetLayoutCache(VkDevice device)
			:device{ device }
		{}

		DescriptorSetLayoutCache::~DescriptorSetLayoutCache() {
			if (device) {
				for (auto& [bindings, layout] : bindingsToLayout) {
					vkDestroyDescriptorSetLayout(device, layout, nullptr);
				}
				device = nullptr;
			}
		}

		std::size_t DescriptorLayoutHash::operator()(const std::vector<VkDescriptorSetLayoutBinding>& bindings) const {
			size_t h{ 0 };
			for (const auto& binding : bindings) {
				h ^= static_cast<size_t>(binding.descriptorType) << 10;
				h ^= static_cast<size_t>(binding.descriptorType) * 3 + 3123132;
			}
			return h;
		}

		VkDescriptorSetLayout DescriptorSetLayoutCache::getLayout(std::vector<VkDescriptorSetLayoutBinding>& bindings) {
			if (!bindingsToLayout.contains(bindings)) {
				VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
					.pNext = nullptr,
					.bindingCount = static_cast<uint32_t>(bindings.size()),
					.pBindings = bindings.data(),
				};
				vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr, &bindingsToLayout[bindings]);
			}
			return bindingsToLayout[bindings];
		}
	}
}

bool operator ==(std::vector<VkDescriptorSetLayoutBinding> const& a, std::vector<VkDescriptorSetLayoutBinding> const& b) {
	if (a.size() != b.size()) return false;
	for (int i = 0; i < a.size(); i++) {
		if (a[i].binding != b[i].binding) return false;
		if (a[i].descriptorCount != b[i].descriptorCount) return false;
		if (a[i].descriptorType != b[i].descriptorType) return false;
		if (a[i].pImmutableSamplers != b[i].pImmutableSamplers) return false;
		if (a[i].stageFlags != b[i].stageFlags) return false;
	}
	return true;
}