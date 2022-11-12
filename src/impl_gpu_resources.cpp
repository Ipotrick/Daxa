#include "impl_gpu_resources.hpp"

namespace daxa
{
    auto GPUResourceId::is_empty() const -> bool
    {
        return version == 0;
    }

    auto ImageId::default_view() const -> ImageViewId
    {
        return ImageViewId{{.index = index, .version = version}};
    }

    auto to_string(ImageId image_id) -> std::string
    {
        std::string ret = {};
        ret += " index: " + std::to_string(image_id.index);
        ret += " version: " + std::to_string(image_id.version);
        return ret;
    }

    auto to_string(types::BufferId buffer_id) -> std::string
    {
        std::string ret = {};
        ret += " index: " + std::to_string(buffer_id.index);
        ret += " version: " + std::to_string(buffer_id.version);
        return ret;
    }

    void GPUShaderResourceTable::initialize(usize max_buffers, usize max_images, usize max_samplers, usize /*max_timeline_query_pools*/,
                                      VkDevice device, VkBuffer device_address_buffer)
    {
        buffer_slots.max_resources = max_buffers;
        image_slots.max_resources = max_images;
        sampler_slots.max_resources = max_samplers;

        VkDescriptorPoolSize const buffer_descriptor_pool_size{
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = static_cast<u32>(buffer_slots.max_resources + 1),
        };

        VkDescriptorPoolSize const storage_image_descriptor_pool_size{
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = static_cast<u32>(image_slots.max_resources),
        };

        VkDescriptorPoolSize const sampled_image_descriptor_pool_size{
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<u32>(image_slots.max_resources),
        };

        VkDescriptorPoolSize const sampler_descriptor_pool_size{
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = static_cast<u32>(sampler_slots.max_resources),
        };

        std::array<VkDescriptorPoolSize, 4> const pool_sizes = {
            buffer_descriptor_pool_size,
            storage_image_descriptor_pool_size,
            sampled_image_descriptor_pool_size,
            sampler_descriptor_pool_size,
        };

        VkDescriptorPoolCreateInfo const vk_descriptor_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1,
            .poolSizeCount = static_cast<u32>(pool_sizes.size()),
            .pPoolSizes = pool_sizes.data(),
        };

        vkCreateDescriptorPool(device, &vk_descriptor_pool_create_info, nullptr, &this->vk_descriptor_pool);

        VkDescriptorSetLayoutBinding const buffer_descriptor_set_layout_binding{
            .binding = BUFFER_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = static_cast<u32>(buffer_slots.max_resources),
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        VkDescriptorSetLayoutBinding const storage_image_descriptor_set_layout_binding{
            .binding = STORAGE_IMAGE_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = static_cast<u32>(image_slots.max_resources),
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        VkDescriptorSetLayoutBinding const sampled_image_descriptor_set_layout_binding{
            .binding = SAMPLED_IMAGE_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<u32>(image_slots.max_resources),
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        VkDescriptorSetLayoutBinding const sampler_descriptor_set_layout_binding{
            .binding = SAMPLER_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = static_cast<u32>(sampler_slots.max_resources),
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        VkDescriptorSetLayoutBinding const buffer_address_buffer_descriptor_set_layout_binding{
            .binding = BUFFER_DEVICE_ADDRESS_BUFFER_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        };

        std::array<VkDescriptorSetLayoutBinding, 5> const descriptor_set_layout_bindings = {
            buffer_descriptor_set_layout_binding,
            storage_image_descriptor_set_layout_binding,
            sampled_image_descriptor_set_layout_binding,
            sampler_descriptor_set_layout_binding,
            buffer_address_buffer_descriptor_set_layout_binding,
        };

        std::array<VkDescriptorBindingFlags, 5> const vk_descriptor_binding_flags = {
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,

        };
        VkDescriptorSetLayoutBindingFlagsCreateInfo vk_descriptor_set_layout_binding_flags_create_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = static_cast<u32>(vk_descriptor_binding_flags.size()),
            .pBindingFlags = vk_descriptor_binding_flags.data(),
        };

        VkDescriptorSetLayoutCreateInfo const vk_descriptor_set_layout_create_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &vk_descriptor_set_layout_binding_flags_create_info,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = static_cast<u32>(descriptor_set_layout_bindings.size()),
            .pBindings = descriptor_set_layout_bindings.data(),
        };

        vkCreateDescriptorSetLayout(device, &vk_descriptor_set_layout_create_info, nullptr, &this->vk_descriptor_set_layout);

        VkDescriptorSetAllocateInfo const vk_descriptor_set_allocate_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = this->vk_descriptor_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &this->vk_descriptor_set_layout,
        };

        vkAllocateDescriptorSets(device, &vk_descriptor_set_allocate_info, &this->vk_descriptor_set);

        VkPipelineLayoutCreateInfo vk_pipeline_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .setLayoutCount = 1,
            .pSetLayouts = &this->vk_descriptor_set_layout,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
        };

        vkCreatePipelineLayout(device, &vk_pipeline_create_info, nullptr, pipeline_layouts.data());

        for (usize i = 1; i < PIPELINE_LAYOUT_COUNT; ++i)
        {
            VkPushConstantRange const vk_push_constant_range{
                .stageFlags = VK_SHADER_STAGE_ALL,
                .offset = 0,
                .size = static_cast<u32>(i * 4),
            };
            vk_pipeline_create_info.pushConstantRangeCount = 1;
            vk_pipeline_create_info.pPushConstantRanges = &vk_push_constant_range;
            vkCreatePipelineLayout(device, &vk_pipeline_create_info, nullptr, &pipeline_layouts.at(i));
        }

        VkDescriptorBufferInfo const write_buffer{
            .buffer = device_address_buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE,
        };
        

        VkWriteDescriptorSet const write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = this->vk_descriptor_set,
            .dstBinding = BUFFER_DEVICE_ADDRESS_BUFFER_BINDING,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &write_buffer,
            .pTexelBufferView = nullptr,
        };

        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }

    void GPUShaderResourceTable::cleanup(VkDevice device)
    {
        for (usize i = 0; i < PIPELINE_LAYOUT_COUNT; ++i)
        {
            vkDestroyPipelineLayout(device, pipeline_layouts.at(i), nullptr);
        }
        vkDestroyDescriptorSetLayout(device, this->vk_descriptor_set_layout, nullptr);
        vkResetDescriptorPool(device, this->vk_descriptor_pool, {});
        vkDestroyDescriptorPool(device, this->vk_descriptor_pool, nullptr);
    }

    void write_descriptor_set_sampler(VkDevice vk_device, VkDescriptorSet vk_descriptor_set, VkSampler vk_sampler, u32 index)
    {
        VkDescriptorImageInfo const vk_descriptor_image_info{
            .sampler = vk_sampler,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VkWriteDescriptorSet const vk_write_descriptor_set_storage{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = vk_descriptor_set,
            .dstBinding = SAMPLER_BINDING,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = &vk_descriptor_image_info,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr,
        };

        vkUpdateDescriptorSets(vk_device, 1, &vk_write_descriptor_set_storage, 0, nullptr);
    }

    void write_descriptor_set_buffer(VkDevice vk_device, VkDescriptorSet vk_descriptor_set, VkBuffer vk_buffer, VkDeviceSize offset, VkDeviceSize range, u32 index)
    {
        VkDescriptorBufferInfo const vk_descriptor_image_info{
            .buffer = vk_buffer,
            .offset = offset,
            .range = range,
        };

        VkWriteDescriptorSet const vk_write_descriptor_set{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = vk_descriptor_set,
            .dstBinding = BUFFER_BINDING,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &vk_descriptor_image_info,
            .pTexelBufferView = nullptr,
        };

        vkUpdateDescriptorSets(vk_device, 1, &vk_write_descriptor_set, 0, nullptr);
    }

    void write_descriptor_set_image(VkDevice vk_device, VkDescriptorSet vk_descriptor_set, VkImageView vk_image_view, ImageUsageFlags usage, u32 index)
    {
        u32 descriptor_set_write_count = 0;
        std::array<VkWriteDescriptorSet, 2> descriptor_set_writes = {};

        VkDescriptorImageInfo const vk_descriptor_image_info{
            .sampler = VK_NULL_HANDLE,
            .imageView = vk_image_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };

        VkWriteDescriptorSet const vk_write_descriptor_set{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = vk_descriptor_set,
            .dstBinding = STORAGE_IMAGE_BINDING,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &vk_descriptor_image_info,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr,
        };

        if ((usage & ImageUsageFlagBits::SHADER_READ_WRITE) != ImageUsageFlagBits::NONE)
        {
            descriptor_set_writes.at(descriptor_set_write_count++) = vk_write_descriptor_set;
        }

        VkDescriptorImageInfo const vk_descriptor_image_info_sampled{
            .sampler = VK_NULL_HANDLE,
            .imageView = vk_image_view,
            .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        };

        VkWriteDescriptorSet const vk_write_descriptor_set_sampled{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = vk_descriptor_set,
            .dstBinding = SAMPLED_IMAGE_BINDING,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &vk_descriptor_image_info_sampled,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr,
        };

        if ((usage & ImageUsageFlagBits::SHADER_READ_ONLY) != ImageUsageFlagBits::NONE)
        {
            descriptor_set_writes.at(descriptor_set_write_count++) = vk_write_descriptor_set_sampled;
        }

        vkUpdateDescriptorSets(vk_device, descriptor_set_write_count, descriptor_set_writes.data(), 0, nullptr);
    }
} // namespace daxa
