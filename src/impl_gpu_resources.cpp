#include "impl_gpu_resources.hpp"

namespace daxa
{
    void GPUResourceTable::initialize(usize max_buffers, usize max_images, usize max_samplers, VkDevice device)
    {
        buffer_slots.max_resources = max_buffers;
        image_slots.max_resources = max_images;
        sampler_slots.max_resources = max_samplers;

        VkDescriptorPoolSize buffer_descriptor_pool_size{
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = static_cast<u32>(buffer_slots.max_resources),
        };

        VkDescriptorPoolSize storage_image_descriptor_pool_size{
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = static_cast<u32>(image_slots.max_resources),
        };

        VkDescriptorPoolSize sampled_image_descriptor_pool_size{
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<u32>(image_slots.max_resources),
        };

        VkDescriptorPoolSize sampler_descriptor_pool_size{
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = static_cast<u32>(sampler_slots.max_resources),
        };

        VkDescriptorPoolSize pool_sizes[] = {
            buffer_descriptor_pool_size,
            storage_image_descriptor_pool_size,
            sampled_image_descriptor_pool_size,
            sampler_descriptor_pool_size,
        };

        VkDescriptorPoolCreateInfo vk_descriptor_pool_create_info_handle{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1,
            .poolSizeCount = 4u,
            .pPoolSizes = pool_sizes,
        };

        vkCreateDescriptorPool(device, &vk_descriptor_pool_create_info_handle, nullptr, &this->vk_descriptor_pool_handle);

        VkDescriptorSetLayoutBinding buffer_descriptor_set_layout_binding{
            .binding = BUFFER_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = static_cast<u32>(buffer_slots.max_resources),
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        VkDescriptorSetLayoutBinding storage_image_descriptor_set_layout_binding{
            .binding = STORAGE_IMAGE_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = static_cast<u32>(image_slots.max_resources),
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        VkDescriptorSetLayoutBinding sampled_image_descriptor_set_layout_binding{
            .binding = SAMPLED_IMAGE_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<u32>(image_slots.max_resources),
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        VkDescriptorSetLayoutBinding sampler_descriptor_set_layout_binding{
            .binding = SAMPLER_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = static_cast<u32>(sampler_slots.max_resources),
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        VkDescriptorSetLayoutBinding descriptor_set_layout_bindings[] = {
            buffer_descriptor_set_layout_binding,
            storage_image_descriptor_set_layout_binding,
            sampled_image_descriptor_set_layout_binding,
            sampler_descriptor_set_layout_binding};

        VkDescriptorBindingFlags vk_descriptor_binding_flags[] = {
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
        };
        VkDescriptorSetLayoutBindingFlagsCreateInfo vk_descriptor_set_layout_binding_flags_create_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = 4u,
            .pBindingFlags = vk_descriptor_binding_flags,
        };

        VkDescriptorSetLayoutCreateInfo vk_descriptor_set_layout_create_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &vk_descriptor_set_layout_binding_flags_create_info,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = 4,
            .pBindings = descriptor_set_layout_bindings,
        };

        vkCreateDescriptorSetLayout(device, &vk_descriptor_set_layout_create_info, nullptr, &this->vk_descriptor_set_layout_handle);

        VkDescriptorSetAllocateInfo vk_descriptor_set_allocate_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = this->vk_descriptor_pool_handle,
            .descriptorSetCount = 1,
            .pSetLayouts = &this->vk_descriptor_set_layout_handle,
        };

        vkAllocateDescriptorSets(device, &vk_descriptor_set_allocate_info, &this->vk_descriptor_set_handle);

        VkPipelineLayoutCreateInfo vk_pipeline_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .setLayoutCount = 1,
            .pSetLayouts = &this->vk_descriptor_set_layout_handle,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
        };

        vkCreatePipelineLayout(device, &vk_pipeline_create_info, nullptr, &pipeline_layouts[0]);

        for (usize i = 1; i < 7; ++i)
        {
            VkPushConstantRange vk_push_constant_range{
                .stageFlags = VK_SHADER_STAGE_ALL,
                .offset = 0,
                .size = static_cast<u32>((1ull << (i - 1ull)) * sizeof(u32)),
            };
            vk_pipeline_create_info.pushConstantRangeCount = 1;
            vk_pipeline_create_info.pPushConstantRanges = &vk_push_constant_range;
            vkCreatePipelineLayout(device, &vk_pipeline_create_info, nullptr, &pipeline_layouts[i]);
        }
    }

    void GPUResourceTable::cleanup(VkDevice device)
    {
        for (usize i = 0; i < 7; ++i)
        {
            vkDestroyPipelineLayout(device, pipeline_layouts[i], nullptr);
        }
        vkDestroyDescriptorSetLayout(device, this->vk_descriptor_set_layout_handle, nullptr);
        vkResetDescriptorPool(device, this->vk_descriptor_pool_handle, {});
        vkDestroyDescriptorPool(device, this->vk_descriptor_pool_handle, nullptr);
    }

    auto get_pipeline_layout_index_from_push_constant_size(usize push_constant_size) -> usize
    {
        return static_cast<usize>(std::ceil(std::log2(std::max(push_constant_size * 2ull, 1ull))));
    }
} // namespace daxa
