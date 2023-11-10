#include "impl_device.hpp"

#include <utility>
#include "impl_features.hpp"

#include "impl_device.hpp"

/// --- Begin Helpers ---

namespace
{
    auto initialize_image_create_info_from_image_info(daxa_ImageInfo const & image_info, u32 const * queue_family_index_ptr) -> VkImageCreateInfo
    {
        DAXA_DBG_ASSERT_TRUE_M(std::popcount(image_info.sample_count) == 1 && image_info.sample_count <= 64, "image samples must be power of two and between 1 and 64(inclusive)");
        DAXA_DBG_ASSERT_TRUE_M(
            image_info.size.width > 0 &&
                image_info.size.height > 0 &&
                image_info.size.depth > 0,
            "image (x,y,z) dimensions must be greater then 0");
        DAXA_DBG_ASSERT_TRUE_M(image_info.array_layer_count > 0, "image array layer count must be greater then 0");
        DAXA_DBG_ASSERT_TRUE_M(image_info.mip_level_count > 0, "image mip level count must be greater then 0");

        auto const vk_image_type = static_cast<VkImageType>(image_info.dimensions - 1);

        VkImageCreateFlags vk_image_create_flags = static_cast<VkImageCreateFlags>(image_info.flags);

        VkImageCreateInfo const vk_image_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = vk_image_create_flags,
            .imageType = vk_image_type,
            .format = static_cast<VkFormat>(image_info.format),
            .extent = std::bit_cast<VkExtent3D>(image_info.size),
            .mipLevels = image_info.mip_level_count,
            .arrayLayers = image_info.array_layer_count,
            .samples = static_cast<VkSampleCountFlagBits>(image_info.sample_count),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = image_info.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = queue_family_index_ptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        return vk_image_create_info;
    }
    using namespace daxa::types;
} // namespace

auto create_buffer_helper(daxa_Device self, daxa_BufferInfo const * info, daxa_BufferId * out_id, daxa_MemoryBlock opt_memory_block, usize opt_offset) -> daxa_Result
{
    // --- Begin Parameter Validation ---

    bool parameters_valid = true;
    // Size must be larger then one.
    parameters_valid = parameters_valid && info->size > 0;
    if (!parameters_valid)
        return DAXA_RESULT_INVALID_BUFFER_INFO;

    // --- End Parameter Validation ---

    auto slot_opt = self->gpu_sro_table.buffer_slots.try_create_slot();
    if (!slot_opt.has_value())
    {
        return DAXA_RESULT_EXCEEDED_MAX_BUFFERS;
    }
    auto [id, ret] = slot_opt.value();

    ret.info = *info;

    VkBufferCreateInfo const vk_buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .size = static_cast<VkDeviceSize>(ret.info.size),
        .usage = BUFFER_USE_FLAGS,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &self->main_queue_family_index,
    };

    bool host_accessible = false;
    VmaAllocationInfo vma_allocation_info = {};
    if (opt_memory_block == nullptr)
    {
        auto vma_allocation_flags = static_cast<VmaAllocationCreateFlags>(info->allocate_info);
        if (((vma_allocation_flags & VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT) != 0u) ||
            ((vma_allocation_flags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) != 0u) ||
            ((vma_allocation_flags & VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) != 0u))
        {
            vma_allocation_flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
            host_accessible = true;
        }

        VmaAllocationCreateInfo const vma_allocation_create_info{
            .flags = vma_allocation_flags,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        auto result = vmaCreateBuffer(
            self->vma_allocator,
            &vk_buffer_create_info,
            &vma_allocation_create_info,
            &ret.vk_buffer,
            &ret.vma_allocation,
            &vma_allocation_info);
        if (result != VK_SUCCESS)
        {
            return std::bit_cast<daxa_Result>(result);
        }
    }
    else
    {
        auto const & mem_block = *opt_memory_block;
        ret.opt_memory_block = opt_memory_block;
        opt_memory_block->inc_weak_refcnt();

        // TODO(pahrens): Add validation for memory type requirements.

        auto result = vkCreateBuffer(self->vk_device, &vk_buffer_create_info, nullptr, &ret.vk_buffer);
        if (result != VK_SUCCESS)
        {
            return std::bit_cast<daxa_Result>(result);
        }

        result = vmaBindBufferMemory2(
            self->vma_allocator,
            mem_block.allocation,
            opt_offset,
            ret.vk_buffer,
            {});
        if (result != VK_SUCCESS)
        {
            vkDestroyBuffer(self->vk_device, ret.vk_buffer, nullptr);
            return std::bit_cast<daxa_Result>(result);
        }
    }

    VkBufferDeviceAddressInfo const vk_buffer_device_address_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = ret.vk_buffer,
    };

    ret.device_address = vkGetBufferDeviceAddress(self->vk_device, &vk_buffer_device_address_info);

    ret.host_address = host_accessible ? vma_allocation_info.pMappedData : nullptr;

    self->buffer_device_address_buffer_host_ptr[id.index] = ret.device_address;

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE &&
        info->name.size != 0)
    {
        auto c_str_arr = r_cast<SmallString const *>(&info->name)->c_str();
        VkDebugUtilsObjectNameInfoEXT const buffer_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = std::bit_cast<uint64_t>(ret.vk_buffer),
            .pObjectName = c_str_arr.data(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &buffer_name_info);
    }

    write_descriptor_set_buffer(
        self->vk_device,
        self->gpu_sro_table.vk_descriptor_set, ret.vk_buffer,
        0,
        static_cast<VkDeviceSize>(ret.info.size),
        id.index);

    *out_id = std::bit_cast<daxa_BufferId>(id);
    return DAXA_RESULT_SUCCESS;
}

auto create_image_helper(daxa_Device self, daxa_ImageInfo const * info, daxa_ImageId * out_id, daxa_MemoryBlock opt_memory_block, usize opt_offset) -> daxa_Result
{
    /// --- Begin Validation ---

    if (!(info->dimensions >= 1 && info->dimensions <= 3))
    {
        return DAXA_RESULT_INVALID_IMAGE_INFO;
    }

    /// --- End Validation ---

    auto slot_opt = self->gpu_sro_table.image_slots.try_create_slot();
    if (!slot_opt.has_value())
    {
        return DAXA_RESULT_EXCEEDED_MAX_IMAGES;
    }
    auto [id, ret] = slot_opt.value();

    ret.info = *info;
    ret.view_slot.info = std::bit_cast<daxa_ImageViewInfo>(ImageViewInfo{
        .type = static_cast<ImageViewType>(info->dimensions - 1),
        .format = std::bit_cast<Format>(ret.info.format),
        .image = {id},
        .slice = ImageMipArraySlice{
            .base_mip_level = 0,
            .level_count = info->mip_level_count,
            .base_array_layer = 0,
            .layer_count = info->array_layer_count,
        },
        .name = info->name.data,
    });

    VkImageViewType vk_image_view_type = {};
    if (info->array_layer_count > 1)
    {
        vk_image_view_type = static_cast<VkImageViewType>(info->dimensions + 3);
    }
    else
    {
        vk_image_view_type = static_cast<VkImageViewType>(info->dimensions - 1);
    }

    ret.aspect_flags = infer_aspect_from_format(info->format);
    VkImageViewCreateInfo vk_image_view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        // .image = ret.vk_image, // FILL THIS LATER!
        .viewType = vk_image_view_type,
        .format = *r_cast<VkFormat const *>(&info->format),
        .components = VkComponentMapping{
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange = {
            .aspectMask = ret.aspect_flags,
            .baseMipLevel = 0,
            .levelCount = info->mip_level_count,
            .baseArrayLayer = 0,
            .layerCount = info->array_layer_count,
        },
    };
    VkImageCreateInfo const vk_image_create_info = initialize_image_create_info_from_image_info(*info, &self->main_queue_family_index);
    if (opt_memory_block == nullptr)
    {
        VmaAllocationCreateInfo const vma_allocation_create_info{
            .flags = static_cast<VmaAllocationCreateFlags>(info->allocate_info),
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        auto result = vmaCreateImage(self->vma_allocator, &vk_image_create_info, &vma_allocation_create_info, &ret.vk_image, &ret.vma_allocation, nullptr);
        if (result != VK_SUCCESS)
        {
            self->gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(id);
            return DAXA_RESULT_FAILED_TO_CREATE_IMAGE;
        }

        vk_image_view_create_info.image = ret.vk_image;
        result = vkCreateImageView(self->vk_device, &vk_image_view_create_info, nullptr, &ret.view_slot.vk_image_view);
        if (result != VK_SUCCESS)
        {
            vmaDestroyImage(self->vma_allocator, ret.vk_image, ret.vma_allocation);
            self->gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(id);
            return DAXA_RESULT_FAILED_TO_CREATE_DEFAULT_IMAGE_VIEW;
        }
    }
    else
    {
        daxa_ImplMemoryBlock const & mem_block = *opt_memory_block;
        ret.opt_memory_block = opt_memory_block;
        opt_memory_block->inc_weak_refcnt();
        // TODO(pahrens): Add validation for memory requirements.
        auto result = vkCreateImage(self->vk_device, &vk_image_create_info, nullptr, &ret.vk_image);
        if (result != VK_SUCCESS)
        {
            self->gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(id);
            return DAXA_RESULT_FAILED_TO_CREATE_IMAGE;
        }
        result = vmaBindImageMemory2(
            self->vma_allocator,
            mem_block.allocation,
            opt_offset,
            ret.vk_image,
            {});
        if (result != VK_SUCCESS)
        {
            vkDestroyImage(self->vk_device, ret.vk_image, nullptr);
            self->gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(id);
            return DAXA_RESULT_FAILED_TO_CREATE_IMAGE;
        }

        vk_image_view_create_info.image = ret.vk_image;
        result = vkCreateImageView(self->vk_device, &vk_image_view_create_info, nullptr, &ret.view_slot.vk_image_view);
        if (result != VK_SUCCESS)
        {

            vkDestroyImage(self->vk_device, ret.vk_image, nullptr);
            self->gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(id);
            return DAXA_RESULT_FAILED_TO_CREATE_DEFAULT_IMAGE_VIEW;
        }
    }

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && info->name.size != 0)
    {
        auto c_str_arr = r_cast<SmallString const *>(&info->name)->c_str();
        VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE,
            .objectHandle = std::bit_cast<uint64_t>(ret.vk_image),
            .pObjectName = c_str_arr.data(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &swapchain_image_name_info);

        VkDebugUtilsObjectNameInfoEXT const swapchain_image_view_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
            .objectHandle = std::bit_cast<uint64_t>(ret.view_slot.vk_image_view),
            .pObjectName = c_str_arr.data(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &swapchain_image_view_name_info);
    }

    write_descriptor_set_image(
        self->vk_device,
        self->gpu_sro_table.vk_descriptor_set,
        ret.view_slot.vk_image_view,
        std::bit_cast<ImageUsageFlags>(ret.info.usage),
        id.index);

    *out_id = std::bit_cast<daxa_ImageId>(id);
    return DAXA_RESULT_SUCCESS;
}

auto create_acceleration_structure_helper(
    daxa_Device self,
    daxa_AccelerationStructureInfo const * info,
    daxa_BufferId const * buffer,
    u64 const * offset,
    daxa_AccelerationStructureId * out_id) -> daxa_Result
{
    // --- Begin Parameter Validation ---

    // --- End Parameter Validation ---

    auto slot_opt = self->gpu_sro_table.acceleration_structure_slots.try_create_slot();
    if (!slot_opt.has_value())
    {
        return DAXA_RESULT_EXCEEDED_MAX_ACCELERATION_STRUCTURES;
    }
    auto [id, ret] = slot_opt.value();

    ret.info = *info;

    if (buffer)
    {
        ret.buffer_id = *buffer;
        ret.offset = *offset;
    }
    else
    {
        daxa::SmallString buffer_name{std::string_view{info->name.data, info->name.size}};
        if (info->name.size < SMALL_STRING_CHAR_MAX)
            buffer_name.push_back(' ');
        if (info->name.size < SMALL_STRING_CHAR_MAX)
            buffer_name.push_back('b');
        if (info->name.size < SMALL_STRING_CHAR_MAX)
            buffer_name.push_back('u');
        if (info->name.size < SMALL_STRING_CHAR_MAX)
            buffer_name.push_back('f');
        auto cinfo = daxa_BufferInfo{
            .name = std::bit_cast<daxa_SmallString>(buffer_name)};
        auto result = daxa_dvc_create_buffer(self, &cinfo, &ret.buffer_id);
        if (result != DAXA_RESULT_SUCCESS)
        {
            return result;
        }
        ret.offset = 0;
    }
    ret.vk_buffer = self->slot(ret.buffer_id).vk_buffer;

    [[maybe_unused]] daxa_Result safely_ignorable = daxa_dvc_buffer_device_address(self, ret.buffer_id, &ret.bda);

    VkAccelerationStructureCreateInfoKHR vk_create_info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR,
        .buffer = self->slot(ret.buffer_id).vk_buffer,
        .offset = ret.offset,
        .size = ret.info.size,
        .type = std::bit_cast<VkAccelerationStructureTypeKHR>(ret.info.type),
        .deviceAddress = std::bit_cast<VkDeviceAddress>(ret.bda),
    };
    auto vk_result = self->vkCreateAccelerationStructureKHR(self->vk_device, &vk_create_info, nullptr, &ret.vk_as);
    if (vk_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(vk_result);
    }

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && info->name.size != 0)
    {
        auto c_str_arr = r_cast<SmallString const *>(&info->name)->c_str();
        VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR,
            .objectHandle = std::bit_cast<uint64_t>(ret.vk_as),
            .pObjectName = c_str_arr.data(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &swapchain_image_name_info);
    }

    write_descriptor_set_acceleration_structure(
        self->vk_device,
        self->gpu_sro_table.vk_descriptor_set,
        ret.vk_as,
        id.index);

    *out_id = std::bit_cast<daxa_AccelerationStructureId>(id);
    return DAXA_RESULT_SUCCESS;
}

/// --- End Helpers ---

// --- Begin API Functions ---

auto daxa_default_device_score(daxa_DeviceProperties const * c_properties) -> i32
{
    DeviceProperties const * properties = r_cast<DeviceProperties const *>(c_properties);
    i32 score = 0;
    switch (properties->device_type)
    {
    case daxa::DeviceType::DISCRETE_GPU: score += 10000; break;
    case daxa::DeviceType::VIRTUAL_GPU: score += 1000; break;
    case daxa::DeviceType::INTEGRATED_GPU: score += 100; break;
    default: break;
    }
    return score;
}

auto daxa_dvc_buffer_memory_requirements(daxa_Device self, daxa_BufferInfo const * info) -> VkMemoryRequirements
{
    VkBufferCreateInfo const vk_buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .size = static_cast<VkDeviceSize>(info->size),
        .usage = BUFFER_USE_FLAGS,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &self->main_queue_family_index,
    };
    VkDeviceBufferMemoryRequirements buffer_requirement_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS,
        .pNext = {},
        .pCreateInfo = &vk_buffer_create_info,
    };
    VkMemoryRequirements2 mem_requirements = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        .pNext = {},
        .memoryRequirements = {},
    };
    vkGetDeviceBufferMemoryRequirements(self->vk_device, &buffer_requirement_info, &mem_requirements);
    // MemoryRequirements ret = std::bit_cast<MemoryRequirements>(mem_requirements.memoryRequirements);
    return mem_requirements.memoryRequirements;
}

auto daxa_dvc_image_memory_requirements(daxa_Device self, daxa_ImageInfo const * info) -> VkMemoryRequirements
{
    VkImageCreateInfo vk_image_create_info = initialize_image_create_info_from_image_info(*info, &self->main_queue_family_index);
    VkDeviceImageMemoryRequirements image_requirement_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS,
        .pNext = {},
        .pCreateInfo = &vk_image_create_info,
        .planeAspect = static_cast<VkImageAspectFlagBits>(infer_aspect_from_format(info->format)),
    };
    VkMemoryRequirements2 mem_requirements{
        .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        .pNext = {},
        .memoryRequirements = {},
    };
    vkGetDeviceImageMemoryRequirements(self->vk_device, &image_requirement_info, &mem_requirements);
    return mem_requirements.memoryRequirements;
}

auto daxa_dvc_create_buffer(daxa_Device self, daxa_BufferInfo const * info, daxa_BufferId * out_id) -> daxa_Result
{
    return create_buffer_helper(self, info, out_id, nullptr, 0);
}

auto daxa_dvc_create_image(daxa_Device self, daxa_ImageInfo const * info, daxa_ImageId * out_id) -> daxa_Result
{
    return create_image_helper(self, info, out_id, nullptr, 0);
}

auto daxa_dvc_create_buffer_from_memory_block(daxa_Device self, daxa_MemoryBlockBufferInfo const * info, daxa_BufferId * out_id) -> daxa_Result
{
    return create_buffer_helper(self, &info->buffer_info, out_id, *info->memory_block, info->offset);
}

auto daxa_dvc_create_image_from_block(daxa_Device self, daxa_MemoryBlockImageInfo const * info, daxa_ImageId * out_id) -> daxa_Result
{
    return create_image_helper(self, &info->image_info, out_id, *info->memory_block, info->offset);
}

auto daxa_dvc_create_acceleration_structure(daxa_Device device, daxa_AccelerationStructureInfo const * info, daxa_AccelerationStructureId * out_id) -> daxa_Result
{
    return create_acceleration_structure_helper(device, info, nullptr, nullptr, out_id);
}

auto daxa_dvc_create_acceleration_structure_from_buffer(daxa_Device device, daxa_BufferAccelerationStructureInfo const * info, daxa_AccelerationStructureId * out_id) -> daxa_Result
{
    return create_acceleration_structure_helper(device, &info->acceleration_structure_info, &info->buffer_id, &info->offset, out_id);
}

auto daxa_dvc_create_image_view(daxa_Device self, daxa_ImageViewInfo const * info, daxa_ImageViewId * out_id) -> daxa_Result
{
    /// --- Begin Validation ---

    /// --- End Validation ---

    auto slot_opt = self->gpu_sro_table.image_slots.try_create_slot();
    if (!slot_opt.has_value())
    {
        return DAXA_RESULT_EXCEEDED_MAX_IMAGE_VIEWS;
    }
    auto [id, image_slot] = slot_opt.value();

    ImplImageSlot const & parent_image_slot = self->slot(info->image);
    image_slot = {};
    auto & ret = image_slot.view_slot;
    ret.info = *info;
    daxa_ImageMipArraySlice slice = self->validate_image_slice(ret.info.slice, ret.info.image);
    ret.info.slice = slice;
    VkImageViewCreateInfo const vk_image_view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .image = parent_image_slot.vk_image,
        .viewType = static_cast<VkImageViewType>(ret.info.type),
        .format = *r_cast<VkFormat const *>(&ret.info.format),
        .components = VkComponentMapping{
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange = make_subresource_range(slice, parent_image_slot.aspect_flags),
    };
    auto result = vkCreateImageView(self->vk_device, &vk_image_view_create_info, nullptr, &ret.vk_image_view);
    if (result != VK_SUCCESS)
    {
        self->gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(id);
        return DAXA_RESULT_FAILED_TO_CREATE_IMAGE_VIEW;
    }
    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && info->name.size != 0)
    {
        auto c_str_arr = r_cast<SmallString const *>(&info->name)->c_str();
        VkDebugUtilsObjectNameInfoEXT const name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
            .objectHandle = std::bit_cast<uint64_t>(ret.vk_image_view),
            .pObjectName = c_str_arr.data(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &name_info);
    }
    write_descriptor_set_image(
        self->vk_device,
        self->gpu_sro_table.vk_descriptor_set,
        ret.vk_image_view,
        std::bit_cast<ImageUsageFlags>(parent_image_slot.info.usage),
        id.index);
    *out_id = std::bit_cast<daxa_ImageViewId>(id);
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_create_sampler(daxa_Device self, daxa_SamplerInfo const * info, daxa_SamplerId * out_id) -> daxa_Result
{
    /// --- Begin Validation ---

    if (!(info->mipmap_filter != VkFilter::VK_FILTER_CUBIC_IMG))
    {
        return DAXA_RESULT_INVALID_SAMPLER_INFO;
    }

    /// --- End Validation ---
    auto slot_opt = self->gpu_sro_table.sampler_slots.try_create_slot();
    if (!slot_opt.has_value())
    {
        return DAXA_RESULT_EXCEEDED_MAX_SAMPLERS;
    }
    auto [id, ret] = slot_opt.value();

    ret.info = *info;

    VkSamplerReductionModeCreateInfo vk_sampler_reduction_mode_create_info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO,
        .pNext = nullptr,
        .reductionMode = static_cast<VkSamplerReductionMode>(ret.info.reduction_mode),
    };

    VkSamplerCreateInfo const vk_sampler_create_info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = r_cast<void *>(&vk_sampler_reduction_mode_create_info),
        .flags = {},
        .magFilter = static_cast<VkFilter>(ret.info.magnification_filter),
        .minFilter = static_cast<VkFilter>(ret.info.minification_filter),
        .mipmapMode = static_cast<VkSamplerMipmapMode>(ret.info.mipmap_filter),
        .addressModeU = static_cast<VkSamplerAddressMode>(ret.info.address_mode_u),
        .addressModeV = static_cast<VkSamplerAddressMode>(ret.info.address_mode_v),
        .addressModeW = static_cast<VkSamplerAddressMode>(ret.info.address_mode_w),
        .mipLodBias = ret.info.mip_lod_bias,
        .anisotropyEnable = static_cast<VkBool32>(ret.info.enable_anisotropy),
        .maxAnisotropy = ret.info.max_anisotropy,
        .compareEnable = static_cast<VkBool32>(ret.info.enable_compare),
        .compareOp = static_cast<VkCompareOp>(ret.info.compare_op),
        .minLod = ret.info.min_lod,
        .maxLod = ret.info.max_lod,
        .borderColor = static_cast<VkBorderColor>(ret.info.border_color),
        .unnormalizedCoordinates = static_cast<VkBool32>(ret.info.enable_unnormalized_coordinates),
    };

    auto result = vkCreateSampler(self->vk_device, &vk_sampler_create_info, nullptr, &ret.vk_sampler);
    if (result != VK_SUCCESS)
    {
        self->gpu_sro_table.sampler_slots.unsafe_destroy_zombie_slot(id);
        return DAXA_RESULT_FAILED_TO_CREATE_SAMPLER;
    }

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && info->name.size != 0)
    {
        auto c_str_arr = r_cast<SmallString const *>(&info->name)->c_str();
        VkDebugUtilsObjectNameInfoEXT const sampler_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_SAMPLER,
            .objectHandle = std::bit_cast<uint64_t>(ret.vk_sampler),
            .pObjectName = c_str_arr.data(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &sampler_name_info);
    }

    write_descriptor_set_sampler(self->vk_device, self->gpu_sro_table.vk_descriptor_set, ret.vk_sampler, id.index);
    *out_id = std::bit_cast<daxa_SamplerId>(id);
    return DAXA_RESULT_SUCCESS;
}

#define _DAXA_DECL_COMMON_GP_RES_FUNCTIONS(name, Name, NAME, SLOT_NAME)                                          \
    auto daxa_dvc_destroy_##name(daxa_Device self, daxa_##Name##Id id) -> daxa_Result                            \
    {                                                                                                            \
        _DAXA_TEST_PRINT("STRONG daxa_dvc_destroy_%s\n", #name);                                                 \
        auto success = self->gpu_sro_table.SLOT_NAME.try_zombify(std::bit_cast<GPUResourceId>(id));              \
        if (success)                                                                                             \
        {                                                                                                        \
            self->zombify_##name(std::bit_cast<Name##Id>(id));                                                   \
            return DAXA_RESULT_SUCCESS;                                                                          \
        }                                                                                                        \
        return DAXA_RESULT_INVALID_##NAME##_ID;                                                                  \
    }                                                                                                            \
    auto daxa_dvc_info_##name(daxa_Device self, daxa_##Name##Id id, daxa_##Name##Info * out_info) -> daxa_Result \
    {                                                                                                            \
        /*NOTE: THIS CAN RACE. BUT IT IS OK AS ITS A POD AND WE CHECK IF ITS VALID AFTER THE COPY!*/             \
        auto info_copy = self->slot(id).info;                                                                    \
        if (daxa_dvc_is_##name##_valid(self, id))                                                                \
        {                                                                                                        \
            *out_info = info_copy;                                                                               \
            return DAXA_RESULT_SUCCESS;                                                                          \
        }                                                                                                        \
        return DAXA_RESULT_INVALID_##NAME##_ID;                                                                  \
    }                                                                                                            \
    auto daxa_dvc_is_##name##_valid(daxa_Device self, daxa_##Name##Id id) -> daxa_Bool8                          \
    {                                                                                                            \
        return std::bit_cast<daxa_Bool8>(self->gpu_sro_table.SLOT_NAME.is_id_valid(                              \
            std::bit_cast<daxa::GPUResourceId>(id)));                                                            \
    }                                                                                                            \
    auto daxa_dvc_get_vk_##name(daxa_Device self, daxa_##Name##Id id) -> Vk##Name                                \
    {                                                                                                            \
        return self->slot(id).vk_##name;                                                                         \
    }

_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(buffer, Buffer, BUFFER, buffer_slots)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(image, Image, IMAGE, image_slots)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(image_view, ImageView, IMAGE_VIEW, image_slots)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(sampler, Sampler, SAMPLER, sampler_slots)

auto daxa_dvc_buffer_device_address(daxa_Device self, daxa_BufferId id, daxa_BufferDeviceAddress * out_bda) -> daxa_Result
{
    if (self->slot(std::bit_cast<BufferId>(id)).device_address == 0)
    {
        return DAXA_RESULT_BUFFER_NOT_DEVICE_VISIBLE;
    }
    *out_bda = static_cast<daxa_BufferDeviceAddress>(self->slot(std::bit_cast<BufferId>(id)).device_address);
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_buffer_host_address(daxa_Device self, daxa_BufferId id, void ** out_ptr) -> daxa_Result
{
    if (self->slot(std::bit_cast<BufferId>(id)).host_address == 0)
    {
        return DAXA_RESULT_BUFFER_NOT_HOST_VISIBLE;
    }
    *out_ptr = self->slot(std::bit_cast<BufferId>(id)).host_address;
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_info(daxa_Device self) -> daxa_DeviceInfo const *
{
    return r_cast<daxa_DeviceInfo const *>(&self->info);
}

auto daxa_dvc_get_vk_device(daxa_Device self) -> VkDevice
{
    return self->vk_device;
}

auto daxa_dvc_wait_idle(daxa_Device self) -> daxa_Result
{
    auto vk_result = vkQueueWaitIdle(self->main_queue_vk_queue);
    if (vk_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(vk_result);
    }
    vk_result = vkDeviceWaitIdle(self->vk_device);
    return std::bit_cast<daxa_Result>(vk_result);
}

auto daxa_dvc_submit(daxa_Device self, daxa_CommandSubmitInfo const * info) -> daxa_Result
{
    _DAXA_TEST_PRINT("\n");
    std::shared_lock lifetime_lock{self->gpu_sro_table.lifetime_lock};

    for (daxa_ExecutableCommandList commands : std::span{info->command_lists, info->command_list_count})
    {
        for (BufferId id : commands->data.used_buffers)
        {
            if (!daxa_dvc_is_buffer_valid(self, id))
            {
                return DAXA_RESULT_COMMAND_REFERENCES_INVALID_BUFFER_ID;
            }
        }
        for (ImageId id : commands->data.used_images)
        {
            if (!daxa_dvc_is_image_valid(self, id))
            {
                return DAXA_RESULT_COMMAND_REFERENCES_INVALID_IMAGE_ID;
            }
        }
        for (ImageViewId id : commands->data.used_image_views)
        {
            if (!daxa_dvc_is_image_view_valid(self, id))
            {
                return DAXA_RESULT_COMMAND_REFERENCES_INVALID_IMAGE_VIEW_ID;
            }
        }
        for (SamplerId id : commands->data.used_samplers)
        {
            if (!daxa_dvc_is_sampler_valid(self, id))
            {
                return DAXA_RESULT_COMMAND_REFERENCES_INVALID_SAMPLER_ID;
            }
        }
    }

    u64 const current_main_queue_cpu_timeline_value = self->main_queue_cpu_timeline.fetch_add(1) + 1;

    for (auto const & commands : std::span{info->command_lists, info->command_list_count})
    {
        for (auto [id, index] : commands->data.deferred_destructions)
        {
            // TODO(lifetime): check these and report errors if these were destroyed too early.
            [[maybe_unused]] daxa_Result _ignore = {};
            switch (index)
            {
            case DEFERRED_DESTRUCTION_BUFFER_INDEX: _ignore = daxa_dvc_destroy_buffer(self, std::bit_cast<daxa_BufferId>(id)); break;
            case DEFERRED_DESTRUCTION_IMAGE_INDEX: _ignore = daxa_dvc_destroy_image(self, std::bit_cast<daxa_ImageId>(id)); break;
            case DEFERRED_DESTRUCTION_IMAGE_VIEW_INDEX: _ignore = daxa_dvc_destroy_image_view(self, std::bit_cast<daxa_ImageViewId>(id)); break;
            case DEFERRED_DESTRUCTION_SAMPLER_INDEX:
                _ignore = daxa_dvc_destroy_sampler(self, std::bit_cast<daxa_SamplerId>(id));
                break;
                // TODO(capi): DO NOT THROW FROM A C FUNCTION
                // default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
            }
        }
    }

    std::vector<VkCommandBuffer> submit_vk_command_buffers = {};
    for (auto const & commands : std::span{info->command_lists, info->command_list_count})
    {
        submit_vk_command_buffers.push_back(commands->data.vk_cmd_buffer);
    }

    std::vector<VkSemaphore> submit_semaphore_signals = {}; // All timeline semaphores come first, then binary semaphores follow.
    std::vector<u64> submit_semaphore_signal_values = {};   // Used for timeline semaphores. Ignored (push dummy value) for binary semaphores.

    // Add main queue timeline signaling as first timeline semaphore signaling:
    submit_semaphore_signals.push_back(self->vk_main_queue_gpu_timeline_semaphore);
    submit_semaphore_signal_values.push_back(current_main_queue_cpu_timeline_value);

    for (auto const & pair : std::span{info->signal_timeline_semaphores, info->signal_timeline_semaphore_count})
    {
        submit_semaphore_signals.push_back(pair.semaphore->vk_semaphore);
        submit_semaphore_signal_values.push_back(pair.value);
    }

    for (auto const & binary_semaphore : std::span{info->signal_binary_semaphores, info->signal_binary_semaphore_count})
    {
        submit_semaphore_signals.push_back(binary_semaphore->vk_semaphore);
        submit_semaphore_signal_values.push_back(0); // The vulkan spec requires to have dummy values for binary semaphores.
    }

    // used to synchronize with previous submits:
    std::vector<VkSemaphore> submit_semaphore_waits = {}; // All timeline semaphores come first, then binary semaphores follow.
    std::vector<VkPipelineStageFlags> submit_semaphore_wait_stage_masks = {};
    std::vector<u64> submit_semaphore_wait_values = {}; // Used for timeline semaphores. Ignored (push dummy value) for binary semaphores.

    for (auto const & pair : std::span{info->wait_timeline_semaphores, info->wait_timeline_semaphore_count})
    {
        submit_semaphore_waits.push_back(pair.semaphore->vk_semaphore);
        submit_semaphore_wait_stage_masks.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        submit_semaphore_wait_values.push_back(pair.value);
    }

    for (auto const & binary_semaphore : std::span{info->wait_binary_semaphores, info->wait_binary_semaphore_count})
    {
        submit_semaphore_waits.push_back(binary_semaphore->vk_semaphore);
        submit_semaphore_wait_stage_masks.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        submit_semaphore_wait_values.push_back(0);
    }

    VkTimelineSemaphoreSubmitInfo timeline_info{
        .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreValueCount = static_cast<u32>(submit_semaphore_wait_values.size()),
        .pWaitSemaphoreValues = submit_semaphore_wait_values.data(),
        .signalSemaphoreValueCount = static_cast<u32>(submit_semaphore_signal_values.size()),
        .pSignalSemaphoreValues = submit_semaphore_signal_values.data(),
    };

    VkSubmitInfo const vk_submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = r_cast<void *>(&timeline_info),
        .waitSemaphoreCount = static_cast<u32>(submit_semaphore_waits.size()),
        .pWaitSemaphores = submit_semaphore_waits.data(),
        .pWaitDstStageMask = submit_semaphore_wait_stage_masks.data(),
        .commandBufferCount = static_cast<u32>(submit_vk_command_buffers.size()),
        .pCommandBuffers = submit_vk_command_buffers.data(),
        .signalSemaphoreCount = static_cast<u32>(submit_semaphore_signals.size()),
        .pSignalSemaphores = submit_semaphore_signals.data(),
    };
    auto result = vkQueueSubmit(self->main_queue_vk_queue, 1, &vk_submit_info, VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(result);
    }

    std::unique_lock const lock{self->main_queue_zombies_mtx};

    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_present(daxa_Device self, daxa_PresentInfo const * info) -> daxa_Result
{
    // used to synchronize with previous submits:
    std::vector<VkSemaphore> submit_semaphore_waits = {};

    for (auto const & binary_semaphore : std::span{info->wait_binary_semaphores, info->wait_binary_semaphore_count})
    {
        submit_semaphore_waits.push_back(binary_semaphore->vk_semaphore);
    }

    VkPresentInfoKHR const present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = static_cast<u32>(submit_semaphore_waits.size()),
        .pWaitSemaphores = submit_semaphore_waits.data(),
        .swapchainCount = static_cast<u32>(1),
        .pSwapchains = &info->swapchain->vk_swapchain,
        .pImageIndices = &info->swapchain->current_image_index,
        .pResults = {},
    };

    auto result = vkQueuePresentKHR(self->main_queue_vk_queue, &present_info);

    return std::bit_cast<daxa_Result>(result);
}

auto daxa_dvc_collect_garbage(daxa_Device self) -> daxa_Result
{
    std::unique_lock lifetime_lock{self->gpu_sro_table.lifetime_lock};
    std::unique_lock lock{self->main_queue_zombies_mtx};

    u64 gpu_timeline_value = std::numeric_limits<u64>::max();
    {
        auto result = vkGetSemaphoreCounterValue(
            self->vk_device,
            self->vk_main_queue_gpu_timeline_semaphore,
            &gpu_timeline_value);
        if (result != VK_SUCCESS)
        {
            return std::bit_cast<daxa_Result>(result);
        }
    }

    auto check_and_cleanup_gpu_resources = [&](auto & zombies, auto const & cleanup_fn)
    {
        while (!zombies.empty())
        {
            auto & [timeline_value, object] = zombies.back();

            if (timeline_value > gpu_timeline_value)
            {
                break;
            }

            cleanup_fn(object);
            zombies.pop_back();
        }
    };
    check_and_cleanup_gpu_resources(
        self->main_queue_buffer_zombies,
        [&](auto id)
        {
            self->cleanup_buffer(id);
        });
    check_and_cleanup_gpu_resources(
        self->main_queue_image_view_zombies,
        [&](auto id)
        {
            self->cleanup_image_view(id);
        });
    check_and_cleanup_gpu_resources(
        self->main_queue_image_zombies,
        [&](auto id)
        {
            self->cleanup_image(id);
        });
    check_and_cleanup_gpu_resources(
        self->main_queue_sampler_zombies,
        [&](auto id)
        {
            self->cleanup_sampler(id);
        });
    check_and_cleanup_gpu_resources(
        self->main_queue_pipeline_zombies,
        [&](auto & pipeline_zombie)
        {
            vkDestroyPipeline(self->vk_device, pipeline_zombie.vk_pipeline, nullptr);
        });
    check_and_cleanup_gpu_resources(
        self->main_queue_semaphore_zombies,
        [&](auto & semaphore_zombie)
        {
            vkDestroySemaphore(self->vk_device, semaphore_zombie.vk_semaphore, nullptr);
        });
    check_and_cleanup_gpu_resources(
        self->main_queue_split_barrier_zombies,
        [&](auto & split_barrier_zombie)
        {
            vkDestroyEvent(self->vk_device, split_barrier_zombie.vk_event, nullptr);
        });
    check_and_cleanup_gpu_resources(
        self->main_queue_timeline_query_pool_zombies,
        [&](auto & timeline_query_pool_zombie)
        {
            vkDestroyQueryPool(self->vk_device, timeline_query_pool_zombie.vk_timeline_query_pool, nullptr);
        });
    check_and_cleanup_gpu_resources(
        self->main_queue_memory_block_zombies,
        [&](auto & memory_block_zombie)
        {
            vmaFreeMemory(self->vma_allocator, memory_block_zombie.allocation);
        });

    {
        std::unique_lock const l_lock{self->main_queue_command_pool_buffer_recycle_mtx};
        while (!self->main_queue_command_list_zombies.empty())
        {
            auto & [timeline_value, object] = self->main_queue_command_list_zombies.back();

            if (timeline_value > gpu_timeline_value)
            {
                break;
            }

            // This apparently "leaks" in our current impl. Instead, we recreate below.
            // TODO: reuse command buffers instead?
            // auto vk_result = vkResetCommandPool(self->vk_device, object.vk_cmd_pool, {});

            vkFreeCommandBuffers(self->vk_device, object.vk_cmd_pool, object.allocated_command_buffers.size(), object.allocated_command_buffers.data());
            auto vk_result = vkResetCommandPool(self->vk_device, object.vk_cmd_pool, {});
            if (vk_result != VK_SUCCESS)
            {
                return std::bit_cast<daxa_Result>(vk_result);
            }

            self->buffer_pool_pool.put_back(object.vk_cmd_pool);
            self->main_queue_command_list_zombies.pop_back();
        }
    }
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_properties(daxa_Device device) -> daxa_DeviceProperties const *
{
    return r_cast<daxa_DeviceProperties const *>(&device->vk_physical_device_properties2.properties);
}

auto daxa_dvc_inc_refcnt(daxa_Device self) -> u64
{
    _DAXA_TEST_PRINT("device inc refcnt from %u to %u\n", self->strong_count, self->strong_count + 1);
    return self->inc_refcnt();
}

auto daxa_dvc_dec_refcnt(daxa_Device self) -> u64
{
    _DAXA_TEST_PRINT("device dec refcnt from %u to %u\n", self->strong_count, self->strong_count - 1);
    return self->dec_refcnt(
        &daxa_ImplDevice::zero_ref_callback,
        self->instance);
}

// --- End API Functions ---

// --- Begin Internal Functions ---

auto daxa_ImplDevice::create(daxa_Instance instance, daxa_DeviceInfo const & info, VkPhysicalDevice physical_device, daxa_Device out_device) -> daxa_Result
{
    using namespace daxa;
    VkResult result = {};

    // Set properties and feature variables:
    auto self = out_device;
    self->vk_physical_device = physical_device;
    self->instance = instance;
    self->info = *r_cast<DeviceInfo const *>(&info);
    self->vk_physical_device_properties2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = {},
        .properties = {},
    };
    vkGetPhysicalDeviceProperties2(self->vk_physical_device, &self->vk_physical_device_properties2);

    // SELECT QUEUE

    u32 queue_family_props_count = 0;
    std::vector<VkQueueFamilyProperties> queue_props;
    vkGetPhysicalDeviceQueueFamilyProperties(self->vk_physical_device, &queue_family_props_count, nullptr);
    queue_props.resize(queue_family_props_count);
    vkGetPhysicalDeviceQueueFamilyProperties(self->vk_physical_device, &queue_family_props_count, queue_props.data());
    std::vector<VkBool32> supports_present;
    supports_present.resize(queue_family_props_count);

    for (u32 i = 0; i < queue_family_props_count; i++)
    {
        if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && (queue_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0 && (queue_props[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
        {
            self->main_queue_family_index = i;
            break;
        }
    }
    DAXA_DBG_ASSERT_TRUE_M(self->main_queue_family_index != std::numeric_limits<u32>::max(), "found no suitable queue family");

    std::array<f32, 1> queue_priorities = {0.0};
    VkDeviceQueueCreateInfo const queue_ci{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = self->main_queue_family_index,
        .queueCount = static_cast<u32>(queue_priorities.size()),
        .pQueuePriorities = queue_priorities.data(),
    };

    PhysicalDeviceFeatureTable feature_table = {};
    feature_table.initialize(info);
    PhysicalDeviceExtensionList extension_list = {};
    extension_list.initialize(info);

    VkPhysicalDeviceFeatures2 physical_device_features_2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = feature_table.chain,
        .features = feature_table.features,
    };

    VkDeviceCreateInfo const device_ci = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = r_cast<void const *>(&physical_device_features_2),
        .flags = {},
        .queueCreateInfoCount = static_cast<u32>(1),
        .pQueueCreateInfos = &queue_ci,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<u32>(extension_list.size),
        .ppEnabledExtensionNames = extension_list.data,
        .pEnabledFeatures = nullptr,
    };
    result = vkCreateDevice(self->vk_physical_device, &device_ci, nullptr, &self->vk_device);
    if (result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(result);
    }

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE)
    {
        self->vkSetDebugUtilsObjectNameEXT = r_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(self->vk_device, "vkSetDebugUtilsObjectNameEXT"));
        self->vkCmdBeginDebugUtilsLabelEXT = r_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdBeginDebugUtilsLabelEXT"));
        self->vkCmdEndDebugUtilsLabelEXT = r_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdEndDebugUtilsLabelEXT"));
    }
    self->vkCmdPushDescriptorSetKHR = r_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetDeviceProcAddr(self->vk_device, "vkCmdPushDescriptorSetKHR"));

    if ((self->info.flags & DeviceFlagBits::MESH_SHADER) != DeviceFlagBits::NONE)
    {
        self->vkCmdDrawMeshTasksEXT = r_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdDrawMeshTasksEXT"));
        self->vkCmdDrawMeshTasksIndirectEXT = r_cast<PFN_vkCmdDrawMeshTasksIndirectEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdDrawMeshTasksIndirectEXT"));
        self->vkCmdDrawMeshTasksIndirectCountEXT = r_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdDrawMeshTasksIndirectCountEXT"));
        auto * out_struct = r_cast<VkBaseOutStructure *>(&self->vk_physical_device_properties2);
        while (out_struct != nullptr)
        {
            if (out_struct->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT)
            {
                self->mesh_shader_properties = *r_cast<VkPhysicalDeviceMeshShaderPropertiesEXT *>(out_struct);
            }
            out_struct = out_struct->pNext;
        }
    }

    if ((self->info.flags & DeviceFlagBits::RAY_TRACING) != DeviceFlagBits::NONE)
    {
        self->vkGetAccelerationStructureBuildSizesKHR = r_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(self->vk_device, "vkGetAccelerationStructureBuildSizesKHR"));
        self->vkCreateAccelerationStructureKHR = r_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(self->vk_device, "vkCreateAccelerationStructureKHR"));
        self->vkCmdWriteAccelerationStructuresPropertiesKHR = r_cast<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>(vkGetDeviceProcAddr(self->vk_device, "vkCmdWriteAccelerationStructuresPropertiesKHR"));
        self->vkCmdBuildAccelerationStructuresKHR = r_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(self->vk_device, "vkCmdBuildAccelerationStructuresKHR"));
    }

    vkGetDeviceQueue(self->vk_device, self->main_queue_family_index, 0, &self->main_queue_vk_queue);

    VkCommandPool init_cmd_pool = {};
    VkCommandBuffer init_cmd_buffer = {};
    VkCommandPoolCreateInfo const vk_command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = self->main_queue_family_index,
    };

    result = vkCreateCommandPool(self->vk_device, &vk_command_pool_create_info, nullptr, &init_cmd_pool);
    if (result != VK_SUCCESS)
    {
        vkDestroyDevice(self->vk_device, nullptr);
        return std::bit_cast<daxa_Result>(result);
    }

    VkCommandBufferAllocateInfo const vk_command_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = init_cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    result = vkAllocateCommandBuffers(self->vk_device, &vk_command_buffer_allocate_info, &init_cmd_buffer);
    if (result != VK_SUCCESS)
    {
        vkDestroyCommandPool(self->vk_device, init_cmd_pool, nullptr);
        vkDestroyDevice(self->vk_device, nullptr);
        return std::bit_cast<daxa_Result>(result);
    }

    VkCommandBufferBeginInfo const vk_command_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = {},
    };
    result = vkBeginCommandBuffer(init_cmd_buffer, &vk_command_buffer_begin_info);
    if (result != VK_SUCCESS)
    {
        vkDestroyCommandPool(self->vk_device, init_cmd_pool, nullptr);
        vkDestroyDevice(self->vk_device, nullptr);
        return std::bit_cast<daxa_Result>(result);
    }

    VkSemaphoreTypeCreateInfo timeline_ci{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .pNext = nullptr,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = 0,
    };

    VkSemaphoreCreateInfo const vk_semaphore_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = r_cast<void *>(&timeline_ci),
        .flags = {},
    };

    result = vkCreateSemaphore(self->vk_device, &vk_semaphore_create_info, nullptr, &self->vk_main_queue_gpu_timeline_semaphore);
    if (result != VK_SUCCESS)
    {
        vkDestroyCommandPool(self->vk_device, init_cmd_pool, nullptr);
        vkDestroyDevice(self->vk_device, nullptr);
        return std::bit_cast<daxa_Result>(result);
    }

    if (self->info.max_allowed_buffers > self->vk_physical_device_properties2.properties.limits.maxDescriptorSetStorageBuffers)
    {
        return DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_BUFFER_COUNT;
    }
    auto const max_device_supported_images_in_set = std::min(self->vk_physical_device_properties2.properties.limits.maxDescriptorSetSampledImages, self->vk_physical_device_properties2.properties.limits.maxDescriptorSetStorageImages);
    if (self->info.max_allowed_buffers > max_device_supported_images_in_set)
    {
        return DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_IMAGE_COUNT;
    }
    if (self->info.max_allowed_samplers > self->vk_physical_device_properties2.properties.limits.maxDescriptorSetSamplers)
    {
        return DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_SAMPLER_COUNT;
    }

    VmaVulkanFunctions const vma_vulkan_functions
    {
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkGetPhysicalDeviceProperties = {},
        .vkGetPhysicalDeviceMemoryProperties = {},
        .vkAllocateMemory = {},
        .vkFreeMemory = {},
        .vkMapMemory = {},
        .vkUnmapMemory = {},
        .vkFlushMappedMemoryRanges = {},
        .vkInvalidateMappedMemoryRanges = {},
        .vkBindBufferMemory = {},
        .vkBindImageMemory = {},
        .vkGetBufferMemoryRequirements = {},
        .vkGetImageMemoryRequirements = {},
        .vkCreateBuffer = {},
        .vkDestroyBuffer = {},
        .vkCreateImage = {},
        .vkDestroyImage = {},
        .vkCmdCopyBuffer = {},
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
        .vkGetBufferMemoryRequirements2KHR = {},
        .vkGetImageMemoryRequirements2KHR = {},
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
        .vkBindBufferMemory2KHR = {},
        .vkBindImageMemory2KHR = {},
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
        .vkGetPhysicalDeviceMemoryProperties2KHR = {},
#endif
#if VMA_VULKAN_VERSION >= 1003000
        .vkGetDeviceBufferMemoryRequirements = {},
        .vkGetDeviceImageMemoryRequirements = {},
#endif
    };

    VmaAllocatorCreateInfo const vma_allocator_create_info{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = self->vk_physical_device,
        .device = self->vk_device,
        .preferredLargeHeapBlockSize = 0, // Sets it to lib internal default (256MiB).
        .pAllocationCallbacks = nullptr,
        .pDeviceMemoryCallbacks = nullptr,
        .pHeapSizeLimit = nullptr,
        .pVulkanFunctions = &vma_vulkan_functions,
        .instance = self->instance->vk_instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
        .pTypeExternalMemoryHandleTypes = {},
    };

    result = vmaCreateAllocator(&vma_allocator_create_info, &self->vma_allocator);
    if (result != VK_SUCCESS)
    {
        vkDestroySemaphore(self->vk_device, self->vk_main_queue_gpu_timeline_semaphore, nullptr);
        vkDestroyCommandPool(self->vk_device, init_cmd_pool, nullptr);
        vkDestroyDevice(self->vk_device, nullptr);
        return std::bit_cast<daxa_Result>(result);
    }

    {
        auto buffer_data = std::array<u8, 4>{0xff, 0x00, 0xff, 0xff};

        VkBufferCreateInfo const vk_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .size = sizeof(u8) * 4,
            .usage = BUFFER_USE_FLAGS,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &self->main_queue_family_index,
        };

        VmaAllocationInfo vma_allocation_info = {};

        auto vma_allocation_flags = static_cast<VmaAllocationCreateFlags>(
            VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);

        VmaAllocationCreateInfo const vma_allocation_create_info{
            .flags = vma_allocation_flags,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        result = vmaCreateBuffer(self->vma_allocator, &vk_buffer_create_info, &vma_allocation_create_info, &self->vk_null_buffer, &self->vk_null_buffer_vma_allocation, &vma_allocation_info);
        if (result != VK_SUCCESS)
        {
            vmaDestroyAllocator(self->vma_allocator);
            vkDestroySemaphore(self->vk_device, self->vk_main_queue_gpu_timeline_semaphore, nullptr);
            vkDestroyCommandPool(self->vk_device, init_cmd_pool, nullptr);
            vkDestroyDevice(self->vk_device, nullptr);
            return DAXA_RESULT_FAILED_TO_CREATE_NULL_BUFFER;
        }

        *static_cast<decltype(buffer_data) *>(vma_allocation_info.pMappedData) = buffer_data;
    }

    {
        auto image_info = ImageInfo{
            .dimensions = 2,
            .format = Format::R8G8B8A8_UNORM,
            .size = {1, 1, 1},
            .mip_level_count = 1,
            .array_layer_count = 1,
            .sample_count = 1,
            .usage = ImageUsageFlagBits::SHADER_SAMPLED | ImageUsageFlagBits::SHADER_STORAGE | ImageUsageFlagBits::TRANSFER_DST,
            .allocate_info = MemoryFlagBits::DEDICATED_MEMORY,
        };
        VkImageCreateInfo const vk_image_create_info = initialize_image_create_info_from_image_info(
            *r_cast<daxa_ImageInfo const *>(&image_info), &self->main_queue_family_index);

        VmaAllocationCreateInfo const vma_allocation_create_info{
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        result = vmaCreateImage(self->vma_allocator, &vk_image_create_info, &vma_allocation_create_info, &self->vk_null_image, &self->vk_null_image_vma_allocation, nullptr);
        if (result != VK_SUCCESS)
        {
            vmaDestroyBuffer(self->vma_allocator, self->vk_null_buffer, self->vk_null_buffer_vma_allocation);
            vmaDestroyAllocator(self->vma_allocator);
            vkDestroySemaphore(self->vk_device, self->vk_main_queue_gpu_timeline_semaphore, nullptr);
            vkDestroyCommandPool(self->vk_device, init_cmd_pool, nullptr);
            vkDestroyDevice(self->vk_device, nullptr);
            return DAXA_RESULT_FAILED_TO_CREATE_NULL_IMAGE;
        }

        VkImageViewCreateInfo const vk_image_view_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .image = self->vk_null_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = *r_cast<VkFormat const *>(&image_info.format),
            .components = VkComponentMapping{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = image_info.mip_level_count,
                .baseArrayLayer = 0,
                .layerCount = image_info.array_layer_count,
            },
        };

        result = vkCreateImageView(self->vk_device, &vk_image_view_create_info, nullptr, &self->vk_null_image_view);
        if (result != VK_SUCCESS)
        {
            vmaDestroyImage(self->vma_allocator, self->vk_null_image, self->vk_null_image_vma_allocation);
            vmaDestroyBuffer(self->vma_allocator, self->vk_null_buffer, self->vk_null_buffer_vma_allocation);
            vmaDestroyAllocator(self->vma_allocator);
            vkDestroySemaphore(self->vk_device, self->vk_main_queue_gpu_timeline_semaphore, nullptr);
            vkDestroyCommandPool(self->vk_device, init_cmd_pool, nullptr);
            vkDestroyDevice(self->vk_device, nullptr);
            return DAXA_RESULT_FAILED_TO_CREATE_NULL_IMAGE_VIEW;
        }

        VkImageMemoryBarrier vk_image_mem_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = {},
            .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout = {},
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = self->vk_null_image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        vkCmdPipelineBarrier(init_cmd_buffer, VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, {}, {}, {}, 1, &vk_image_mem_barrier);
        VkBufferImageCopy const vk_buffer_image_copy{
            .bufferOffset = 0u,
            .bufferRowLength = 0u,
            .bufferImageHeight = 0u,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .imageOffset = VkOffset3D{0, 0, 0},
            .imageExtent = VkExtent3D{1, 1, 1},
        };
        vkCmdCopyBufferToImage(init_cmd_buffer, self->vk_null_buffer, self->vk_null_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &vk_buffer_image_copy);
        vk_image_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        vk_image_mem_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        vk_image_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        vk_image_mem_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL,
        vk_image_mem_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        vk_image_mem_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        vkCmdPipelineBarrier(init_cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, {}, {}, {}, 1, &vk_image_mem_barrier);
    }

    {
        VkSamplerCreateInfo const vk_sampler_create_info{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .magFilter = VkFilter::VK_FILTER_LINEAR,
            .minFilter = VkFilter::VK_FILTER_LINEAR,
            .mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 0,
            .compareEnable = VK_FALSE,
            .compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS,
            .minLod = 0,
            .maxLod = 0,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };
        result = vkCreateSampler(self->vk_device, &vk_sampler_create_info, nullptr, &self->vk_null_sampler);
        if (result != VK_SUCCESS)
        {
            vkDestroyImageView(self->vk_device, self->vk_null_image_view, nullptr);
            vmaDestroyImage(self->vma_allocator, self->vk_null_image, self->vk_null_image_vma_allocation);
            vmaDestroyBuffer(self->vma_allocator, self->vk_null_buffer, self->vk_null_buffer_vma_allocation);
            vmaDestroyAllocator(self->vma_allocator);
            vkDestroySemaphore(self->vk_device, self->vk_main_queue_gpu_timeline_semaphore, nullptr);
            vkDestroyCommandPool(self->vk_device, init_cmd_pool, nullptr);
            vkDestroyDevice(self->vk_device, nullptr);
            return DAXA_RESULT_FAILED_TO_CREATE_NULL_SAMPLER;
        }
    }

    {
        VkBufferUsageFlags const usage_flags =
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        VkBufferCreateInfo const vk_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .size = self->info.max_allowed_buffers * sizeof(u64),
            .usage = usage_flags,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &self->main_queue_family_index,
        };

        VmaAllocationCreateInfo const vma_allocation_create_info{
            .flags = static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT),
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        result = vmaCreateBuffer(self->vma_allocator, &vk_buffer_create_info, &vma_allocation_create_info, &self->buffer_device_address_buffer, &self->buffer_device_address_buffer_allocation, nullptr);
        vmaMapMemory(self->vma_allocator, self->buffer_device_address_buffer_allocation, r_cast<void **>(&self->buffer_device_address_buffer_host_ptr));
        if (result != VK_SUCCESS)
        {
            vkDestroySampler(self->vk_device, self->vk_null_sampler, nullptr);
            vkDestroyImageView(self->vk_device, self->vk_null_image_view, nullptr);
            vmaDestroyImage(self->vma_allocator, self->vk_null_image, self->vk_null_image_vma_allocation);
            vmaDestroyBuffer(self->vma_allocator, self->vk_null_buffer, self->vk_null_buffer_vma_allocation);
            vmaDestroyAllocator(self->vma_allocator);
            vkDestroySemaphore(self->vk_device, self->vk_main_queue_gpu_timeline_semaphore, nullptr);
            vkDestroyCommandPool(self->vk_device, init_cmd_pool, nullptr);
            vkDestroyDevice(self->vk_device, nullptr);
            return DAXA_RESULT_FAILED_TO_CREATE_BDA_BUFFER;
        }
    }

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && !self->info.name.view().empty())
    {
        auto const device_name = self->info.name.c_str();
        VkDebugUtilsObjectNameInfoEXT const device_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_DEVICE,
            .objectHandle = std::bit_cast<uint64_t>(self->vk_device),
            .pObjectName = device_name.data(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_name_info);

        auto const queue_name = self->info.name.c_str();
        VkDebugUtilsObjectNameInfoEXT const device_main_queue_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_QUEUE,
            .objectHandle = std::bit_cast<uint64_t>(self->main_queue_vk_queue),
            .pObjectName = queue_name.data(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_main_queue_name_info);

        auto const semaphore_name = self->info.name.c_str();
        VkDebugUtilsObjectNameInfoEXT const device_main_queue_timeline_semaphore_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_SEMAPHORE,
            .objectHandle = std::bit_cast<uint64_t>(self->vk_main_queue_gpu_timeline_semaphore),
            .pObjectName = semaphore_name.data(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_main_queue_timeline_semaphore_name_info);

        auto const buffer_name = self->info.name.c_str();
        VkDebugUtilsObjectNameInfoEXT const device_main_queue_timeline_buffer_device_address_buffer_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = std::bit_cast<uint64_t>(self->buffer_device_address_buffer),
            .pObjectName = buffer_name.data(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_main_queue_timeline_buffer_device_address_buffer_name_info);
    }

    self->gpu_sro_table.initialize(
        self->info.max_allowed_buffers,
        self->info.max_allowed_images,
        self->info.max_allowed_samplers,
        (info.flags & DAXA_DEVICE_FLAG_RAY_TRACING) ? self->info.max_allowed_acceleration_structures : (~0),
        self->vk_device,
        self->buffer_device_address_buffer,
        self->vkSetDebugUtilsObjectNameEXT);

    auto end_err_cleanup = [&]()
    {
        vmaDestroyBuffer(self->vma_allocator, self->buffer_device_address_buffer, self->buffer_device_address_buffer_allocation);
        vkDestroySampler(self->vk_device, self->vk_null_sampler, nullptr);
        vkDestroyImageView(self->vk_device, self->vk_null_image_view, nullptr);
        vmaDestroyImage(self->vma_allocator, self->vk_null_image, self->vk_null_image_vma_allocation);
        vmaDestroyBuffer(self->vma_allocator, self->vk_null_buffer, self->vk_null_buffer_vma_allocation);
        vmaDestroyAllocator(self->vma_allocator);
        vkDestroySemaphore(self->vk_device, self->vk_main_queue_gpu_timeline_semaphore, nullptr);
        vkDestroyCommandPool(self->vk_device, init_cmd_pool, nullptr);
        vkDestroyDevice(self->vk_device, nullptr);
    };

    result = vkEndCommandBuffer(init_cmd_buffer);
    if (result != VK_SUCCESS)
    {
        end_err_cleanup();
        return DAXA_RESULT_FAILED_TO_SUBMIT_DEVICE_INIT_COMMANDS;
    }
    // Submit initial commands to set up the daxa device.
    constexpr VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo init_submit{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = {},
        .waitSemaphoreCount = {},
        .pWaitSemaphores = {},
        .pWaitDstStageMask = &wait_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &init_cmd_buffer,
        .signalSemaphoreCount = {},
        .pSignalSemaphores = {},
    };
    result = vkQueueSubmit(self->main_queue_vk_queue, 1, &init_submit, {});
    if (result != VK_SUCCESS)
    {
        end_err_cleanup();
        return DAXA_RESULT_FAILED_TO_SUBMIT_DEVICE_INIT_COMMANDS;
    }
    // Wait for commands in from the init cmd list to complete.
    result = vkDeviceWaitIdle(self->vk_device);
    if (result != VK_SUCCESS)
    {
        end_err_cleanup();
        return DAXA_RESULT_FAILED_TO_SUBMIT_DEVICE_INIT_COMMANDS;
    }
    vkDestroyCommandPool(self->vk_device, init_cmd_pool, {});

    return DAXA_RESULT_SUCCESS;
}

auto daxa_ImplDevice::validate_image_slice(daxa_ImageMipArraySlice const & slice, daxa_ImageId id) -> daxa_ImageMipArraySlice
{
    if (slice.level_count == std::numeric_limits<u32>::max() || slice.level_count == 0)
    {
        auto & image_info = this->slot(id).info;
        return daxa_ImageMipArraySlice{
            .base_mip_level = 0,
            .level_count = image_info.mip_level_count,
            .base_array_layer = 0,
            .layer_count = image_info.array_layer_count,
        };
    }
    else
    {
        return slice;
    }
}

auto daxa_ImplDevice::validate_image_slice(daxa_ImageMipArraySlice const & slice, daxa_ImageViewId id) -> daxa_ImageMipArraySlice
{
    if (slice.level_count == std::numeric_limits<u32>::max() || slice.level_count == 0)
    {
        return this->slot(id).info.slice;
    }
    else
    {
        return slice;
    }
}

auto daxa_ImplDevice::new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index, ImageUsageFlags usage, ImageInfo const & image_info) -> std::pair<daxa_Result, ImageId>
{
    auto slot_opt = this->gpu_sro_table.image_slots.try_create_slot();
    DAXA_DBG_ASSERT_TRUE_M(slot_opt.has_value(), "CRITICAL INTERNAL ERROR, EXCEEDED MAX IMAGES IN SWAPCHAIN CREATION");
    auto [id, ret] = slot_opt.value();

    ret.vk_image = swapchain_image;
    ret.view_slot.info = std::bit_cast<daxa_ImageViewInfo>(ImageViewInfo{
        .type = static_cast<ImageViewType>(image_info.dimensions - 1),
        .format = image_info.format,
        .image = {id},
        .slice = ImageMipArraySlice{
            .base_mip_level = 0,
            .level_count = image_info.mip_level_count,
            .base_array_layer = 0,
            .layer_count = image_info.array_layer_count,
        },
        .name = image_info.name,
    });
    ret.aspect_flags = infer_aspect_from_format(image_info.format);

    VkImageViewCreateInfo const view_ci{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = swapchain_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    ret.swapchain_image_index = static_cast<i32>(index);

    ret.info = *r_cast<daxa_ImageInfo const *>(&image_info);
    auto result = vkCreateImageView(vk_device, &view_ci, nullptr, &ret.view_slot.vk_image_view);
    if (result != VK_SUCCESS)
    {
        return {std::bit_cast<daxa_Result>(result), ImageId{}};
    }

    if ((this->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && !image_info.name.empty())
    {
        auto c_str_arr = image_info.name.c_str();
        VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE,
            .objectHandle = std::bit_cast<uint64_t>(ret.vk_image),
            .pObjectName = c_str_arr.data(),
        };
        this->vkSetDebugUtilsObjectNameEXT(this->vk_device, &swapchain_image_name_info);

        VkDebugUtilsObjectNameInfoEXT const swapchain_image_view_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
            .objectHandle = std::bit_cast<uint64_t>(ret.view_slot.vk_image_view),
            .pObjectName = c_str_arr.data(),
        };
        this->vkSetDebugUtilsObjectNameEXT(this->vk_device, &swapchain_image_view_name_info);
    }

    write_descriptor_set_image(this->vk_device, this->gpu_sro_table.vk_descriptor_set, ret.view_slot.vk_image_view, usage, id.index);

    return {DAXA_RESULT_SUCCESS, ImageId{id}};
}

void daxa_ImplDevice::cleanup_buffer(BufferId id)
{
    auto gid = std::bit_cast<GPUResourceId>(id);
    ImplBufferSlot const & buffer_slot = this->gpu_sro_table.buffer_slots.unsafe_get(gid);
    this->buffer_device_address_buffer_host_ptr[gid.index] = 0;
    write_descriptor_set_buffer(this->vk_device, this->gpu_sro_table.vk_descriptor_set, this->vk_null_buffer, 0, VK_WHOLE_SIZE, gid.index);
    if (buffer_slot.opt_memory_block != nullptr)
    {
        vkDestroyBuffer(this->vk_device, buffer_slot.vk_buffer, {});
    }
    else
    {
        vmaDestroyBuffer(this->vma_allocator, buffer_slot.vk_buffer, buffer_slot.vma_allocation);
    }
    gpu_sro_table.buffer_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_image(ImageId id)
{
    _DAXA_TEST_PRINT("cleanup image\n");
    auto gid = std::bit_cast<GPUResourceId>(id);
    ImplImageSlot const & image_slot = gpu_sro_table.image_slots.unsafe_get(gid);
    write_descriptor_set_image(
        this->vk_device,
        this->gpu_sro_table.vk_descriptor_set,
        this->vk_null_image_view,
        std::bit_cast<ImageUsageFlags>(image_slot.info.usage),
        gid.index);
    vkDestroyImageView(vk_device, image_slot.view_slot.vk_image_view, nullptr);
    if (image_slot.swapchain_image_index == NOT_OWNED_BY_SWAPCHAIN)
    {
        if (image_slot.opt_memory_block != nullptr)
        {
            vkDestroyImage(this->vk_device, image_slot.vk_image, {});
        }
        else
        {
            vmaDestroyImage(this->vma_allocator, image_slot.vk_image, image_slot.vma_allocation);
        }
    }
    gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_image_view(ImageViewId id)
{
    DAXA_DBG_ASSERT_TRUE_M(gpu_sro_table.image_slots.unsafe_get(std::bit_cast<GPUResourceId>(id)).vk_image == VK_NULL_HANDLE, "can not destroy default image view of image");
    ImplImageViewSlot const & image_slot = gpu_sro_table.image_slots.unsafe_get(std::bit_cast<GPUResourceId>(id)).view_slot;
    write_descriptor_set_image(this->vk_device, this->gpu_sro_table.vk_descriptor_set, this->vk_null_image_view, ImageUsageFlagBits::SHADER_STORAGE | ImageUsageFlagBits::SHADER_SAMPLED, std::bit_cast<daxa::ImageViewId>(id).index);
    vkDestroyImageView(vk_device, image_slot.vk_image_view, nullptr);
    gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_sampler(SamplerId id)
{
    ImplSamplerSlot const & sampler_slot = this->gpu_sro_table.sampler_slots.unsafe_get(std::bit_cast<GPUResourceId>(id));
    write_descriptor_set_sampler(this->vk_device, this->gpu_sro_table.vk_descriptor_set, this->vk_null_sampler, std::bit_cast<GPUResourceId>(id).index);
    vkDestroySampler(this->vk_device, sampler_slot.vk_sampler, nullptr);
    gpu_sro_table.sampler_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_BufferId id) const -> ImplBufferSlot const &
{
    return gpu_sro_table.buffer_slots.unsafe_get(std::bit_cast<daxa::GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_ImageId id) const -> ImplImageSlot const &
{
    return gpu_sro_table.image_slots.unsafe_get(std::bit_cast<daxa::GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_ImageViewId id) const -> ImplImageViewSlot const &
{
    return gpu_sro_table.image_slots.unsafe_get(std::bit_cast<daxa::GPUResourceId>(id)).view_slot;
}

auto daxa_ImplDevice::slot(daxa_SamplerId id) const -> ImplSamplerSlot const &
{
    return gpu_sro_table.sampler_slots.unsafe_get(std::bit_cast<daxa::GPUResourceId>(id));
}

void daxa_ImplDevice::zero_ref_callback(ImplHandle const * handle)
{
    _DAXA_TEST_PRINT("daxa_ImplDevice::zero_ref_callback\n");
    auto self = rc_cast<daxa_Device>(handle);
    auto result = daxa_dvc_wait_idle(self);
    DAXA_DBG_ASSERT_TRUE_M(result == DAXA_RESULT_SUCCESS, "failed to wait idle");
    result = daxa_dvc_collect_garbage(self);
    DAXA_DBG_ASSERT_TRUE_M(result == DAXA_RESULT_SUCCESS, "failed to wait idle");
    self->buffer_pool_pool.cleanup(self);
    vmaUnmapMemory(self->vma_allocator, self->buffer_device_address_buffer_allocation);
    vmaDestroyBuffer(self->vma_allocator, self->buffer_device_address_buffer, self->buffer_device_address_buffer_allocation);
    self->gpu_sro_table.cleanup(self->vk_device);
    vmaDestroyImage(self->vma_allocator, self->vk_null_image, self->vk_null_image_vma_allocation);
    vmaDestroyBuffer(self->vma_allocator, self->vk_null_buffer, self->vk_null_buffer_vma_allocation);
    vmaDestroyAllocator(self->vma_allocator);
    vkDestroySampler(self->vk_device, self->vk_null_sampler, nullptr);
    vkDestroyImageView(self->vk_device, self->vk_null_image_view, nullptr);
    vkDestroySemaphore(self->vk_device, self->vk_main_queue_gpu_timeline_semaphore, nullptr);
    vkDestroyDevice(self->vk_device, nullptr);
    self->instance->dec_weak_refcnt(
        daxa_ImplInstance::zero_ref_callback,
        self->instance);
    delete self;
}

void daxa_ImplDevice::zombify_buffer(BufferId id)
{
    _DAXA_TEST_PRINT("daxa_ImplDevice::zombify_buffer\n");
    {
        auto & slot = gpu_sro_table.buffer_slots.unsafe_get(std::bit_cast<GPUResourceId>(id));
        if (slot.opt_memory_block != nullptr)
        {
            slot.opt_memory_block->dec_weak_refcnt(
                daxa_ImplMemoryBlock::zero_ref_callback,
                this->instance);
        }
        u64 const main_queue_cpu_timeline_value = this->main_queue_cpu_timeline.load(std::memory_order::relaxed);
        {
            std::unique_lock const lock{this->main_queue_zombies_mtx};
            this->main_queue_buffer_zombies.push_front({
                main_queue_cpu_timeline_value,
                std::bit_cast<BufferId>(id),
            });
        }
    }
}

void daxa_ImplDevice::zombify_image(ImageId id)
{
    _DAXA_TEST_PRINT("daxa_ImplDevice::zombify_image (%i,%i)\n", id.index, id.version);
    auto & slot = gpu_sro_table.image_slots.unsafe_get(std::bit_cast<GPUResourceId>(id));
    if (slot.opt_memory_block != nullptr)
    {
        slot.opt_memory_block->dec_weak_refcnt(
            daxa_ImplMemoryBlock::zero_ref_callback,
            this->instance);
    }
    u64 const main_queue_cpu_timeline_value = this->main_queue_cpu_timeline.load(std::memory_order::relaxed);
    {
        std::unique_lock const lock{this->main_queue_zombies_mtx};
        this->main_queue_image_zombies.push_front({
            main_queue_cpu_timeline_value,
            std::bit_cast<ImageId>(id),
        });
    }
}

void daxa_ImplDevice::zombify_image_view(ImageViewId id)
{
    _DAXA_TEST_PRINT("daxa_ImplDevice::zombify_image_view\n");
    u64 const main_queue_cpu_timeline_value = this->main_queue_cpu_timeline.load(std::memory_order::relaxed);
    std::unique_lock const lock{this->main_queue_zombies_mtx};
    this->main_queue_image_view_zombies.push_front({
        main_queue_cpu_timeline_value,
        std::bit_cast<ImageViewId>(id),
    });
}

void daxa_ImplDevice::zombify_sampler(SamplerId id)
{
    _DAXA_TEST_PRINT("daxa_ImplDevice::zombify_sampler\n");
    u64 const main_queue_cpu_timeline_value = this->main_queue_cpu_timeline.load(std::memory_order::relaxed);
    std::unique_lock const lock{this->main_queue_zombies_mtx};
    this->main_queue_sampler_zombies.push_front({
        main_queue_cpu_timeline_value,
        std::bit_cast<SamplerId>(id),
    });
}

// --- End Internal Functions ---