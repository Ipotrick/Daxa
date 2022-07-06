#include "GPURessources.hpp"

namespace daxa {
    void createGPURessourceTable(VkDevice device, GPURessourceTable& table) {
        auto bindAllPoolSizes = std::array{
			BIND_ALL_SAMPLER_POOL_SIZE,
			BIND_ALL_COMBINED_IMAGE_SAMPLER_POOL_SIZE,
			BIND_ALL_SAMPLED_IMAGE_POOL_SIZE,
			BIND_ALL_STORAGE_IMAGE_POOL_SIZE,
			BIND_ALL_STORAGE_BUFFER_POOL_SIZE,
		};
		VkDescriptorPoolCreateInfo poolCI {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			//.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = 1,
			.poolSizeCount = (u32)bindAllPoolSizes.size(),
			.pPoolSizes = bindAllPoolSizes.data(),
		};
		DAXA_CHECK_VK_RESULT_M(vkCreateDescriptorPool(device, &poolCI, nullptr, &table.bindAllSetPool), "failed to create bind all set pool");
		auto bindAllSetDescriptorSetLayoutBindings = std::array {
			BIND_ALL_SAMPLER_SET_LAYOUT_BINDING,
			BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING,
			BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING,
			BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING,
			BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING,
		};
		auto bindAllSetLayoutBindingFlags = std::array<VkDescriptorBindingFlags, 5>{
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
		};
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindAllSetLayoutBindingFlagsCI {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.pNext = nullptr,
			.bindingCount = (u32)bindAllSetLayoutBindingFlags.size(),
			.pBindingFlags = bindAllSetLayoutBindingFlags.data(),
		};
		VkDescriptorSetLayoutCreateInfo bindAllSetLayoutCI {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &bindAllSetLayoutBindingFlagsCI,
			//.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = (u32)bindAllSetDescriptorSetLayoutBindings.size(),
			.pBindings = bindAllSetDescriptorSetLayoutBindings.data()
		};
		DAXA_CHECK_VK_RESULT_M(vkCreateDescriptorSetLayout(device, &bindAllSetLayoutCI, nullptr, &table.bindAllSetLayout), "failed to create bind all set layout");
		VkDescriptorSetAllocateInfo bindAllSetAI {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = table.bindAllSetPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &table.bindAllSetLayout,
		};
		DAXA_CHECK_VK_RESULT_M(vkAllocateDescriptorSets(device, &bindAllSetAI, &table.bindAllSet), "failed to create bind all set");
    }

    void destroyGPURessourceTable(VkDevice device, GPURessourceTable& table) {
        vkResetDescriptorPool(device, table.bindAllSetPool, 0);
        vkDestroyDescriptorPool(device, table.bindAllSetPool, nullptr);
        vkDestroyDescriptorSetLayout(device, table.bindAllSetLayout, nullptr);
    }
}