#include "impl_device.hpp"

#include <utility>
#include "impl_features.hpp"

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

#include "impl_device.hpp"

#include <utility>

// --- Begin API Functions ---

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
    // --- Begin Parameter Validation ---

    bool parameters_valid = true;
    // Size must be larger then one.
    parameters_valid = parameters_valid && info->size > 0;
    if (!parameters_valid)
        return DAXA_RESULT_INVALID_BUFFER_INFO;

    // --- End Parameter Validation ---

    auto [id, ret] = self->gpu_shader_resource_table.buffer_slots.new_slot();
    static_assert(sizeof(BufferInfo) == sizeof(daxa_BufferInfo));
    ret.info = *info;
    ret.info_name = {ret.info.name.data, ret.info.name.size};
    ret.info.name = {ret.info_name.data(), ret.info_name.size()};

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
    if (AutoAllocInfo const * auto_info = daxa::get_if<AutoAllocInfo>(
            r_cast<AllocateInfo *>(&ret.info.allocate_info)))
    {
        auto vma_allocation_flags = static_cast<VmaAllocationCreateFlags>(auto_info->data);
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
        auto const & manual_info = *daxa::get_if<ManualAllocInfo>(
            r_cast<AllocateInfo *>(&ret.info.allocate_info));
        auto const & mem_block = *manual_info.memory_block.as<daxa_ImplMemoryBlock const>();

        // TODO(pahrens): Add validation for memory type requirements.

        auto result = vkCreateBuffer(self->vk_device, &vk_buffer_create_info, nullptr, &ret.vk_buffer);
        if (result != VK_SUCCESS)
        {
            return std::bit_cast<daxa_Result>(result);
        }

        result = vmaBindBufferMemory2(
            self->vma_allocator,
            mem_block.allocation,
            manual_info.offset,
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
    ret.zombie = false;

    self->buffer_device_address_buffer_host_ptr[id.index] = ret.device_address;

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE &&
        !ret.info_name.empty())
    {
        auto const & buffer_name = ret.info_name;
        VkDebugUtilsObjectNameInfoEXT const buffer_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = reinterpret_cast<uint64_t>(ret.vk_buffer),
            .pObjectName = buffer_name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &buffer_name_info);
    }

    write_descriptor_set_buffer(
        self->vk_device,
        self->gpu_shader_resource_table.vk_descriptor_set, ret.vk_buffer,
        0,
        static_cast<VkDeviceSize>(ret.info.size),
        id.index);

    *out_id = std::bit_cast<daxa_BufferId>(id);
    self->inc_weak_refcnt();
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_create_image(daxa_Device self, daxa_ImageInfo const * info, daxa_ImageId * out_id) -> daxa_Result
{
    /// --- Begin Validation ---

    if (!(info->dimensions >= 1 && info->dimensions <= 3))
    {
        return DAXA_RESULT_INVALID_IMAGE_INFO;
    }

    /// --- End Validation ---

    auto [id, image_slot_variant] = self->gpu_shader_resource_table.image_slots.new_slot();

    ImplImageSlot ret = {};
    ret.info = *info;
    ret.info_name = {ret.info.name.data, ret.info.name.size};
    ret.info.name = {ret.info_name.data(), ret.info_name.size()};
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

    VkImageViewCreateInfo const vk_image_view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .image = ret.vk_image,
        .viewType = vk_image_view_type,
        .format = *reinterpret_cast<VkFormat const *>(&info->format),
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
    ret.aspect_flags = infer_aspect_from_format(info->format);
    VkImageCreateInfo const vk_image_create_info = initialize_image_create_info_from_image_info(*info, &self->main_queue_family_index);
    if (AutoAllocInfo const * auto_info = daxa::get_if<AutoAllocInfo>(
            r_cast<AllocateInfo *>(&ret.info.allocate_info)))
    {
        VmaAllocationCreateInfo const vma_allocation_create_info{
            .flags = static_cast<VmaAllocationCreateFlags>(auto_info->data),
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
            self->gpu_shader_resource_table.image_slots.return_slot(id);
            return DAXA_RESULT_FAILED_TO_CREATE_IMAGE;
        }

        result = vkCreateImageView(self->vk_device, &vk_image_view_create_info, nullptr, &ret.view_slot.vk_image_view);
        if (result != VK_SUCCESS)
        {
            vmaDestroyImage(self->vma_allocator, ret.vk_image, ret.vma_allocation);
            self->gpu_shader_resource_table.image_slots.return_slot(id);
            return DAXA_RESULT_FAILED_TO_CREATE_DEFAULT_IMAGE_VIEW;
        }
    }
    else
    {
        ManualAllocInfo const & manual_info = *daxa::get_if<ManualAllocInfo>(
            r_cast<AllocateInfo *>(&ret.info.allocate_info));
        daxa_ImplMemoryBlock const & mem_block = *manual_info.memory_block.as<daxa_ImplMemoryBlock const>();
        // TODO(pahrens): Add validation for memory requirements.
        auto result = vkCreateImage(self->vk_device, &vk_image_create_info, nullptr, &ret.vk_image);
        if (result != VK_SUCCESS)
        {
            self->gpu_shader_resource_table.image_slots.return_slot(id);
            return DAXA_RESULT_FAILED_TO_CREATE_IMAGE;
        }
        result = vmaBindImageMemory2(
            self->vma_allocator,
            mem_block.allocation,
            manual_info.offset,
            ret.vk_image,
            {});
        if (result != VK_SUCCESS)
        {
            vkDestroyImage(self->vk_device, ret.vk_image, nullptr);
            self->gpu_shader_resource_table.image_slots.return_slot(id);
            return DAXA_RESULT_FAILED_TO_CREATE_IMAGE;
        }

        result = vkCreateImageView(self->vk_device, &vk_image_view_create_info, nullptr, &ret.view_slot.vk_image_view);
        if (result != VK_SUCCESS)
        {

            vkDestroyImage(self->vk_device, ret.vk_image, nullptr);
            self->gpu_shader_resource_table.image_slots.return_slot(id);
            return DAXA_RESULT_FAILED_TO_CREATE_DEFAULT_IMAGE_VIEW;
        }
    }

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && info->name.size != 0)
    {
        VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE,
            .objectHandle = reinterpret_cast<uint64_t>(ret.vk_image),
            .pObjectName = info->name.data,
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &swapchain_image_name_info);

        VkDebugUtilsObjectNameInfoEXT const swapchain_image_view_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
            .objectHandle = reinterpret_cast<uint64_t>(ret.view_slot.vk_image_view),
            .pObjectName = info->name.data,
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &swapchain_image_view_name_info);
    }

    write_descriptor_set_image(
        self->vk_device,
        self->gpu_shader_resource_table.vk_descriptor_set,
        ret.view_slot.vk_image_view,
        std::bit_cast<ImageUsageFlags>(ret.info.usage),
        id.index);

    image_slot_variant = ret;

    *out_id = std::bit_cast<daxa_ImageId>(id);
    self->inc_weak_refcnt();
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_create_image_view(daxa_Device self, daxa_ImageViewInfo const * info, daxa_ImageViewId * out_id) -> daxa_Result
{
    /// --- Begin Validation ---

    /// --- End Validation ---

    auto [id, image_slot] = self->gpu_shader_resource_table.image_slots.new_slot();
    image_slot = {};
    ImplImageSlot const & parent_image_slot = self->slot(info->image);
    ImplImageViewSlot ret = {};
    ret.info = *info;
    ret.info_name = {ret.info.name.data, ret.info.name.size};
    ret.info.name = {ret.info_name.data(), ret.info_name.size()};
    daxa_ImageMipArraySlice slice = self->validate_image_slice(ret.info.slice, ret.info.image);
    ret.info.slice = slice;
    VkImageViewCreateInfo const vk_image_view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .image = parent_image_slot.vk_image,
        .viewType = static_cast<VkImageViewType>(ret.info.type),
        .format = *reinterpret_cast<VkFormat const *>(&ret.info.format),
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
        self->gpu_shader_resource_table.image_slots.return_slot(id);
        return DAXA_RESULT_FAILED_TO_CREATE_IMAGE_VIEW;
    }
    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && info->name.size != 0)
    {
        VkDebugUtilsObjectNameInfoEXT const name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
            .objectHandle = reinterpret_cast<uint64_t>(ret.vk_image_view),
            .pObjectName = ret.info_name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &name_info);
    }
    write_descriptor_set_image(
        self->vk_device,
        self->gpu_shader_resource_table.vk_descriptor_set,
        ret.vk_image_view,
        std::bit_cast<ImageUsageFlags>(parent_image_slot.info.usage),
        id.index);
    image_slot.view_slot = ret;
    *out_id = std::bit_cast<daxa_ImageViewId>(id);
    self->inc_weak_refcnt_image(std::bit_cast<ImageId>(info->image));
    self->inc_weak_refcnt();
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

    auto [id, ret] = self->gpu_shader_resource_table.sampler_slots.new_slot();

    ret.info = *info;
    ret.info_name = {ret.info.name.data, ret.info.name.size};
    ret.info.name = {ret.info_name.data(), ret.info_name.size()};
    ret.zombie = false;

    VkSamplerReductionModeCreateInfo vk_sampler_reduction_mode_create_info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO,
        .pNext = nullptr,
        .reductionMode = static_cast<VkSamplerReductionMode>(ret.info.reduction_mode),
    };

    VkSamplerCreateInfo const vk_sampler_create_info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = reinterpret_cast<void *>(&vk_sampler_reduction_mode_create_info),
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
        self->gpu_shader_resource_table.sampler_slots.return_slot(id);
        return DAXA_RESULT_FAILED_TO_CREATE_SAMPLER;
    }

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && info->name.size != 0)
    {
        VkDebugUtilsObjectNameInfoEXT const sampler_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_SAMPLER,
            .objectHandle = reinterpret_cast<uint64_t>(ret.vk_sampler),
            .pObjectName = ret.info_name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &sampler_name_info);
    }

    write_descriptor_set_sampler(self->vk_device, self->gpu_shader_resource_table.vk_descriptor_set, ret.vk_sampler, id.index);
    *out_id = std::bit_cast<daxa_SamplerId>(id);
    self->inc_weak_refcnt();
    return DAXA_RESULT_SUCCESS;
}

#define _DAXA_DECL_COMMON_GP_RES_FUNCTIONS(name, Name, NAME, SLOT_NAME)                                                    \
    auto daxa_dvc_inc_refcnt_##name(daxa_Device self, daxa_##Name##Id id)->uint64_t                                        \
    {                                                                                                                      \
        return std::atomic_ref{self->slot(id).strong_count}.fetch_add(1, std::memory_order::relaxed);                      \
    }                                                                                                                      \
    auto daxa_dvc_dec_refcnt_##name(daxa_Device self, daxa_##Name##Id id)->uint64_t                                        \
    {                                                                                                                      \
        u64 prev = std::atomic_ref{self->slot(id).strong_count}.fetch_sub(1, std::memory_order::relaxed);                  \
        if (prev == 1)                                                                                                     \
        {                                                                                                                  \
            auto weak = std::atomic_ref{self->slot(id).weak_count}.load(std::memory_order::relaxed);                       \
            if (weak == 0)                                                                                                 \
            {                                                                                                              \
                self->zero_ref_callback_##name(std::bit_cast<Name##Id>(id));                                               \
            }                                                                                                              \
            else if ((self->instance->info.flags & InstanceFlagBits::PARENT_MUST_OUTLIVE_CHILD) != InstanceFlagBits::NONE) \
            {                                                                                                              \
                DAXA_DBG_ASSERT_TRUE_M(false, "not all children have been destroyed prior to destroying object");          \
            }                                                                                                              \
        }                                                                                                                  \
        return prev;                                                                                                       \
    }                                                                                                                      \
    auto daxa_dvc_info_##name(daxa_Device self, daxa_##Name##Id id)->daxa_##Name##Info const *                             \
    {                                                                                                                      \
        return &self->slot(id).info;                                                                                       \
    }                                                                                                                      \
    auto daxa_dvc_is_##name##_valid(daxa_Device self, daxa_##Name##Id id)->daxa_Bool8                                      \
    {                                                                                                                      \
        return std::bit_cast<bool>(self->gpu_shader_resource_table.SLOT_NAME.is_id_valid(                                  \
            std::bit_cast<daxa::GPUResourceId>(id)));                                                                      \
    }                                                                                                                      \
    auto daxa_dvc_get_vk_##name(daxa_Device self, daxa_##Name##Id id)->Vk##Name                                            \
    {                                                                                                                      \
        return self->slot(id).vk_##name;                                                                                   \
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
    return reinterpret_cast<daxa_DeviceInfo const *>(&self->info);
}

auto daxa_dvc_get_vk_device(daxa_Device self) -> VkDevice
{
    return self->vk_device;
}

auto daxa_dvc_wait_idle(daxa_Device self) -> daxa_Result
{
    {
        auto result = vkQueueWaitIdle(self->main_queue_vk_queue);
        if (result != VK_SUCCESS)
        {
            return std::bit_cast<daxa_Result>(result);
        }
    }
    {
        auto result = vkDeviceWaitIdle(self->vk_device);
        if (result != VK_SUCCESS)
        {
            return std::bit_cast<daxa_Result>(result);
        }
    }
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_submit(daxa_Device self, daxa_CommandSubmitInfo const * info) -> daxa_Result
{
    daxa_dvc_collect_garbage(self);

    u64 const current_main_queue_cpu_timeline_value = self->main_queue_cpu_timeline.fetch_add(1) + 1;

    std::pair<u64, std::vector<ManagedPtr>> submit = {current_main_queue_cpu_timeline_value, {}};

    for (auto const & cmd_list : std::span{info->command_lists, info->command_list_count})
    {
        for (auto [id, index] : cmd_list->deferred_destructions)
        {
            switch (index)
            {
            case DEFERRED_DESTRUCTION_BUFFER_INDEX: self->main_queue_buffer_zombies.push_front({current_main_queue_cpu_timeline_value, BufferId{id}}); break;
            case DEFERRED_DESTRUCTION_IMAGE_INDEX: self->main_queue_image_zombies.push_front({current_main_queue_cpu_timeline_value, ImageId{id}}); break;
            case DEFERRED_DESTRUCTION_IMAGE_VIEW_INDEX: self->main_queue_image_view_zombies.push_front({current_main_queue_cpu_timeline_value, ImageViewId{id}}); break;
            case DEFERRED_DESTRUCTION_SAMPLER_INDEX: self->main_queue_sampler_zombies.push_front({current_main_queue_cpu_timeline_value, SamplerId{id}}); break;
            default: DAXA_DBG_ASSERT_TRUE_M(false, "unreachable");
            }
        }
    }

    std::vector<VkCommandBuffer> submit_vk_command_buffers = {};
    for (auto const & cmd_list : std::span{info->command_lists, info->command_list_count})
    {
        if (!cmd_list->recording_complete)
        {
            return DAXA_RESULT_INCOMPLETE_COMMAND_LIST;
        }
        submit.second.push_back(cmd_list);
        submit_vk_command_buffers.push_back(cmd_list->vk_cmd_buffer);
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
        .pNext = reinterpret_cast<void *>(&timeline_info),
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
    self->main_queue_submits_zombies.push_front(std::move(submit));

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

    daxa_dvc_collect_garbage(self);
    return std::bit_cast<daxa_Result>(result);
}

daxa_Result
daxa_dvc_collect_garbage(daxa_Device self)
{
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
    // Need to unlock, as command list destructor locks this mutex too.
    lock.unlock();
    check_and_cleanup_gpu_resources(
        self->main_queue_submits_zombies,
        [](auto & /* command_lists */) {});
    lock.lock();
    {
        std::unique_lock const l_lock{self->main_queue_command_pool_buffer_recycle_mtx};
        check_and_cleanup_gpu_resources(
            self->main_queue_command_list_zombies,
            [&](auto & command_list_zombie)
            {
                self->buffer_pool_pool.put_back({command_list_zombie.vk_cmd_pool, command_list_zombie.vk_cmd_buffer});
            });
    }
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
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_inc_refcnt(daxa_Device self) -> u64
{
    return self->inc_refcnt();
}

auto daxa_dvc_dec_refcnt(daxa_Device self) -> u64
{
    return self->dec_refcnt(
        &daxa_ImplDevice::zero_ref_callback,
        self->instance);
}

// --- End API Functions ---

// --- Begin Internal Functions ---

auto daxa_ImplDevice::create(daxa_Instance instance, daxa_DeviceInfo const & info, VkPhysicalDevice physical_device, daxa_Device out_device) -> daxa_Result
{
    VkResult result = {};

    auto self = out_device;
    self->vk_physical_device = physical_device;
    self->instance = instance;
    self->info = *reinterpret_cast<DeviceInfo const *>(&info);
    self->info_name = std::string{info.name.data, info.name.size};
    self->info.name = {self->info_name.c_str(), self->info_name.size()};
    using namespace daxa;

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
        .pNext = reinterpret_cast<void const *>(&physical_device_features_2),
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
        self->vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(self->vk_device, "vkSetDebugUtilsObjectNameEXT"));
        self->vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdBeginDebugUtilsLabelEXT"));
        self->vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdEndDebugUtilsLabelEXT"));
    }
    self->vkCmdPushDescriptorSetKHR = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetDeviceProcAddr(self->vk_device, "vkCmdPushDescriptorSetKHR"));

    if ((self->info.flags & DeviceFlagBits::MESH_SHADER_BIT) != DeviceFlagBits::NONE)
    {
        self->vkCmdDrawMeshTasksEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdDrawMeshTasksEXT"));
        self->vkCmdDrawMeshTasksIndirectEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdDrawMeshTasksIndirectEXT"));
        self->vkCmdDrawMeshTasksIndirectCountEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdDrawMeshTasksIndirectCountEXT"));
        auto * out_struct = reinterpret_cast<VkBaseOutStructure *>(&self->vk_physical_device_properties2);
        while (out_struct != nullptr)
        {
            if (out_struct->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT)
            {
                self->mesh_shader_properties = *reinterpret_cast<VkPhysicalDeviceMeshShaderPropertiesEXT *>(out_struct);
            }
            out_struct = out_struct->pNext;
        }
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
        .pNext = reinterpret_cast<void *>(&timeline_ci),
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
            *reinterpret_cast<daxa_ImageInfo const *>(&image_info), &self->main_queue_family_index);

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
            .format = *reinterpret_cast<VkFormat const *>(&image_info.format),
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
        vmaMapMemory(self->vma_allocator, self->buffer_device_address_buffer_allocation, reinterpret_cast<void **>(&self->buffer_device_address_buffer_host_ptr));
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

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && !self->info_name.empty())
    {
        auto const device_name = self->info_name;
        VkDebugUtilsObjectNameInfoEXT const device_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_DEVICE,
            .objectHandle = reinterpret_cast<uint64_t>(self->vk_device),
            .pObjectName = device_name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_name_info);

        auto const queue_name = self->info_name;
        VkDebugUtilsObjectNameInfoEXT const device_main_queue_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_QUEUE,
            .objectHandle = reinterpret_cast<uint64_t>(self->main_queue_vk_queue),
            .pObjectName = queue_name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_main_queue_name_info);

        auto const semaphore_name = self->info_name;
        VkDebugUtilsObjectNameInfoEXT const device_main_queue_timeline_semaphore_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_SEMAPHORE,
            .objectHandle = reinterpret_cast<uint64_t>(self->vk_main_queue_gpu_timeline_semaphore),
            .pObjectName = semaphore_name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_main_queue_timeline_semaphore_name_info);

        auto const buffer_name = self->info_name;
        VkDebugUtilsObjectNameInfoEXT const device_main_queue_timeline_buffer_device_address_buffer_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = reinterpret_cast<uint64_t>(self->buffer_device_address_buffer),
            .pObjectName = buffer_name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_main_queue_timeline_buffer_device_address_buffer_name_info);
    }

    self->gpu_shader_resource_table.initialize(
        self->info.max_allowed_buffers,
        self->info.max_allowed_images,
        self->info.max_allowed_samplers,
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
    auto [id, image_slot] = gpu_shader_resource_table.image_slots.new_slot();

    ImplImageSlot ret;
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
        VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE,
            .objectHandle = reinterpret_cast<uint64_t>(ret.vk_image),
            .pObjectName = image_info.name.data(),
        };
        this->vkSetDebugUtilsObjectNameEXT(this->vk_device, &swapchain_image_name_info);

        VkDebugUtilsObjectNameInfoEXT const swapchain_image_view_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
            .objectHandle = reinterpret_cast<uint64_t>(ret.view_slot.vk_image_view),
            .pObjectName = image_info.name.data(),
        };
        this->vkSetDebugUtilsObjectNameEXT(this->vk_device, &swapchain_image_view_name_info);
    }

    write_descriptor_set_image(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, ret.view_slot.vk_image_view, usage, id.index);

    image_slot = ret;

    return {DAXA_RESULT_SUCCESS, ImageId{id}};
}

void daxa_ImplDevice::cleanup_buffer(BufferId id)
{
    auto gid = std::bit_cast<GPUResourceId>(id);
    ImplBufferSlot & buffer_slot = this->gpu_shader_resource_table.buffer_slots.dereference_id(gid);
    this->buffer_device_address_buffer_host_ptr[gid.index] = 0;
    write_descriptor_set_buffer(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, this->vk_null_buffer, 0, VK_WHOLE_SIZE, gid.index);
    if (auto _ptr = daxa::get_if<AutoAllocInfo>(
            r_cast<AllocateInfo *>(&buffer_slot.info.allocate_info)))
    {
        vmaDestroyBuffer(this->vma_allocator, buffer_slot.vk_buffer, buffer_slot.vma_allocation);
    }
    else
    {
        vkDestroyBuffer(this->vk_device, buffer_slot.vk_buffer, {});
    }
    buffer_slot = {};
    gpu_shader_resource_table.buffer_slots.return_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_image(ImageId id)
{
    auto gid = std::bit_cast<GPUResourceId>(id);
    ImplImageSlot & image_slot = gpu_shader_resource_table.image_slots.dereference_id(gid);
    write_descriptor_set_image(
        this->vk_device,
        this->gpu_shader_resource_table.vk_descriptor_set,
        this->vk_null_image_view,
        std::bit_cast<ImageUsageFlags>(image_slot.info.usage),
        gid.index);
    vkDestroyImageView(vk_device, image_slot.view_slot.vk_image_view, nullptr);
    if (image_slot.swapchain_image_index == NOT_OWNED_BY_SWAPCHAIN)
    {
        if (auto _ptr = daxa::get_if<AutoAllocInfo>(
                r_cast<AllocateInfo *>(&image_slot.info.allocate_info)))
        {
            vmaDestroyImage(this->vma_allocator, image_slot.vk_image, image_slot.vma_allocation);
        }
        else
        {
            vkDestroyImage(this->vk_device, image_slot.vk_image, {});
        }
    }
    image_slot = {};
    gpu_shader_resource_table.image_slots.return_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_image_view(ImageViewId id)
{
    DAXA_DBG_ASSERT_TRUE_M(gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<GPUResourceId>(id)).vk_image == VK_NULL_HANDLE, "can not destroy default image view of image");
    ImplImageViewSlot & image_slot = gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<GPUResourceId>(id)).view_slot;
    write_descriptor_set_image(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, this->vk_null_image_view, ImageUsageFlagBits::SHADER_STORAGE | ImageUsageFlagBits::SHADER_SAMPLED, std::bit_cast<daxa::ImageViewId>(id).index);
    vkDestroyImageView(vk_device, image_slot.vk_image_view, nullptr);
    daxa_dvc_dec_refcnt_image(this, std::bit_cast<daxa_ImageId>(image_slot.info.image));
    image_slot = {};
    gpu_shader_resource_table.image_slots.return_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_sampler(SamplerId id)
{
    ImplSamplerSlot & sampler_slot = this->gpu_shader_resource_table.sampler_slots.dereference_id(std::bit_cast<GPUResourceId>(id));
    write_descriptor_set_sampler(this->vk_device, this->gpu_shader_resource_table.vk_descriptor_set, this->vk_null_sampler, std::bit_cast<GPUResourceId>(id).index);
    vkDestroySampler(this->vk_device, sampler_slot.vk_sampler, nullptr);
    sampler_slot = {};
    gpu_shader_resource_table.sampler_slots.return_slot(std::bit_cast<GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(BufferId id) -> ImplBufferSlot &
{
    return gpu_shader_resource_table.buffer_slots.dereference_id(id);
}

auto daxa_ImplDevice::slot(ImageId id) -> ImplImageSlot &
{
    return gpu_shader_resource_table.image_slots.dereference_id(id);
}

auto daxa_ImplDevice::slot(ImageViewId id) -> ImplImageViewSlot &
{
    return gpu_shader_resource_table.image_slots.dereference_id(id).view_slot;
}

auto daxa_ImplDevice::slot(SamplerId id) -> ImplSamplerSlot &
{
    return gpu_shader_resource_table.sampler_slots.dereference_id(id);
}

auto daxa_ImplDevice::slot(BufferId id) const -> ImplBufferSlot const &
{
    return gpu_shader_resource_table.buffer_slots.dereference_id(id);
}

auto daxa_ImplDevice::slot(ImageId id) const -> ImplImageSlot const &
{
    return gpu_shader_resource_table.image_slots.dereference_id(id);
}

auto daxa_ImplDevice::slot(ImageViewId id) const -> ImplImageViewSlot const &
{
    return gpu_shader_resource_table.image_slots.dereference_id(id).view_slot;
}

auto daxa_ImplDevice::slot(SamplerId id) const -> ImplSamplerSlot const &
{
    return gpu_shader_resource_table.sampler_slots.dereference_id(id);
}

auto daxa_ImplDevice::slot(daxa_BufferId id) -> ImplBufferSlot &
{
    return gpu_shader_resource_table.buffer_slots.dereference_id(std::bit_cast<daxa::GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_ImageId id) -> ImplImageSlot &
{
    return gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<daxa::GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_ImageViewId id) -> ImplImageViewSlot &
{
    return gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<daxa::GPUResourceId>(id)).view_slot;
}

auto daxa_ImplDevice::slot(daxa_SamplerId id) -> ImplSamplerSlot &
{
    return gpu_shader_resource_table.sampler_slots.dereference_id(std::bit_cast<daxa::GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_BufferId id) const -> ImplBufferSlot const &
{
    return gpu_shader_resource_table.buffer_slots.dereference_id(std::bit_cast<daxa::GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_ImageId id) const -> ImplImageSlot const &
{
    return gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<daxa::GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_ImageViewId id) const -> ImplImageViewSlot const &
{
    return gpu_shader_resource_table.image_slots.dereference_id(std::bit_cast<daxa::GPUResourceId>(id)).view_slot;
}

auto daxa_ImplDevice::slot(daxa_SamplerId id) const -> ImplSamplerSlot const &
{
    return gpu_shader_resource_table.sampler_slots.dereference_id(std::bit_cast<daxa::GPUResourceId>(id));
}

void daxa_ImplDevice::zero_ref_callback(daxa_ImplHandle * handle)
{
    auto self = r_cast<daxa_Device>(handle);
    daxa_dvc_wait_idle(self);
    daxa_dvc_collect_garbage(self);
    self->buffer_pool_pool.cleanup(self);
    vmaUnmapMemory(self->vma_allocator, self->buffer_device_address_buffer_allocation);
    vmaDestroyBuffer(self->vma_allocator, self->buffer_device_address_buffer, self->buffer_device_address_buffer_allocation);
    self->gpu_shader_resource_table.cleanup(self->vk_device);
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

#define _DAXA_DECL_COMMON_GP_RES_REFCNT_FUNCTIONS(name, Name, NAME, SLOT_NAME)                                            \
    auto daxa_ImplDevice::inc_weak_refcnt_##name(Name##Id id)->u64                                                        \
    {                                                                                                                     \
        return std::atomic_ref{this->slot(id).weak_count}.fetch_add(1, std::memory_order::relaxed);                       \
    }                                                                                                                     \
    auto daxa_ImplDevice::dec_weak_refcnt_##name(Name##Id id)->u64                                                        \
    {                                                                                                                     \
        auto & slot = this->slot(id);                                                                                     \
        auto prev = std::atomic_ref{slot.weak_count}.fetch_sub(1, std::memory_order::relaxed);                            \
        if (prev == 1)                                                                                                    \
        {                                                                                                                 \
            auto strong = std::atomic_ref{slot.weak_count}.load(std::memory_order::relaxed);                              \
            if (strong == 0)                                                                                              \
            {                                                                                                             \
                this->zero_ref_callback_##name(id);                                                                       \
            }                                                                                                             \
        }                                                                                                                 \
        return prev;                                                                                                      \
    }                                                                                                                     \
    void daxa_ImplDevice::zero_ref_callback_##name(Name##Id id)                                                           \
    {                                                                                                                     \
        std::unique_lock const lock{this->main_queue_zombies_mtx};                                                        \
        u64 const main_queue_cpu_timeline_value = this->main_queue_cpu_timeline.load(std::memory_order::relaxed);         \
        gpu_shader_resource_table.SLOT_NAME.dereference_id(std::bit_cast<GPUResourceId>(id)).zombie = true;               \
        this->main_queue_##name##_zombies.push_front({main_queue_cpu_timeline_value, std::bit_cast<daxa::Name##Id>(id)}); \
        this->dec_weak_refcnt(                                                                                            \
            daxa_ImplDevice::zero_ref_callback,                                                                           \
            this->instance);                                                                                              \
    }

_DAXA_DECL_COMMON_GP_RES_REFCNT_FUNCTIONS(buffer, Buffer, BUFFER, buffer_slots)
_DAXA_DECL_COMMON_GP_RES_REFCNT_FUNCTIONS(image, Image, IMAGE, image_slots)
_DAXA_DECL_COMMON_GP_RES_REFCNT_FUNCTIONS(image_view, ImageView, IMAGE_VIEW, image_slots)
_DAXA_DECL_COMMON_GP_RES_REFCNT_FUNCTIONS(sampler, Sampler, SAMPLER, sampler_slots)

// --- End Internal Functions ---