#include "impl_core.hpp"

#include "impl_device.hpp"

#include <utility>
#include <functional>
#include "impl_features.hpp"

#include "impl_device.hpp"

/// --- Begin Helpers ---

namespace
{
    auto initialize_image_create_info_from_image_info(daxa_Device self, daxa_ImageInfo const & image_info) -> VkImageCreateInfo
    {
        DAXA_DBG_ASSERT_TRUE_M(std::popcount(image_info.sample_count) == 1 && image_info.sample_count <= 8, "image samples must be power of two and between 1 and 64(inclusive)");
        DAXA_DBG_ASSERT_TRUE_M(
            image_info.size.width > 0 &&
                image_info.size.height > 0 &&
                image_info.size.depth > 0,
            "image (x,y,z) dimensions must be greater then 0");
        DAXA_DBG_ASSERT_TRUE_M(image_info.array_layer_count > 0, "image array layer count must be greater then 0");
        DAXA_DBG_ASSERT_TRUE_M(image_info.mip_level_count > 0, "image mip level count must be greater then 0");

        auto const vk_image_type = static_cast<VkImageType>(image_info.dimensions - 1);

        VkImageCreateFlags vk_image_create_flags = static_cast<VkImageCreateFlags>(image_info.flags);

        VkImageCreateInfo vk_image_create_info{
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
            .pQueueFamilyIndices = &self->get_queue(DAXA_QUEUE_MAIN).vk_queue_family_index,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        if (image_info.sharing_mode == daxa_SharingMode::DAXA_SHARING_MODE_CONCURRENT)
        {
            vk_image_create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            vk_image_create_info.queueFamilyIndexCount = self->valid_vk_queue_family_count;
            vk_image_create_info.pQueueFamilyIndices = self->valid_vk_queue_families.data();
        }
        return vk_image_create_info;
    }
    using namespace daxa::types;

    inline auto create_buffer_use_flags(daxa_Device self) -> VkBufferUsageFlags
    {
        VkBufferUsageFlags result = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        if (self->properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_BASIC_RAY_TRACING)
        {
            result |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                      VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                      VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
        }
        return result;
    }
} // namespace

auto daxa_ImplDevice::ImplQueue::initialize(daxa_Device device) -> daxa_Result
{
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

    auto vk_device = device->vk_device;

    VK_CALL_D(device, vkGetDeviceQueue, vk_device, vk_queue_family_index, queue_index, &this->vk_queue);
    daxa_Result result = DAXA_RESULT_SUCCESS;
    if (this->vk_queue == VK_NULL_HANDLE)
    {
        result = DAXA_RESULT_ERROR_COULD_NOT_QUERY_QUEUE;
    }
    _DAXA_RETURN_IF_ERROR(result, result)

    result = static_cast<daxa_Result>(VK_CALL_D(device, vkCreateSemaphore, vk_device, &vk_semaphore_create_info, nullptr, &this->gpu_queue_local_timeline));
    _DAXA_RETURN_IF_ERROR(result, result)

    return result;
}

void daxa_ImplDevice::ImplQueue::cleanup(daxa_Device device)
{
    if (this->gpu_queue_local_timeline)
    {
        VK_CALL_D(device, vkDestroySemaphore, device->vk_device, this->gpu_queue_local_timeline, nullptr);
    }
}

auto daxa_ImplDevice::ImplQueue::get_oldest_pending_submit(daxa_Device device, std::optional<u64> & out) -> daxa_Result
{
    if (this->gpu_queue_local_timeline)
    {
        u64 latest_gpu = {};
        auto result = static_cast<daxa_Result>(VK_CALL_D(device, vkGetSemaphoreCounterValue, device->vk_device, this->gpu_queue_local_timeline, &latest_gpu));
        _DAXA_RETURN_IF_ERROR(result, result);

        u64 latest_cpu = this->latest_pending_submit_timeline_value.load(std::memory_order::acquire);

        bool const cpu_ahead_of_gpu = latest_cpu > latest_gpu;
        if (cpu_ahead_of_gpu)
        {
            out = latest_gpu;
        }
    }
    return DAXA_RESULT_SUCCESS;
}

auto create_buffer_helper(daxa_Device self, daxa_BufferInfo const * info, daxa_BufferId * out_id, daxa_MemoryBlock opt_memory_block, usize opt_offset) -> daxa_Result
{
    daxa_Result result = DAXA_RESULT_SUCCESS;
    // --- Begin Parameter Validation ---

    bool parameters_valid = true;
    // Size must be larger then one.
    parameters_valid = parameters_valid && info->size > 0;
    if (!parameters_valid)
    {
        result = DAXA_RESULT_INVALID_BUFFER_INFO;
    }
    _DAXA_RETURN_IF_ERROR(result, result)

    // --- End Parameter Validation ---

    auto slot_opt = self->gpu_sro_table.buffer_slots.try_create_slot();
    if (!slot_opt.has_value())
    {
        result = DAXA_RESULT_EXCEEDED_MAX_BUFFERS;
    }
    _DAXA_RETURN_IF_ERROR(result, result)

    auto [id, ret] = slot_opt.value();

    defer
    {
        if (result != DAXA_RESULT_SUCCESS)
        {
            self->gpu_sro_table.buffer_slots.unsafe_destroy_zombie_slot(id);
            if (ret.vk_buffer)
            {
                VK_CALL_D(self, vkDestroyBuffer, self->vk_device, ret.vk_buffer, nullptr);
            }
        }
    };

    ret.info = *info;

    VkBufferCreateInfo const vk_buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .size = static_cast<VkDeviceSize>(ret.info.size),
        .usage = create_buffer_use_flags(self),
        .sharingMode = VK_SHARING_MODE_CONCURRENT,                  // Buffers are always shared.
        .queueFamilyIndexCount = self->valid_vk_queue_family_count, // Buffers are always shared across all queues.
        .pQueueFamilyIndices = self->valid_vk_queue_families.data(),
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

        result = static_cast<daxa_Result>(vmaCreateBuffer(
            self->vma_allocator,
            &vk_buffer_create_info,
            &vma_allocation_create_info,
            &ret.vk_buffer,
            &ret.vma_allocation,
            &vma_allocation_info));
        _DAXA_RETURN_IF_ERROR(result, result)
    }
    else
    {
        auto const & mem_block = *opt_memory_block;
        ret.opt_memory_block = opt_memory_block;
        opt_memory_block->inc_weak_refcnt();

        result = static_cast<daxa_Result>(VK_CALL_D(self, vkCreateBuffer, self->vk_device, &vk_buffer_create_info, nullptr, &ret.vk_buffer));
        _DAXA_RETURN_IF_ERROR(result, result)

        result = static_cast<daxa_Result>(vmaBindBufferMemory2(
            self->vma_allocator,
            mem_block.allocation,
            opt_offset,
            ret.vk_buffer,
            {}));
        _DAXA_RETURN_IF_ERROR(result, result)
    }

    VkBufferDeviceAddressInfo const vk_buffer_device_address_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = ret.vk_buffer,
    };

    ret.device_address = VK_CALL_D(self, vkGetBufferDeviceAddress,self->vk_device, &vk_buffer_device_address_info);

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
        VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &buffer_name_info);
    }

    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_buffer(
            self,
            self->gpu_sro_table.vk_descriptor_set, ret.vk_buffer,
            0,
            static_cast<VkDeviceSize>(ret.info.size),
            id.index);
    }

    *out_id = std::bit_cast<daxa_BufferId>(id);
    return result;
}

auto create_image_helper(daxa_Device self, daxa_ImageInfo const * info, daxa_ImageId * out_id, daxa_MemoryBlock opt_memory_block, usize opt_offset) -> daxa_Result
{
    daxa_Result result = DAXA_RESULT_SUCCESS;
    /// --- Begin Validation ---

    if (!(info->dimensions >= 1 && info->dimensions <= 3))
    {
        return DAXA_RESULT_INVALID_IMAGE_INFO;
    }

    /// --- End Validation ---

    auto slot_opt = self->gpu_sro_table.image_slots.try_create_slot();
    if (!slot_opt.has_value())
    {
        result = DAXA_RESULT_EXCEEDED_MAX_IMAGES;
    }
    _DAXA_RETURN_IF_ERROR(result, result)

    auto [id, ret] = slot_opt.value();
    defer
    {
        if (result != DAXA_RESULT_SUCCESS)
        {
            if (ret.vk_image)
            {
                vmaDestroyImage(self->vma_allocator, ret.vk_image, ret.vma_allocation);
            }
            if (ret.view_slot.vk_image_view)
            {
                VK_CALL_D(self, vkDestroyImageView, self->vk_device, ret.view_slot.vk_image_view, nullptr);
            }
        }
    };

    VkImageViewType vk_image_view_type = {};
    if (info->array_layer_count > 1)
    {
        vk_image_view_type = static_cast<VkImageViewType>(info->dimensions + 3);
    }
    else
    {
        vk_image_view_type = static_cast<VkImageViewType>(info->dimensions - 1);
    }

    ret.info = *info;
    ret.view_slot.info = std::bit_cast<daxa_ImageViewInfo>(ImageViewInfo{
        .type = static_cast<ImageViewType>(vk_image_view_type),
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
    VkImageCreateInfo const vk_image_create_info = initialize_image_create_info_from_image_info(self, *info);
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

        VmaAllocationInfo allocation_info {};
        result = static_cast<daxa_Result>(vmaCreateImage(self->vma_allocator, &vk_image_create_info, &vma_allocation_create_info, &ret.vk_image, &ret.vma_allocation, &allocation_info));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_IMAGE);
        ret.vk_dev_mem = allocation_info.deviceMemory;

        vk_image_view_create_info.image = ret.vk_image;
        result = static_cast<daxa_Result>(VK_CALL_D(self, vkCreateImageView, self->vk_device, &vk_image_view_create_info, nullptr, &ret.view_slot.vk_image_view));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_DEFAULT_IMAGE_VIEW);
    }
    else
    {
        daxa_ImplMemoryBlock const & mem_block = *opt_memory_block;
        ret.opt_memory_block = opt_memory_block;
        opt_memory_block->inc_weak_refcnt();
        // TODO(pahrens): Add validation for memory requirements.
        result = static_cast<daxa_Result>(VK_CALL_D(self, vkCreateImage, self->vk_device, &vk_image_create_info, nullptr, &ret.vk_image));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_IMAGE);

        result = static_cast<daxa_Result>(vmaBindImageMemory2(
            self->vma_allocator,
            mem_block.allocation,
            opt_offset,
            ret.vk_image,
            {}));
        _DAXA_RETURN_IF_ERROR(result, result);

        VmaAllocationInfo2 allocation_info {};
        vmaGetAllocationInfo2(self->vma_allocator,
                            mem_block.allocation,
                            &allocation_info);
        ret.vk_dev_mem = allocation_info.allocationInfo.deviceMemory;

        vk_image_view_create_info.image = ret.vk_image;
        result = static_cast<daxa_Result>(VK_CALL_D(self, vkCreateImageView, self->vk_device, &vk_image_view_create_info, nullptr, &ret.view_slot.vk_image_view));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_DEFAULT_IMAGE_VIEW);
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
        VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &swapchain_image_name_info);

        VkDebugUtilsObjectNameInfoEXT const swapchain_image_view_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
            .objectHandle = std::bit_cast<uint64_t>(ret.view_slot.vk_image_view),
            .pObjectName = c_str_arr.data(),
        };
        VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &swapchain_image_view_name_info);
    }

    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_image(
            self,
            self->gpu_sro_table.vk_descriptor_set,
            ret.view_slot.vk_image_view,
            std::bit_cast<ImageUsageFlags>(ret.info.usage),
            id.index);
    }
    *out_id = std::bit_cast<daxa_ImageId>(id);
    return result;
}

auto create_acceleration_structure_helper(
    daxa_Device self,
    auto & table,
    VkAccelerationStructureTypeKHR vk_as_type,
    auto const & info,
    daxa_BufferId const * buffer,
    u64 const * offset,
    auto * out_id) -> daxa_Result
{
    daxa_Result result = DAXA_RESULT_SUCCESS;
    // --- Begin Parameter Validation ---

    if ((self->properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_BASIC_RAY_TRACING) == 0)
    {
        result = DAXA_RESULT_INVALID_WITHOUT_ENABLING_RAY_TRACING;
    }
    _DAXA_RETURN_IF_ERROR(result, result);

    // --- End Parameter Validation ---

    auto slot_opt = table.try_create_slot();
    if (!slot_opt.has_value())
    {
        result = DAXA_RESULT_EXCEEDED_MAX_ACCELERATION_STRUCTURES;
    }
    _DAXA_RETURN_IF_ERROR(result, result);

    auto [id, ret] = slot_opt.value();
    defer
    {
        if (result != DAXA_RESULT_SUCCESS)
        {
            table.unsafe_destroy_zombie_slot(id);
            if (!ret.buffer_id.is_empty())
            {
                [[maybe_unused]] auto const _ignore = daxa_dvc_destroy_buffer(self, ret.buffer_id);
            }
        }
    };

    ret.info = info;

    if (buffer)
    {
        ret.buffer_id = std::bit_cast<daxa::BufferId>(*buffer);
        ret.offset = *offset;
        ret.owns_buffer = false;
    }
    else
    {
        daxa::SmallString buffer_name{std::string_view{ret.info.name.data, ret.info.name.size}};
        if (ret.info.name.size < DAXA_SMALL_STRING_CAPACITY)
            buffer_name.push_back(' ');
        if (ret.info.name.size < DAXA_SMALL_STRING_CAPACITY)
            buffer_name.push_back('b');
        if (ret.info.name.size < DAXA_SMALL_STRING_CAPACITY)
            buffer_name.push_back('u');
        if (ret.info.name.size < DAXA_SMALL_STRING_CAPACITY)
            buffer_name.push_back('f');
        auto cinfo = daxa_BufferInfo{
            .size = ret.info.size,
            .allocate_info = {},
            .name = std::bit_cast<daxa_SmallString>(buffer_name),
        };
        result = daxa_dvc_create_buffer(self, &cinfo, r_cast<daxa_BufferId *>(&ret.buffer_id));
        _DAXA_RETURN_IF_ERROR(result, result);
        ret.offset = 0;
        ret.owns_buffer = true;
    }
    ret.vk_buffer = self->slot(ret.buffer_id).vk_buffer;

    VkAccelerationStructureCreateInfoKHR vk_create_info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .createFlags = {}, // VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR,
        .buffer = self->slot(ret.buffer_id).vk_buffer,
        .offset = ret.offset,
        .size = ret.info.size,
        .type = vk_as_type,
        .deviceAddress = {},
    };
    result = static_cast<daxa_Result>(VK_CALL_D_EXT(self, 
        vkCreateAccelerationStructureKHR, 
        self->vk_device, 
        &vk_create_info, 
        nullptr, 
        &ret.vk_acceleration_structure));
    _DAXA_RETURN_IF_ERROR(result, result);

    auto vk_acceleration_structure_device_address_info_khr = VkAccelerationStructureDeviceAddressInfoKHR{
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
        .pNext = nullptr,
        .accelerationStructure = ret.vk_acceleration_structure,
    };
    ret.device_address = VK_CALL_D_EXT(self,
        vkGetAccelerationStructureDeviceAddressKHR,
        self->vk_device,
        &vk_acceleration_structure_device_address_info_khr);

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && ret.info.name.size != 0)
    {
        auto c_str_arr = r_cast<SmallString const *>(&ret.info.name)->c_str();
        VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR,
            .objectHandle = std::bit_cast<uint64_t>(ret.vk_acceleration_structure),
            .pObjectName = c_str_arr.data(),
        };
        VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &swapchain_image_name_info);
    }

    if (vk_as_type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR)
    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_acceleration_structure(
            self,
            self->gpu_sro_table.vk_descriptor_set,
            ret.vk_acceleration_structure,
            id.index);
    }

    *out_id = std::bit_cast<typename std::remove_pointer<decltype(out_id)>::type>(id);
    return result;
}

/// --- End Helpers ---

// --- Begin API Functions ---

auto daxa_dvc_device_memory_report(daxa_Device self, daxa_DeviceMemoryReport * report) -> daxa_Result
{
    if (report == nullptr)
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_MEMORY_MAP_FAILED, DAXA_RESULT_ERROR_MEMORY_MAP_FAILED);
    }

    std::shared_lock lifetime_lock{self->gpu_sro_table.lifetime_lock};

    auto const buffer_list_allocation_size = report->buffer_count;
    auto const image_list_allocation_size = report->image_count;
    auto const tlas_list_allocation_size = report->tlas_count;
    auto const blas_list_allocation_size = report->blas_count;
    auto const memory_block_list_allocation_size = report->memory_block_count;

    report->buffer_count = {};
    report->image_count = {};
    report->tlas_count = {};
    report->blas_count = {};
    report->memory_block_count = {};
    report->total_buffer_device_memory_use = {};
    report->total_image_device_memory_use = {};
    report->total_aliased_tlas_device_memory_use = {};
    report->total_aliased_blas_device_memory_use = {};
    report->total_memory_block_device_memory_use = {};

    // TODO: Refactor all objects to live in arrays so we can always iterate all objects.
    std::unordered_map<daxa_MemoryBlock, u32> mem_blocks = {};

    for (u32 bi = 0; bi < self->gpu_sro_table.buffer_slots.next_index; ++bi)
    {
        u64 version = self->gpu_sro_table.buffer_slots.version_of_slot(bi);
        if ((version & GpuResourcePool<u32>::VERSION_ZOMBIE_BIT) == 0) 
        {
            daxa::BufferId id = { bi, version };
            auto& slot = self->gpu_sro_table.buffer_slots.unsafe_get(id);
            if (slot.vk_buffer == nullptr)
            {
                continue;
            }

            u32 out_idx = report->buffer_count++;

            bool const aliased = slot.vma_allocation == nullptr;
            u64 memory_size = {};
            if (!aliased)
            {
                VmaAllocationInfo vma_alloc_info = {};
                vmaGetAllocationInfo(self->vma_allocator, slot.vma_allocation, &vma_alloc_info);
                memory_size = vma_alloc_info.size;
                report->total_buffer_device_memory_use += memory_size;
            }
            else
            {
                auto requirements = daxa_dvc_buffer_memory_requirements(self, &slot.info);
                memory_size = requirements.size;
            }

            if (slot.opt_memory_block != nullptr)
            {
                mem_blocks[slot.opt_memory_block] += 1;
            }
    
            if (report->buffer_list != nullptr && out_idx < buffer_list_allocation_size)
            {
                report->buffer_list[out_idx] = {
                    std::bit_cast<daxa_BufferId>(id),
                    memory_size,
                    aliased,
                };
            }
        }
    }

    for (u32 ii = 0; ii < self->gpu_sro_table.image_slots.next_index; ++ii)
    {
        u64 version = self->gpu_sro_table.image_slots.version_of_slot(ii);
        if ((version & GpuResourcePool<u32>::VERSION_ZOMBIE_BIT) == 0) 
        {
            daxa::ImageId id = { ii, version };
            auto& slot = self->gpu_sro_table.image_slots.unsafe_get(id);
            if (slot.vk_image == nullptr)
            {
                continue;
            }

            u32 out_idx = report->image_count++;

            bool const aliased = slot.vma_allocation == nullptr;
            u64 memory_size = {};
            if (!aliased)
            {
                VmaAllocationInfo vma_alloc_info = {};
                vmaGetAllocationInfo(self->vma_allocator, slot.vma_allocation, &vma_alloc_info);
                memory_size = vma_alloc_info.size;
                report->total_image_device_memory_use += memory_size;
            }
            else
            {
                auto requirements = daxa_dvc_image_memory_requirements(self, &slot.info);
                memory_size = requirements.size;
            }

            if (slot.opt_memory_block != nullptr)
            {
                mem_blocks[slot.opt_memory_block] += 1;
            }

            if (report->image_list != nullptr && out_idx < image_list_allocation_size)
            {
                report->image_list[out_idx] = {
                    std::bit_cast<daxa_ImageId>(id),
                    memory_size,
                    aliased,
                };
            }
        }
    }
    
    for (u32 ti = 0; ti < self->gpu_sro_table.tlas_slots.next_index; ++ti)
    {
        u64 version = self->gpu_sro_table.tlas_slots.version_of_slot(ti);
        if ((version & GpuResourcePool<u32>::VERSION_ZOMBIE_BIT) == 0) 
        {
            daxa::TlasId id = { ti, version };
            auto& slot = self->gpu_sro_table.tlas_slots.unsafe_get(id);
            if (slot.vk_acceleration_structure == nullptr)
            {
                continue;
            }

            u32 out_idx = report->tlas_count++;
            report->total_aliased_tlas_device_memory_use += slot.info.size;
    
            if (report->tlas_list != nullptr && out_idx < tlas_list_allocation_size)
            {
                report->tlas_list[out_idx] = {
                    std::bit_cast<daxa_TlasId>(id),
                    slot.info.size
                };
            }
        }
    }
    
    for (u32 bli = 0; bli < self->gpu_sro_table.blas_slots.next_index; ++bli)
    {
        u64 version = self->gpu_sro_table.blas_slots.version_of_slot(bli);
        if ((version & GpuResourcePool<u32>::VERSION_ZOMBIE_BIT) == 0) 
        {
            daxa::BlasId id = { bli, version };
            auto& slot = self->gpu_sro_table.blas_slots.unsafe_get(id);
            if (slot.vk_acceleration_structure == nullptr)
            {
                continue;
            }

            u32 out_idx = report->blas_count++;
            report->total_aliased_blas_device_memory_use += slot.info.size;
    
            if (report->blas_list != nullptr && out_idx < blas_list_allocation_size)
            {
                report->blas_list[out_idx] = {
                    std::bit_cast<daxa_BlasId>(id),
                    slot.info.size
                };
            }
        }
    }

    for (auto v : mem_blocks)
    {
        daxa_MemoryBlock const& block = v.first;
        u32 out_idx = report->memory_block_count++;

        report->total_memory_block_device_memory_use += block->alloc_info.size;

        if (report->memory_block_list != nullptr && out_idx < memory_block_list_allocation_size)
        {
            block->inc_refcnt();
            report->memory_block_list[out_idx] = {
                block,
                block->alloc_info.size,
            };
        }
    }
    
    report->total_device_memory_use = 
        report->total_buffer_device_memory_use +
        report->total_image_device_memory_use +
        report->total_memory_block_device_memory_use;

    return DAXA_RESULT_SUCCESS;
} 

auto daxa_default_device_score(daxa_DeviceProperties const * c_properties) -> i32
{
    DeviceProperties const * properties = r_cast<DeviceProperties const *>(c_properties);
    i32 score = 0;
    // TODO: Maybe just return an unconditional score of 1 to make it so the
    // first GPU is selected. In `daxa_instance_create_device`, incompatible
    // devices should be discarded.
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
        .usage = create_buffer_use_flags(self),
        .sharingMode = VK_SHARING_MODE_CONCURRENT,                  // Buffers are always shared.
        .queueFamilyIndexCount = self->valid_vk_queue_family_count, // Buffers are always shared across all queues.
        .pQueueFamilyIndices = self->valid_vk_queue_families.data(),
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
    VK_CALL_D(self, vkGetDeviceBufferMemoryRequirements, self->vk_device, &buffer_requirement_info, &mem_requirements);
    // MemoryRequirements ret = std::bit_cast<MemoryRequirements>(mem_requirements.memoryRequirements);
    return mem_requirements.memoryRequirements;
}

auto daxa_dvc_image_memory_requirements(daxa_Device self, daxa_ImageInfo const * info) -> VkMemoryRequirements
{
    VkImageCreateInfo vk_image_create_info = initialize_image_create_info_from_image_info(self, *info);
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
    VK_CALL_D(self, vkGetDeviceImageMemoryRequirements, self->vk_device, &image_requirement_info, &mem_requirements);
    return mem_requirements.memoryRequirements;
}

auto daxa_dvc_get_tlas_build_sizes(
    daxa_Device self,
    daxa_TlasBuildInfo const * build_info,
    daxa_AccelerationStructureBuildSizesInfo * out)
    -> daxa_Result
{
    if ((self->properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_BASIC_RAY_TRACING) == 0)
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_INVALID_WITHOUT_ENABLING_RAY_TRACING, DAXA_RESULT_INVALID_WITHOUT_ENABLING_RAY_TRACING);
    }
    std::vector<VkAccelerationStructureBuildGeometryInfoKHR> vk_build_geometry_infos = {};
    std::vector<VkAccelerationStructureGeometryKHR> vk_geometry_infos = {};
    std::vector<u32> primitive_counts = {};
    std::vector<u32 const *> primitive_counts_ptrs = {};
    daxa_as_build_info_to_vk(
        self,
        build_info, // tlas array
        1,          // tlas array size
        nullptr,    // blas array
        0,          // blas array size
        vk_build_geometry_infos,
        vk_geometry_infos,
        primitive_counts,
        primitive_counts_ptrs);
    VkAccelerationStructureBuildSizesInfoKHR vk_acceleration_structure_build_sizes_info_khr = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
        .pNext = nullptr,
    };
    VK_CALL_D_EXT(self,
        vkGetAccelerationStructureBuildSizesKHR,
        self->vk_device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        vk_build_geometry_infos.data(),
        primitive_counts.data(),
        &vk_acceleration_structure_build_sizes_info_khr);
    out->acceleration_structure_size = vk_acceleration_structure_build_sizes_info_khr.accelerationStructureSize;
    out->build_scratch_size = vk_acceleration_structure_build_sizes_info_khr.buildScratchSize;
    out->update_scratch_size = vk_acceleration_structure_build_sizes_info_khr.updateScratchSize;
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_get_blas_build_sizes(
    daxa_Device self,
    daxa_BlasBuildInfo const * build_info,
    daxa_AccelerationStructureBuildSizesInfo * out)
    -> daxa_Result
{
    if ((self->properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_BASIC_RAY_TRACING) == 0)
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_INVALID_WITHOUT_ENABLING_RAY_TRACING, DAXA_RESULT_INVALID_WITHOUT_ENABLING_RAY_TRACING);
    }
    std::vector<VkAccelerationStructureBuildGeometryInfoKHR> vk_build_geometry_infos = {};
    std::vector<VkAccelerationStructureGeometryKHR> vk_geometry_infos = {};
    std::vector<u32> primitive_counts = {};
    std::vector<u32 const *> primitive_counts_ptrs = {};
    daxa_as_build_info_to_vk(
        self,
        nullptr,    // tlas array
        0,          // tlas array size
        build_info, // blas array
        1,          // blas array size
        vk_build_geometry_infos,
        vk_geometry_infos,
        primitive_counts,
        primitive_counts_ptrs);
    VkAccelerationStructureBuildSizesInfoKHR vk_acceleration_structure_build_sizes_info_khr = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
        .pNext = nullptr,
    };
    VK_CALL_D_EXT(self, vkGetAccelerationStructureBuildSizesKHR,
        self->vk_device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        vk_build_geometry_infos.data(),
        primitive_counts.data(),
        &vk_acceleration_structure_build_sizes_info_khr);
    out->acceleration_structure_size = vk_acceleration_structure_build_sizes_info_khr.accelerationStructureSize;
    out->build_scratch_size = vk_acceleration_structure_build_sizes_info_khr.buildScratchSize;
    out->update_scratch_size = vk_acceleration_structure_build_sizes_info_khr.updateScratchSize;
    return DAXA_RESULT_SUCCESS;
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

auto daxa_dvc_create_tlas(daxa_Device self, daxa_TlasInfo const * info, daxa_TlasId * out_id) -> daxa_Result
{
    return create_acceleration_structure_helper(
        self,
        self->gpu_sro_table.tlas_slots,
        VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        *info,
        nullptr,
        nullptr,
        out_id);
}

auto daxa_dvc_create_blas(daxa_Device self, daxa_BlasInfo const * info, daxa_BlasId * out_id) -> daxa_Result
{
    return create_acceleration_structure_helper(
        self,
        self->gpu_sro_table.blas_slots,
        VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        *info,
        nullptr,
        nullptr,
        out_id);
}

auto daxa_dvc_create_tlas_from_buffer(daxa_Device self, daxa_BufferTlasInfo const * info, daxa_TlasId * out_id) -> daxa_Result
{
    return create_acceleration_structure_helper(
        self,
        self->gpu_sro_table.tlas_slots,
        VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        info->tlas_info,
        &info->buffer_id,
        &info->offset,
        out_id);
}

auto daxa_dvc_create_blas_from_buffer(daxa_Device self, daxa_BufferBlasInfo const * info, daxa_BlasId * out_id) -> daxa_Result
{
    return create_acceleration_structure_helper(
        self,
        self->gpu_sro_table.blas_slots,
        VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        info->blas_info,
        &info->buffer_id,
        &info->offset,
        out_id);
}

auto daxa_dvc_create_image_view(daxa_Device self, daxa_ImageViewInfo const * info, daxa_ImageViewId * out_id) -> daxa_Result
{
    daxa_Result result = DAXA_RESULT_SUCCESS;

    auto slot_opt = self->gpu_sro_table.image_slots.try_create_slot();
    if (!slot_opt.has_value())
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_EXCEEDED_MAX_IMAGE_VIEWS, DAXA_RESULT_EXCEEDED_MAX_IMAGE_VIEWS);
    }
    auto [id, image_slot] = slot_opt.value();
    defer
    {
        if (result != DAXA_RESULT_SUCCESS)
        {
            self->gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(id);
            if (image_slot.view_slot.vk_image_view)
            {
                VK_CALL_D(self, vkDestroyImageView, self->vk_device, image_slot.view_slot.vk_image_view, nullptr);
            }
        }
    };

    ImplImageSlot const & parent_image_slot = self->slot(info->image);

    /// --- Begin Validation ---

    if (info->slice.layer_count > 1)
    {
        bool const array_type = info->type == 
            VK_IMAGE_VIEW_TYPE_1D_ARRAY || 
            info->type == VK_IMAGE_VIEW_TYPE_2D_ARRAY || 
            info->type == VK_IMAGE_VIEW_TYPE_CUBE || 
            info->type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        if (!array_type)
        {
            result = DAXA_RESULT_INVALID_IMAGE_VIEW_INFO;
        }
    }
    _DAXA_RETURN_IF_ERROR(result,result);

    /// --- End Validation ---

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
    result = static_cast<daxa_Result>(VK_CALL_D(self, vkCreateImageView, self->vk_device, &vk_image_view_create_info, nullptr, &ret.vk_image_view));
    _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_IMAGE_VIEW);
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
        VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &name_info);
    }

    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_image(
            self,
            self->gpu_sro_table.vk_descriptor_set,
            ret.vk_image_view,
            std::bit_cast<ImageUsageFlags>(parent_image_slot.info.usage),
            id.index);
        *out_id = std::bit_cast<daxa_ImageViewId>(id);
    }
    return result;
}

auto daxa_dvc_create_sampler(daxa_Device self, daxa_SamplerInfo const * info, daxa_SamplerId * out_id) -> daxa_Result
{
    daxa_Result result = DAXA_RESULT_SUCCESS;
    /// --- Begin Validation ---

    if (!(info->mipmap_filter != VkFilter::VK_FILTER_CUBIC_IMG))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_INVALID_SAMPLER_INFO, DAXA_RESULT_INVALID_SAMPLER_INFO);
    }

    /// --- End Validation ---
    auto slot_opt = self->gpu_sro_table.sampler_slots.try_create_slot();
    if (!slot_opt.has_value())
    {
        return DAXA_RESULT_EXCEEDED_MAX_SAMPLERS;
    }
    auto [id, ret] = slot_opt.value();
    defer
    {
        if (result != DAXA_RESULT_SUCCESS)
        {
            self->gpu_sro_table.sampler_slots.unsafe_destroy_zombie_slot(id);
            if (ret.vk_sampler)
            {
                VK_CALL_D(self, vkDestroySampler, self->vk_device, ret.vk_sampler, nullptr);
            }
        }
    };

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

    result = static_cast<daxa_Result>(VK_CALL_D(self, vkCreateSampler, self->vk_device, &vk_sampler_create_info, nullptr, &ret.vk_sampler));
    _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_SAMPLER)

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
        VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &sampler_name_info);
    }

    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_sampler(self, self->gpu_sro_table.vk_descriptor_set, ret.vk_sampler, id.index);
    }
    *out_id = std::bit_cast<daxa_SamplerId>(id);
    return result;
}

#define _DAXA_DECL_COMMON_GP_RES_FUNCTIONS(name, Name, NAME, SLOT_NAME, vk_name, VK_NAME)                      \
    auto daxa_dvc_destroy_##name(daxa_Device self, daxa_##Name##Id id)->daxa_Result                            \
    {                                                                                                          \
        _DAXA_TEST_PRINT("STRONG daxa_dvc_destroy_%s\n", #name);                                               \
        auto success = self->gpu_sro_table.SLOT_NAME.try_zombify(std::bit_cast<GPUResourceId>(id));            \
        if (success)                                                                                           \
        {                                                                                                      \
            self->zombify_##name(std::bit_cast<Name##Id>(id));                                                 \
            return DAXA_RESULT_SUCCESS;                                                                        \
        }                                                                                                      \
        return DAXA_RESULT_INVALID_##NAME##_ID;                                                                \
    }                                                                                                          \
    auto daxa_dvc_info_##name(daxa_Device self, daxa_##Name##Id id, daxa_##Name##Info * out_info)->daxa_Result \
    {                                                                                                          \
        /*NOTE: THIS CAN RACE. BUT IT IS OK AS ITS A POD AND WE CHECK IF ITS VALID AFTER THE COPY!*/           \
        auto info_copy = self->slot(id).info;                                                                  \
        if (daxa_dvc_is_##name##_valid(self, id))                                                              \
        {                                                                                                      \
            *out_info = info_copy;                                                                             \
            return DAXA_RESULT_SUCCESS;                                                                        \
        }                                                                                                      \
        return DAXA_RESULT_INVALID_##NAME##_ID;                                                                \
    }                                                                                                          \
    auto daxa_dvc_get_vk_##name(daxa_Device self, daxa_##Name##Id id, VK_NAME * out_vk_handle)->daxa_Result    \
    {                                                                                                          \
        if (daxa_dvc_is_##name##_valid(self, id))                                                              \
        {                                                                                                      \
            *out_vk_handle = self->slot(id).vk_##vk_name;                                                      \
            return DAXA_RESULT_SUCCESS;                                                                        \
        }                                                                                                      \
        return DAXA_RESULT_INVALID_##NAME##_ID;                                                                \
    }                                                                                                          \
    auto daxa_dvc_is_##name##_valid(daxa_Device self, daxa_##Name##Id id)->daxa_Bool8                          \
    {                                                                                                          \
        return std::bit_cast<daxa_Bool8>(self->gpu_sro_table.SLOT_NAME.is_id_valid(                            \
            std::bit_cast<daxa::GPUResourceId>(id)));                                                          \
    }

_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(buffer, Buffer, BUFFER, buffer_slots, buffer, VkBuffer)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(image, Image, IMAGE, image_slots, image, VkImage)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(image_view, ImageView, IMAGE_VIEW, image_slots, image_view, VkImageView)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(sampler, Sampler, SAMPLER, sampler_slots, sampler, VkSampler)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(tlas, Tlas, TLAS, tlas_slots, acceleration_structure, VkAccelerationStructureKHR)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(blas, Blas, BLAS, blas_slots, acceleration_structure, VkAccelerationStructureKHR)

DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_get_vk_image_memory(daxa_Device device, daxa_ImageId id, VkDeviceMemory * out_vk_handle)
{
    if (daxa_dvc_is_image_valid(device, id))
    {
        *out_vk_handle = device->slot(id).vk_dev_mem;
        return DAXA_RESULT_SUCCESS;
    }
    return DAXA_RESULT_INVALID_IMAGE_ID;
}

auto daxa_dvc_buffer_device_address(daxa_Device self, daxa_BufferId id, daxa_DeviceAddress * out_addr) -> daxa_Result
{
    if (!daxa_dvc_is_buffer_valid(self, id))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_INVALID_BUFFER_ID, DAXA_RESULT_INVALID_BUFFER_ID);
    }
    *out_addr = static_cast<daxa_DeviceAddress>(self->slot(std::bit_cast<BufferId>(id)).device_address);
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_buffer_host_address(daxa_Device self, daxa_BufferId id, void ** out_addr) -> daxa_Result
{
    if (!daxa_dvc_is_buffer_valid(self, id))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_INVALID_BUFFER_ID, DAXA_RESULT_INVALID_BUFFER_ID);
    }
    if (self->slot(std::bit_cast<BufferId>(id)).host_address == 0)
    {
        return DAXA_RESULT_BUFFER_NOT_HOST_VISIBLE;
    }
    *out_addr = self->slot(std::bit_cast<BufferId>(id)).host_address;
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_tlas_device_address(daxa_Device self, daxa_TlasId id, daxa_DeviceAddress * out_addr) -> daxa_Result
{
    if (!daxa_dvc_is_tlas_valid(self, id))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_INVALID_TLAS_ID, DAXA_RESULT_INVALID_TLAS_ID);
    }
    *out_addr = static_cast<daxa_DeviceAddress>(self->slot(std::bit_cast<TlasId>(id)).device_address);
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_blas_device_address(daxa_Device self, daxa_BlasId id, daxa_DeviceAddress * out_addr) -> daxa_Result
{
    if (!daxa_dvc_is_blas_valid(self, id))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_INVALID_BLAS_ID, DAXA_RESULT_INVALID_BLAS_ID);
    }
    *out_addr = static_cast<daxa_DeviceAddress>(self->slot(std::bit_cast<BlasId>(id)).device_address);
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_info(daxa_Device self) -> daxa_DeviceInfo2 const *
{
    return r_cast<daxa_DeviceInfo2 const *>(&self->info);
}

auto daxa_dvc_get_vk_device(daxa_Device self) -> VkDevice
{
    return self->vk_device;
}

auto daxa_dvc_get_vk_physical_device(daxa_Device self) -> VkPhysicalDevice
{
    return self->vk_physical_device;
}

auto daxa_dvc_wait_idle(daxa_Device self) -> daxa_Result
{
    return std::bit_cast<daxa_Result>(VK_CALL_D(self, vkDeviceWaitIdle, self->vk_device));
}

auto daxa_dvc_queue_wait_idle(daxa_Device self, daxa_Queue queue) -> daxa_Result
{
    if (!self->valid_queue(queue))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_INVALID_QUEUE, DAXA_RESULT_ERROR_INVALID_QUEUE);
    }
    if (queue.index >= self->queue_families[queue.family].queue_count)
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_INVALID_QUEUE, DAXA_RESULT_ERROR_INVALID_QUEUE);
    }
    return std::bit_cast<daxa_Result>(VK_CALL_D(self, vkQueueWaitIdle, self->get_queue(queue).vk_queue));
}

auto daxa_dvc_queue_count(daxa_Device self, daxa_QueueFamily queue_family, u32 * out_value) -> daxa_Result
{
    if (queue_family >= DAXA_QUEUE_FAMILY_MAX_ENUM)
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_INVALID_QUEUE, DAXA_RESULT_ERROR_INVALID_QUEUE);
    }
    *out_value = self->queue_families[queue_family].queue_count;
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_latest_submit_index(daxa_Device self, daxa_u64 * submit_index) -> daxa_Result
{
    *submit_index = self->global_submit_timeline.load();
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_oldest_pending_submit_index(daxa_Device self, daxa_u64 * submit_index) -> daxa_Result
{
    u64 min_pending_device_timeline_value_of_all_queues = std::numeric_limits<u64>::max();
    for (auto & queue : self->queues)
    {
        std::optional<u64> latest_pending_submit = {};
        auto result = queue.get_oldest_pending_submit(self->vk_device, latest_pending_submit);
        _DAXA_RETURN_IF_ERROR(result, result)

        if (latest_pending_submit.has_value())
        {
            min_pending_device_timeline_value_of_all_queues = std::min(min_pending_device_timeline_value_of_all_queues, latest_pending_submit.value());
        }
    }
    *submit_index = min_pending_device_timeline_value_of_all_queues;
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_submit(daxa_Device self, daxa_CommandSubmitInfo const * info) -> daxa_Result
{
    if (!self->valid_queue(info->queue))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_INVALID_QUEUE, DAXA_RESULT_ERROR_INVALID_QUEUE);
    }

    std::shared_lock lifetime_lock{self->gpu_sro_table.lifetime_lock};

    if (static_cast<u32>(info->queue.index) >= self->queue_families[info->queue.family].queue_count)
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_INVALID_QUEUE, DAXA_RESULT_ERROR_INVALID_QUEUE);
    }

    for (daxa_ExecutableCommandList commands : std::span{info->command_lists, info->command_list_count})
    {
        if (commands->cmd_recorder->info.queue_family != info->queue.family)
        {
            _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_CMD_LIST_SUBMIT_QUEUE_FAMILY_MISMATCH, DAXA_RESULT_ERROR_CMD_LIST_SUBMIT_QUEUE_FAMILY_MISMATCH);
        }
        for (BufferId id : commands->data.used_buffers)
        {
            if (!daxa_dvc_is_buffer_valid(self, id))
            {
                _DAXA_RETURN_IF_ERROR(DAXA_RESULT_COMMAND_REFERENCES_INVALID_BUFFER_ID, DAXA_RESULT_COMMAND_REFERENCES_INVALID_BUFFER_ID);
            }
        }
        for (ImageId id : commands->data.used_images)
        {
            if (!daxa_dvc_is_image_valid(self, id))
            {
                _DAXA_RETURN_IF_ERROR(DAXA_RESULT_COMMAND_REFERENCES_INVALID_IMAGE_ID, DAXA_RESULT_COMMAND_REFERENCES_INVALID_IMAGE_ID);
            }
        }
        for (ImageViewId id : commands->data.used_image_views)
        {
            if (!daxa_dvc_is_image_view_valid(self, id))
            {
                _DAXA_RETURN_IF_ERROR(DAXA_RESULT_COMMAND_REFERENCES_INVALID_IMAGE_VIEW_ID, DAXA_RESULT_COMMAND_REFERENCES_INVALID_IMAGE_VIEW_ID);
            }
        }
        for (SamplerId id : commands->data.used_samplers)
        {
            if (!daxa_dvc_is_sampler_valid(self, id))
            {
                _DAXA_RETURN_IF_ERROR(DAXA_RESULT_COMMAND_REFERENCES_INVALID_SAMPLER_ID, DAXA_RESULT_COMMAND_REFERENCES_INVALID_SAMPLER_ID);
            }
        }
    }

    daxa_ImplDevice::ImplQueue & queue = self->get_queue(info->queue);
    u64 const current_timeline_value = self->global_submit_timeline.fetch_add(1) + 1;
    queue.latest_pending_submit_timeline_value.store(current_timeline_value);

    for (auto const & commands : std::span{info->command_lists, info->command_list_count})
    {
        executable_cmd_list_execute_deferred_destructions(self, commands->data);
    }

    std::vector<VkCommandBuffer> submit_vk_command_buffers = {};
    for (auto const & commands : std::span{info->command_lists, info->command_list_count})
    {
        submit_vk_command_buffers.push_back(commands->data.vk_cmd_buffer);
    }

    std::vector<VkSemaphore> submit_semaphore_signals = {}; // All timeline semaphores come first, then binary semaphores follow.
    std::vector<u64> submit_semaphore_signal_values = {};   // Used for timeline semaphores. Ignored (push dummy value) for binary semaphores.

    // Add main queue timeline signaling as first timeline semaphore signaling:
    submit_semaphore_signals.push_back(queue.gpu_queue_local_timeline);
    submit_semaphore_signal_values.push_back(current_timeline_value);

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
    auto result = static_cast<daxa_Result>(VK_CALL_D(self, vkQueueSubmit, queue.vk_queue, 1, &vk_submit_info, VK_NULL_HANDLE));
    _DAXA_RETURN_IF_ERROR(result, result)

    std::unique_lock const lock{self->zombies_mtx};

    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_present(daxa_Device self, daxa_PresentInfo const * info) -> daxa_Result
{
    if (info->queue.family != static_cast<daxa_QueueFamily>(info->swapchain->info.queue_family))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_PRESENT_QUEUE_FAMILY_MISMATCH, DAXA_RESULT_ERROR_PRESENT_QUEUE_FAMILY_MISMATCH)
    }
    if (!self->valid_queue(info->queue))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_INVALID_QUEUE, DAXA_RESULT_ERROR_INVALID_QUEUE)
    }
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

    auto result = static_cast<daxa_Result>(VK_CALL_D(self, vkQueuePresentKHR, self->get_queue(info->queue).vk_queue, &present_info));
    _DAXA_RETURN_IF_ERROR(result, result)

    return std::bit_cast<daxa_Result>(result);
}

auto daxa_dvc_collect_garbage(daxa_Device self) -> daxa_Result
{
    std::unique_lock lifetime_lock{self->gpu_sro_table.lifetime_lock};
    std::unique_lock lock{self->zombies_mtx};

    u64 min_pending_device_timeline_value_of_all_queues = 0;
    auto result = daxa_dvc_oldest_pending_submit_index(self, &min_pending_device_timeline_value_of_all_queues);
    _DAXA_RETURN_IF_ERROR(result, result);

    auto check_and_cleanup_gpu_resources = [&](auto & zombies, auto const & cleanup_fn)
    {
        while (!zombies.empty())
        {
            auto & [timeline_value, object] = zombies.back();

            if (timeline_value >= min_pending_device_timeline_value_of_all_queues)
            {
                break;
            }

            cleanup_fn(object);
            zombies.pop_back();
        }
    };
    check_and_cleanup_gpu_resources(
        self->buffer_zombies,
        [&](auto id)
        {
            self->cleanup_buffer(id);
        });
    check_and_cleanup_gpu_resources(
        self->image_view_zombies,
        [&](auto id)
        {
            self->cleanup_image_view(id);
        });
    check_and_cleanup_gpu_resources(
        self->image_zombies,
        [&](auto id)
        {
            self->cleanup_image(id);
        });
    check_and_cleanup_gpu_resources(
        self->sampler_zombies,
        [&](auto id)
        {
            self->cleanup_sampler(id);
        });
    check_and_cleanup_gpu_resources(
        self->tlas_zombies,
        [&](auto id)
        {
            self->cleanup_tlas(id);
        });
    check_and_cleanup_gpu_resources(
        self->blas_zombies,
        [&](auto id)
        {
            self->cleanup_blas(id);
        });
    check_and_cleanup_gpu_resources(
        self->pipeline_zombies,
        [&](auto & pipeline_zombie)
        {
            VK_CALL_D(self, vkDestroyPipeline, self->vk_device, pipeline_zombie.vk_pipeline, nullptr);
        });
    check_and_cleanup_gpu_resources(
        self->semaphore_zombies,
        [&](auto & semaphore_zombie)
        {
            VK_CALL_D(self, vkDestroySemaphore, self->vk_device, semaphore_zombie.vk_semaphore, nullptr);
        });
    check_and_cleanup_gpu_resources(
        self->split_barrier_zombies,
        [&](auto & split_barrier_zombie)
        {
            VK_CALL_D(self, vkDestroyEvent, self->vk_device, split_barrier_zombie.vk_event, nullptr);
        });
    check_and_cleanup_gpu_resources(
        self->timeline_query_pool_zombies,
        [&](auto & timeline_query_pool_zombie)
        {
            VK_CALL_D(self, vkDestroyQueryPool, self->vk_device, timeline_query_pool_zombie.vk_timeline_query_pool, nullptr);
        });
    check_and_cleanup_gpu_resources(
        self->memory_block_zombies,
        [&](auto & memory_block_zombie)
        {
            vmaFreeMemory(self->vma_allocator, memory_block_zombie.allocation);
        });
    {
        std::unique_lock const main_queue_lock{self->command_pool_pools[DAXA_QUEUE_FAMILY_MAIN].mtx};
        std::unique_lock const compute_queue_lock{self->command_pool_pools[DAXA_QUEUE_FAMILY_COMPUTE].mtx};
        std::unique_lock const transfer_queue_lock{self->command_pool_pools[DAXA_QUEUE_FAMILY_TRANSFER].mtx};
        while (!self->command_list_zombies.empty())
        {
            auto & [timeline_value, zombie] = self->command_list_zombies.back();

            // Zombies are sorted. When we see a single zombie that is too young, we can dismiss the rest as they are the same age or even younger.
            if (timeline_value >= min_pending_device_timeline_value_of_all_queues)
            {
                break;
            }

            VK_CALL_D(self, vkFreeCommandBuffers, self->vk_device, zombie.vk_cmd_pool, static_cast<u32>(zombie.allocated_command_buffers.size()), zombie.allocated_command_buffers.data());
            auto result = static_cast<daxa_Result>(VK_CALL_D(self, vkResetCommandPool, self->vk_device, zombie.vk_cmd_pool, {}));
            _DAXA_RETURN_IF_ERROR(result, result)

            self->command_pool_pools[zombie.queue_family].put_back(zombie.vk_cmd_pool);
            self->command_list_zombies.pop_back();
        }
    }
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_properties(daxa_Device device) -> daxa_DeviceProperties const *
{
    return &device->properties;
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

auto daxa_ImplDevice::create_2(daxa_Instance instance, daxa_DeviceInfo2 const & info, ImplPhysicalDevice const & physical_device, daxa_DeviceProperties const & properties, daxa_Device out_device) -> daxa_Result
{
    using namespace daxa;
    daxa_Result result = {};

    // Set properties and feature variables:
    auto self = out_device;
    self->vk_physical_device = physical_device.vk_handle;
    self->properties = properties;
    self->instance = instance;

    // Verify DeviceOptions:
    if (self->info.max_allowed_buffers > self->properties.limits.max_descriptor_set_storage_buffers || self->info.max_allowed_buffers == 0)
    {
        result = DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_BUFFER_COUNT;
    }
    _DAXA_RETURN_IF_ERROR(result, result)

    auto const max_device_supported_images_in_set = std::min(self->properties.limits.max_descriptor_set_sampled_images, self->properties.limits.max_descriptor_set_storage_images);
    if (self->info.max_allowed_buffers > max_device_supported_images_in_set || self->info.max_allowed_buffers == 0)
    {
        result = DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_IMAGE_COUNT;
    }
    _DAXA_RETURN_IF_ERROR(result, result)

    if (self->info.max_allowed_samplers > self->properties.limits.max_descriptor_set_samplers || self->info.max_allowed_samplers == 0)
    {
        result = DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_SAMPLER_COUNT;
    }
    _DAXA_RETURN_IF_ERROR(result, result)

    if ((properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_BASIC_RAY_TRACING) != 0)
    {
        if (self->info.max_allowed_acceleration_structures > self->properties.acceleration_structure_properties.value.max_descriptor_set_update_after_bind_acceleration_structures ||
            self->info.max_allowed_acceleration_structures == 0)
        {
            result = DAXA_RESULT_DEVICE_DOES_NOT_SUPPORT_ACCELERATION_STRUCTURE_COUNT;
        }
        _DAXA_RETURN_IF_ERROR(result, result)
    }

#ifdef STREAMLINE_ENABLED
    // u32 extra_graphics_queue_count = 0;
    u32 extra_compute_queue_count = 0;
    if( self->instance->sl_enabled) {
        // extra_graphics_queue_count = self->instance->streamline.get_requested_graphics_queue_count();
        extra_compute_queue_count = self->instance->streamline.get_requested_compute_queue_count();
        _DAXA_RETURN_IF_ERROR(result, result)
    }
#endif // STREAMLINE_ENABLED

    // Queue Selection and Verification
    u32 vk_queue_request_count = {};
    std::array<VkDeviceQueueCreateInfo, 3> queues_ci = {};
    {
        u32 queue_family_props_count = 0;
        std::vector<VkQueueFamilyProperties> queue_props;
        VK_CALL_I(instance, vkGetPhysicalDeviceQueueFamilyProperties, self->vk_physical_device, &queue_family_props_count, nullptr);
        queue_props.resize(queue_family_props_count);
        VK_CALL_I(instance, vkGetPhysicalDeviceQueueFamilyProperties, self->vk_physical_device, &queue_family_props_count, queue_props.data());
        std::vector<VkBool32> supports_present;
        supports_present.resize(queue_family_props_count);

        // SELECT QUEUE FAMILIES
        struct QueueRequest
        {
            u32 vk_family_index;
            u32 count;
        };
        std::array<QueueRequest, 3> vk_queue_requests = {};
        for (u32 i = 0; i < queue_family_props_count; i++)
        {
            bool const supports_graphics = queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
            bool const supports_compute = queue_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
            bool const supports_transfer = queue_props[i].queueFlags & VK_QUEUE_TRANSFER_BIT;
            if (self->queue_families[DAXA_QUEUE_FAMILY_MAIN].vk_index == ~0u && supports_graphics && supports_compute && supports_transfer)
            {
                self->queue_families[DAXA_QUEUE_FAMILY_MAIN].vk_index = i;
                // NOTE: if sometime in the future graphics queues need to be increased, then check streamline requirements too
                self->queue_families[DAXA_QUEUE_FAMILY_MAIN].queue_count = 1;
                self->command_pool_pools[DAXA_QUEUE_FAMILY_MAIN].queue_family_index = i;
                self->valid_vk_queue_families[self->valid_vk_queue_family_count++] = i;
                vk_queue_requests[vk_queue_request_count++] = QueueRequest{i, 1};
            }
            if (self->queue_families[DAXA_QUEUE_FAMILY_COMPUTE].vk_index == ~0u && !supports_graphics && supports_compute && supports_transfer)
            {
                self->queue_families[DAXA_QUEUE_FAMILY_COMPUTE].vk_index = i;
                self->queue_families[DAXA_QUEUE_FAMILY_COMPUTE].queue_count = std::min(queue_props[i].queueCount, DAXA_MAX_COMPUTE_QUEUE_COUNT);
                self->command_pool_pools[DAXA_QUEUE_FAMILY_COMPUTE].queue_family_index = i;
                self->valid_vk_queue_families[self->valid_vk_queue_family_count++] = i;
                vk_queue_requests[vk_queue_request_count++] = QueueRequest{i, self->queue_families[DAXA_QUEUE_FAMILY_COMPUTE].queue_count};
            }
            if (self->queue_families[DAXA_QUEUE_FAMILY_TRANSFER].vk_index == ~0u && !supports_graphics && !supports_compute && supports_transfer)
            {
                self->queue_families[DAXA_QUEUE_FAMILY_TRANSFER].vk_index = i;
                self->queue_families[DAXA_QUEUE_FAMILY_TRANSFER].queue_count = std::min(queue_props[i].queueCount, DAXA_MAX_TRANSFER_QUEUE_COUNT);
                self->command_pool_pools[DAXA_QUEUE_FAMILY_TRANSFER].queue_family_index = i;
                self->valid_vk_queue_families[self->valid_vk_queue_family_count++] = i;
                vk_queue_requests[vk_queue_request_count++] = QueueRequest{i, self->queue_families[DAXA_QUEUE_FAMILY_TRANSFER].queue_count};
            }
        }

        if (self->queue_families[DAXA_QUEUE_FAMILY_MAIN].vk_index == ~0u)
        {
            result = DAXA_RESULT_ERROR_NO_GRAPHICS_QUEUE_FOUND;
        }
        _DAXA_RETURN_IF_ERROR(result, result)

        std::array<f32, std::max(DAXA_MAX_COMPUTE_QUEUE_COUNT, DAXA_MAX_TRANSFER_QUEUE_COUNT)> queue_priorities = { 0.0f, 0.0f, 0.0f, 0.0f };

        for (u32 family = 0; family < vk_queue_request_count; ++family)
        {
            queues_ci[family] = VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = vk_queue_requests[family].vk_family_index,
                .queueCount = vk_queue_requests[family].count,
                .pQueuePriorities = queue_priorities.data(),
            };
        };
    }

    // FIXME: check and add streamline device extensions and queue requirements

    PhysicalDeviceFeaturesStruct enabled_features = {};
    enabled_features.initialize(physical_device.extensions);
    fill_create_features(enabled_features, properties.implicit_features, info.explicit_features);
    
    // FIXME: check this out
    self->properties.nvx_binary_import = enabled_features.nvx_binary_import;
    self->properties.nvx_image_view_handle = enabled_features.nvx_image_view_handle;

    VkDeviceCreateInfo const device_ci = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = r_cast<void const *>(&enabled_features.physical_device_features_2),
        .flags = {},
        .queueCreateInfoCount = static_cast<u32>(vk_queue_request_count),
        .pQueueCreateInfos = queues_ci.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = physical_device.extensions.extension_name_list_size,
        .ppEnabledExtensionNames = physical_device.extensions.extension_name_list,
        .pEnabledFeatures = nullptr,
    };
    result = static_cast<daxa_Result>(VK_CALL_I(instance, vkCreateDevice, self->vk_physical_device, &device_ci, nullptr, &self->vk_device));
    _DAXA_RETURN_IF_ERROR(result, result)
    defer
    {
        if (result != DAXA_RESULT_SUCCESS && self->vk_device)
        {
            VK_CALL_I(instance, vkDestroyDevice, self->vk_device, nullptr);
        }
    };

    // Load device functions
    if (!self->load_device_functions())
    {
        return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;
    }

    // Queue initialization:
    defer
    {
        if (result != DAXA_RESULT_SUCCESS)
        {
            for (auto & queue : self->queues)
            {
                queue.cleanup(self);
            }
        }
    };
    for (u32 i = 0; i < self->queues.size(); ++i)
    {
        if (self->queues[i].queue_index >= self->queue_families[self->queues[i].family].queue_count)
        {
            continue;
        }
        auto const vk_queue_family = self->queue_families[self->queues[i].family].vk_index;
        self->queues[i].vk_queue_family_index = vk_queue_family;
        result = self->queues[i].initialize(self);
        _DAXA_RETURN_IF_ERROR(result, result)
    }

#ifdef STREAMLINE_ENABLED
    if( self->instance->sl_enabled) {
        self->instance->sl_enabled = self->instance->streamline.check_or_load_features();
        if(!self->instance->sl_enabled) {
            result = result = DAXA_RESULT_ERROR_INITIALIZATION_FAILED;
        }
    }
    _DAXA_RETURN_IF_ERROR(result, result)
    if(!self->instance->streamline.initialize(*r_cast<daxa::Device*>(&self))) {
        result = DAXA_RESULT_ERROR_INITIALIZATION_FAILED;
    }
    _DAXA_RETURN_IF_ERROR(result, result)
#endif // STREAMLINE_ENABLED

    VkCommandPool init_cmd_pool = {};
    VkCommandBuffer init_cmd_buffer = {};
    VkCommandPoolCreateInfo const vk_command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = self->get_queue(DAXA_QUEUE_MAIN).vk_queue_family_index,
    };

    result = static_cast<daxa_Result>(VK_CALL_D(self, vkCreateCommandPool, self->vk_device, &vk_command_pool_create_info, nullptr, &init_cmd_pool));
    _DAXA_RETURN_IF_ERROR(result, result)
    defer
    {
        // Should always be destroyed at end of function!
        if (init_cmd_pool)
        {
            VK_CALL_D(self, vkDestroyCommandPool, self->vk_device, init_cmd_pool, nullptr);
        }
    };

    VkCommandBufferAllocateInfo const vk_command_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = init_cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    result = static_cast<daxa_Result>(VK_CALL_D(self, vkAllocateCommandBuffers, self->vk_device, &vk_command_buffer_allocate_info, &init_cmd_buffer));
    _DAXA_RETURN_IF_ERROR(result, result)

    VkCommandBufferBeginInfo const vk_command_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = {},
    };
    result = static_cast<daxa_Result>(VK_CALL_D(self, vkBeginCommandBuffer, init_cmd_buffer, &vk_command_buffer_begin_info));
    _DAXA_RETURN_IF_ERROR(result, result)

    VmaVulkanFunctions const vma_vulkan_functions{
        .vkGetInstanceProcAddr = VK_GET_INSTANCE_PROC_ADDR(instance),
        .vkGetDeviceProcAddr = VK_GET_DEVICE_PROC_ADDR(instance),
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

    result = static_cast<daxa_Result>(vmaCreateAllocator(&vma_allocator_create_info, &self->vma_allocator));
    _DAXA_RETURN_IF_ERROR(result, result)

    // Bulk on-error defer for coming initializations:
    defer
    {
        if (result != DAXA_RESULT_SUCCESS)
        {
            if (self->vma_allocator)
            {
                vmaDestroyAllocator(self->vma_allocator);
            }
            if (self->vk_null_buffer)
            {
                vmaDestroyBuffer(self->vma_allocator, self->vk_null_buffer, self->vk_null_buffer_vma_allocation);
            }
            if (self->vk_null_image)
            {
                vmaDestroyImage(self->vma_allocator, self->vk_null_image, self->vk_null_image_vma_allocation);
            }
            if (self->vk_null_image_view)
            {
                VK_CALL_D(self, vkDestroyImageView, self->vk_device, self->vk_null_image_view, nullptr);
            }
            if (self->vk_null_sampler)
            {
                VK_CALL_D(self, vkDestroySampler, self->vk_device, self->vk_null_sampler, nullptr);
            }
            if (self->buffer_device_address_buffer)
            {
                vmaDestroyBuffer(self->vma_allocator, self->buffer_device_address_buffer, self->buffer_device_address_buffer_allocation);
            }
        }
    };

    // Create null resources:
    {
        auto buffer_data = std::array<u8, 4>{0xff, 0x00, 0xff, 0xff};

        VkBufferCreateInfo const null_buffer_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .size = sizeof(u8) * 4,
            .usage = create_buffer_use_flags(self),
            .sharingMode = VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = self->valid_vk_queue_family_count,
            .pQueueFamilyIndices = self->valid_vk_queue_families.data(),
        };

        VmaAllocationInfo vma_allocation_info = {};

        auto vma_allocation_flags = static_cast<VmaAllocationCreateFlags>(
            VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);

        VmaAllocationCreateInfo const null_buffer_allocation_create_info{
            .flags = vma_allocation_flags,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        result = static_cast<daxa_Result>(vmaCreateBuffer(self->vma_allocator, &null_buffer_buffer_create_info, &null_buffer_allocation_create_info, &self->vk_null_buffer, &self->vk_null_buffer_vma_allocation, &vma_allocation_info));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_NULL_BUFFER)

        if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE)
        {
            VkDebugUtilsObjectNameInfoEXT const buffer_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_BUFFER,
                .objectHandle = std::bit_cast<uint64_t>(self->vk_null_buffer),
                .pObjectName = "daxa null_buffer",
            };
            VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &buffer_name_info);
        }

        *static_cast<decltype(buffer_data) *>(vma_allocation_info.pMappedData) = buffer_data;

        auto image_info = ImageInfo{
            .dimensions = 2,
            .format = Format::R8G8B8A8_UNORM,
            .size = {1, 1, 1},
            .mip_level_count = 1,
            .array_layer_count = 1,
            .sample_count = 1,
            .usage = ImageUsageFlagBits::SHADER_SAMPLED | ImageUsageFlagBits::SHADER_STORAGE | ImageUsageFlagBits::TRANSFER_DST,
            .sharing_mode = SharingMode::CONCURRENT,
            .allocate_info = MemoryFlagBits::DEDICATED_MEMORY,
        };
        VkImageCreateInfo const vk_image_create_info = initialize_image_create_info_from_image_info(
            self, *r_cast<daxa_ImageInfo const *>(&image_info));

        VmaAllocationCreateInfo const null_img_allocation_create_info{
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        result = static_cast<daxa_Result>(vmaCreateImage(self->vma_allocator, &vk_image_create_info, &null_img_allocation_create_info, &self->vk_null_image, &self->vk_null_image_vma_allocation, nullptr));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_NULL_IMAGE)

        if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE)
        {
            VkDebugUtilsObjectNameInfoEXT const image_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_IMAGE,
                .objectHandle = std::bit_cast<uint64_t>(self->vk_null_image),
                .pObjectName = "daxa null_image",
            };
            VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &image_name_info);
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

        result = static_cast<daxa_Result>(VK_CALL_D(self, vkCreateImageView, self->vk_device, &vk_image_view_create_info, nullptr, &self->vk_null_image_view));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_NULL_IMAGE_VIEW)

        if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE)
        {
            VkDebugUtilsObjectNameInfoEXT const image_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
                .objectHandle = std::bit_cast<uint64_t>(self->vk_null_image_view),
                .pObjectName = "daxa null_image_view",
            };
            VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &image_name_info);
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
        VK_CALL_D(self, vkCmdPipelineBarrier, init_cmd_buffer, VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, {}, {}, {}, 1, &vk_image_mem_barrier);
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
        VK_CALL_D(self, vkCmdCopyBufferToImage, init_cmd_buffer, self->vk_null_buffer, self->vk_null_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &vk_buffer_image_copy);
        vk_image_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        vk_image_mem_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        vk_image_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        vk_image_mem_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL,
        vk_image_mem_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        vk_image_mem_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        VK_CALL_D(self, vkCmdPipelineBarrier, init_cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, {}, {}, {}, 1, &vk_image_mem_barrier);

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
        result = static_cast<daxa_Result>(VK_CALL_D(self, vkCreateSampler, self->vk_device, &vk_sampler_create_info, nullptr, &self->vk_null_sampler));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_NULL_SAMPLER)

        if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE)
        {
            VkDebugUtilsObjectNameInfoEXT const sampler_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_SAMPLER,
                .objectHandle = std::bit_cast<uint64_t>(self->vk_null_sampler),
                .pObjectName = "daxa null_sampler",
            };
            VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &sampler_name_info);
        }

        VkBufferUsageFlags const usage_flags =
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        VkBufferCreateInfo const bda_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .size = self->info.max_allowed_buffers * sizeof(u64),
            .usage = usage_flags,
            .sharingMode = VK_SHARING_MODE_CONCURRENT,                  // Buffers are always shared.
            .queueFamilyIndexCount = self->valid_vk_queue_family_count, // Buffers are always shared across all queues.
            .pQueueFamilyIndices = self->valid_vk_queue_families.data(),
        };

        VmaAllocationCreateInfo const bda_allocation_create_info{
            .flags = static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT),
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        result = static_cast<daxa_Result>(vmaCreateBuffer(self->vma_allocator, &bda_buffer_create_info, &bda_allocation_create_info, &self->buffer_device_address_buffer, &self->buffer_device_address_buffer_allocation, nullptr));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_BDA_BUFFER)
        result = static_cast<daxa_Result>(vmaMapMemory(self->vma_allocator, self->buffer_device_address_buffer_allocation, r_cast<void **>(&self->buffer_device_address_buffer_host_ptr)));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_BDA_BUFFER)
    }

    // Set debug names:
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
        VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &device_name_info);

        auto const buffer_name = self->info.name.c_str();
        VkDebugUtilsObjectNameInfoEXT const device_buffer_device_address_buffer_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = std::bit_cast<uint64_t>(self->buffer_device_address_buffer),
            .pObjectName = buffer_name.data(),
        };
        VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &device_buffer_device_address_buffer_name_info);

        for (auto & queue : self->queues)
        {
            if (queue.vk_queue)
            {
                std::string name = {"[DAXA DEVICE] Queue "};
                name += to_string(static_cast<QueueFamily>(queue.family));

                VkDebugUtilsObjectNameInfoEXT const queue_name_info{
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_QUEUE,
                    .objectHandle = std::bit_cast<uint64_t>(queue.vk_queue),
                    .pObjectName = name.data(),
                };
                VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &queue_name_info);

                std::string timeline_name = {"[DAXA DEVICE] Queue timeline semaphore "};
                timeline_name += to_string(static_cast<QueueFamily>(queue.family));

                VkDebugUtilsObjectNameInfoEXT const timeline_semaphore_name_info{
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_SEMAPHORE,
                    .objectHandle = std::bit_cast<uint64_t>(queue.gpu_queue_local_timeline),
                    .pObjectName = timeline_name.data(),
                };
                VK_CALL_D_EXT(self, vkSetDebugUtilsObjectNameEXT, self->vk_device, &timeline_semaphore_name_info);
            }
        }
    }

    result = self->gpu_sro_table.initialize(
        self->info.max_allowed_buffers,
        self->info.max_allowed_images,
        self->info.max_allowed_samplers,
        (properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_BASIC_RAY_TRACING) ? self->info.max_allowed_acceleration_structures : (~0u),
        self,
        self->buffer_device_address_buffer,
        self->vkSetDebugUtilsObjectNameEXT);
    _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_SUBMIT_DEVICE_INIT_COMMANDS)

    result = static_cast<daxa_Result>(VK_CALL_D(self, vkEndCommandBuffer, init_cmd_buffer));
    _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_SUBMIT_DEVICE_INIT_COMMANDS)

    // Submit initial commands to set up the daxa device.
    VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
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
    result = static_cast<daxa_Result>(VK_CALL_D(self, vkQueueSubmit, self->get_queue(DAXA_QUEUE_MAIN).vk_queue, 1, &init_submit, {}));
    _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_SUBMIT_DEVICE_INIT_COMMANDS)

    // Wait for commands in from the init cmd list to complete.
    result = static_cast<daxa_Result>(VK_CALL_D(self, vkDeviceWaitIdle, self->vk_device));
    _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_SUBMIT_DEVICE_INIT_COMMANDS)

    return DAXA_RESULT_SUCCESS;
}

auto daxa_ImplDevice::load_device_functions() -> bool 
{
#if DAXA_USE_DYNAMIC_VULKAN
    if (!vk_device || !instance->vkGetDeviceProcAddr) {
        return false;
    }

    auto vkGetDeviceProcAddr = instance->vkGetDeviceProcAddr;
    // Load device functions
    vkGetDeviceQueue = r_cast<PFN_vkGetDeviceQueue>(vkGetDeviceProcAddr(vk_device, "vkGetDeviceQueue"));
    vkCreateCommandPool = r_cast<PFN_vkCreateCommandPool>(vkGetDeviceProcAddr(vk_device, "vkCreateCommandPool"));
    vkDestroyCommandPool = r_cast<PFN_vkDestroyCommandPool>(vkGetDeviceProcAddr(vk_device, "vkDestroyCommandPool"));
    vkAllocateCommandBuffers = r_cast<PFN_vkAllocateCommandBuffers>(vkGetDeviceProcAddr(vk_device, "vkAllocateCommandBuffers"));
    vkFreeCommandBuffers = r_cast<PFN_vkFreeCommandBuffers>(vkGetDeviceProcAddr(vk_device, "vkFreeCommandBuffers"));
    vkBeginCommandBuffer = r_cast<PFN_vkBeginCommandBuffer>(vkGetDeviceProcAddr(vk_device, "vkBeginCommandBuffer"));
    vkEndCommandBuffer = r_cast<PFN_vkEndCommandBuffer>(vkGetDeviceProcAddr(vk_device, "vkEndCommandBuffer"));
    vkResetCommandBuffer = r_cast<PFN_vkResetCommandBuffer>(vkGetDeviceProcAddr(vk_device, "vkResetCommandBuffer"));
    vkCmdBeginRenderPass = r_cast<PFN_vkCmdBeginRenderPass>(vkGetDeviceProcAddr(vk_device, "vkCmdBeginRenderPass"));
    vkCmdEndRenderPass = r_cast<PFN_vkCmdEndRenderPass>(vkGetDeviceProcAddr(vk_device, "vkCmdEndRenderPass"));
    vkCmdBindPipeline = r_cast<PFN_vkCmdBindPipeline>(vkGetDeviceProcAddr(vk_device, "vkCmdBindPipeline"));
    vkCmdSetViewport = r_cast<PFN_vkCmdSetViewport>(vkGetDeviceProcAddr(vk_device, "vkCmdSetViewport"));
    vkCmdSetScissor = r_cast<PFN_vkCmdSetScissor>(vkGetDeviceProcAddr(vk_device, "vkCmdSetScissor"));
    vkCmdDraw = r_cast<PFN_vkCmdDraw>(vkGetDeviceProcAddr(vk_device, "vkCmdDraw"));
    vkCmdDrawIndexed = r_cast<PFN_vkCmdDrawIndexed>(vkGetDeviceProcAddr(vk_device, "vkCmdDrawIndexed"));
    vkCmdDrawIndirect = r_cast<PFN_vkCmdDrawIndirect>(vkGetDeviceProcAddr(vk_device, "vkCmdDrawIndirect"));
    vkCmdDrawIndexedIndirect = r_cast<PFN_vkCmdDrawIndexedIndirect>(vkGetDeviceProcAddr(vk_device, "vkCmdDrawIndexedIndirect"));
    vkCmdDrawIndirectCount = r_cast<PFN_vkCmdDrawIndirectCount>(vkGetDeviceProcAddr(vk_device, "vkCmdDrawIndirectCount"));
    vkCmdDrawIndexedIndirectCount = r_cast<PFN_vkCmdDrawIndexedIndirectCount>(vkGetDeviceProcAddr(vk_device, "vkCmdDrawIndexedIndirectCount"));
    vkCmdDispatch = r_cast<PFN_vkCmdDispatch>(vkGetDeviceProcAddr(vk_device, "vkCmdDispatch"));
    vkCmdDispatchIndirect = r_cast<PFN_vkCmdDispatchIndirect>(vkGetDeviceProcAddr(vk_device, "vkCmdDispatchIndirect"));
    vkCmdCopyBuffer = r_cast<PFN_vkCmdCopyBuffer>(vkGetDeviceProcAddr(vk_device, "vkCmdCopyBuffer"));
    vkCmdCopyImage = r_cast<PFN_vkCmdCopyImage>(vkGetDeviceProcAddr(vk_device, "vkCmdCopyImage"));
    vkCmdBlitImage = r_cast<PFN_vkCmdBlitImage>(vkGetDeviceProcAddr(vk_device, "vkCmdBlitImage"));
    vkCmdCopyBufferToImage = r_cast<PFN_vkCmdCopyBufferToImage>(vkGetDeviceProcAddr(vk_device, "vkCmdCopyBufferToImage"));
    vkCmdCopyImageToBuffer = r_cast<PFN_vkCmdCopyImageToBuffer>(vkGetDeviceProcAddr(vk_device, "vkCmdCopyImageToBuffer"));
    vkCmdUpdateBuffer = r_cast<PFN_vkCmdUpdateBuffer>(vkGetDeviceProcAddr(vk_device, "vkCmdUpdateBuffer"));
    vkCmdFillBuffer = r_cast<PFN_vkCmdFillBuffer>(vkGetDeviceProcAddr(vk_device, "vkCmdFillBuffer"));
    vkCmdClearColorImage = r_cast<PFN_vkCmdClearColorImage>(vkGetDeviceProcAddr(vk_device, "vkCmdClearColorImage"));
    vkCmdClearDepthStencilImage = r_cast<PFN_vkCmdClearDepthStencilImage>(vkGetDeviceProcAddr(vk_device, "vkCmdClearDepthStencilImage"));
    vkCmdResolveImage = r_cast<PFN_vkCmdResolveImage>(vkGetDeviceProcAddr(vk_device, "vkCmdResolveImage"));
    vkCmdSetEvent = r_cast<PFN_vkCmdSetEvent>(vkGetDeviceProcAddr(vk_device, "vkCmdSetEvent"));
    vkCmdResetEvent = r_cast<PFN_vkCmdResetEvent>(vkGetDeviceProcAddr(vk_device, "vkCmdResetEvent"));
    vkCmdWaitEvents2 = r_cast<PFN_vkCmdWaitEvents2>(vkGetDeviceProcAddr(vk_device, "vkCmdWaitEvents2"));
    vkCmdPipelineBarrier = r_cast<PFN_vkCmdPipelineBarrier>(vkGetDeviceProcAddr(vk_device, "vkCmdPipelineBarrier"));
    vkCmdPushConstants = r_cast<PFN_vkCmdPushConstants>(vkGetDeviceProcAddr(vk_device, "vkCmdPushConstants"));
    vkCmdBindDescriptorSets = r_cast<PFN_vkCmdBindDescriptorSets>(vkGetDeviceProcAddr(vk_device, "vkCmdBindDescriptorSets"));
    vkCmdBindVertexBuffers = r_cast<PFN_vkCmdBindVertexBuffers>(vkGetDeviceProcAddr(vk_device, "vkCmdBindVertexBuffers"));
    vkCmdBindIndexBuffer = r_cast<PFN_vkCmdBindIndexBuffer>(vkGetDeviceProcAddr(vk_device, "vkCmdBindIndexBuffer"));
    vkResetCommandPool = r_cast<PFN_vkResetCommandPool>(vkGetDeviceProcAddr(vk_device, "vkResetCommandPool"));
    vkCmdBeginRendering = r_cast<PFN_vkCmdBeginRendering>(vkGetDeviceProcAddr(vk_device, "vkCmdBeginRendering"));
    vkCmdEndRendering = r_cast<PFN_vkCmdEndRendering>(vkGetDeviceProcAddr(vk_device, "vkCmdEndRendering"));
    vkCmdSetDepthBias = r_cast<PFN_vkCmdSetDepthBias>(vkGetDeviceProcAddr(vk_device, "vkCmdSetDepthBias"));
    vkCmdSetEvent2 = r_cast<PFN_vkCmdSetEvent2>(vkGetDeviceProcAddr(vk_device, "vkCmdSetEvent2"));
    vkCmdWriteTimestamp2 = r_cast<PFN_vkCmdWriteTimestamp2>(vkGetDeviceProcAddr(vk_device, "vkCmdWriteTimestamp2"));
    vkCmdResetQueryPool = r_cast<PFN_vkCmdResetQueryPool>(vkGetDeviceProcAddr(vk_device, "vkCmdResetQueryPool"));

    // Memory management functions
    vkAllocateMemory = r_cast<PFN_vkAllocateMemory>(vkGetDeviceProcAddr(vk_device, "vkAllocateMemory"));
    vkFreeMemory = r_cast<PFN_vkFreeMemory>(vkGetDeviceProcAddr(vk_device, "vkFreeMemory"));
    vkMapMemory = r_cast<PFN_vkMapMemory>(vkGetDeviceProcAddr(vk_device, "vkMapMemory"));
    vkUnmapMemory = r_cast<PFN_vkUnmapMemory>(vkGetDeviceProcAddr(vk_device, "vkUnmapMemory"));
    vkFlushMappedMemoryRanges = r_cast<PFN_vkFlushMappedMemoryRanges>(vkGetDeviceProcAddr(vk_device, "vkFlushMappedMemoryRanges"));
    vkInvalidateMappedMemoryRanges = r_cast<PFN_vkInvalidateMappedMemoryRanges>(vkGetDeviceProcAddr(vk_device, "vkInvalidateMappedMemoryRanges"));
    vkBindBufferMemory = r_cast<PFN_vkBindBufferMemory>(vkGetDeviceProcAddr(vk_device, "vkBindBufferMemory"));
    vkBindImageMemory = r_cast<PFN_vkBindImageMemory>(vkGetDeviceProcAddr(vk_device, "vkBindImageMemory"));
    vkGetBufferMemoryRequirements = r_cast<PFN_vkGetBufferMemoryRequirements>(vkGetDeviceProcAddr(vk_device, "vkGetBufferMemoryRequirements"));
    vkGetImageMemoryRequirements = r_cast<PFN_vkGetImageMemoryRequirements>(vkGetDeviceProcAddr(vk_device, "vkGetImageMemoryRequirements"));
    vkGetDeviceBufferMemoryRequirements = r_cast<PFN_vkGetDeviceBufferMemoryRequirements>(vkGetDeviceProcAddr(vk_device, "vkGetDeviceBufferMemoryRequirements"));
    vkGetDeviceImageMemoryRequirements = r_cast<PFN_vkGetDeviceImageMemoryRequirements>(vkGetDeviceProcAddr(vk_device, "vkGetDeviceImageMemoryRequirements"));

    // Resource creation/destruction functions
    vkCreateBuffer = r_cast<PFN_vkCreateBuffer>(vkGetDeviceProcAddr(vk_device, "vkCreateBuffer"));
    vkDestroyBuffer = r_cast<PFN_vkDestroyBuffer>(vkGetDeviceProcAddr(vk_device, "vkDestroyBuffer"));
    vkCreateImage = r_cast<PFN_vkCreateImage>(vkGetDeviceProcAddr(vk_device, "vkCreateImage"));
    vkDestroyImage = r_cast<PFN_vkDestroyImage>(vkGetDeviceProcAddr(vk_device, "vkDestroyImage"));
    vkCreateImageView = r_cast<PFN_vkCreateImageView>(vkGetDeviceProcAddr(vk_device, "vkCreateImageView"));
    vkDestroyImageView = r_cast<PFN_vkDestroyImageView>(vkGetDeviceProcAddr(vk_device, "vkDestroyImageView"));
    vkCreateSampler = r_cast<PFN_vkCreateSampler>(vkGetDeviceProcAddr(vk_device, "vkCreateSampler"));
    vkDestroySampler = r_cast<PFN_vkDestroySampler>(vkGetDeviceProcAddr(vk_device, "vkDestroySampler"));
    vkCreateDescriptorSetLayout = r_cast<PFN_vkCreateDescriptorSetLayout>(vkGetDeviceProcAddr(vk_device, "vkCreateDescriptorSetLayout"));
    vkDestroyDescriptorSetLayout = r_cast<PFN_vkDestroyDescriptorSetLayout>(vkGetDeviceProcAddr(vk_device, "vkDestroyDescriptorSetLayout"));
    vkCreatePipelineLayout = r_cast<PFN_vkCreatePipelineLayout>(vkGetDeviceProcAddr(vk_device, "vkCreatePipelineLayout"));
    vkDestroyPipelineLayout = r_cast<PFN_vkDestroyPipelineLayout>(vkGetDeviceProcAddr(vk_device, "vkDestroyPipelineLayout"));
    vkCreateGraphicsPipelines = r_cast<PFN_vkCreateGraphicsPipelines>(vkGetDeviceProcAddr(vk_device, "vkCreateGraphicsPipelines"));
    vkCreateComputePipelines = r_cast<PFN_vkCreateComputePipelines>(vkGetDeviceProcAddr(vk_device, "vkCreateComputePipelines"));
    vkDestroyPipeline = r_cast<PFN_vkDestroyPipeline>(vkGetDeviceProcAddr(vk_device, "vkDestroyPipeline"));
    vkCreateDescriptorPool = r_cast<PFN_vkCreateDescriptorPool>(vkGetDeviceProcAddr(vk_device, "vkCreateDescriptorPool"));
    vkDestroyDescriptorPool = r_cast<PFN_vkDestroyDescriptorPool>(vkGetDeviceProcAddr(vk_device, "vkDestroyDescriptorPool"));
    vkAllocateDescriptorSets = r_cast<PFN_vkAllocateDescriptorSets>(vkGetDeviceProcAddr(vk_device, "vkAllocateDescriptorSets"));
    vkFreeDescriptorSets = r_cast<PFN_vkFreeDescriptorSets>(vkGetDeviceProcAddr(vk_device, "vkFreeDescriptorSets"));
    vkUpdateDescriptorSets = r_cast<PFN_vkUpdateDescriptorSets>(vkGetDeviceProcAddr(vk_device, "vkUpdateDescriptorSets"));
    vkResetDescriptorPool = r_cast<PFN_vkResetDescriptorPool>(vkGetDeviceProcAddr(vk_device, "vkResetDescriptorPool"));

    // Synchronization functions
    vkCreateSemaphore = r_cast<PFN_vkCreateSemaphore>(vkGetDeviceProcAddr(vk_device, "vkCreateSemaphore"));
    vkDestroySemaphore = r_cast<PFN_vkDestroySemaphore>(vkGetDeviceProcAddr(vk_device, "vkDestroySemaphore"));
    vkCreateFence = r_cast<PFN_vkCreateFence>(vkGetDeviceProcAddr(vk_device, "vkCreateFence"));
    vkDestroyFence = r_cast<PFN_vkDestroyFence>(vkGetDeviceProcAddr(vk_device, "vkDestroyFence"));
    vkWaitForFences = r_cast<PFN_vkWaitForFences>(vkGetDeviceProcAddr(vk_device, "vkWaitForFences"));
    vkResetFences = r_cast<PFN_vkResetFences>(vkGetDeviceProcAddr(vk_device, "vkResetFences"));
    vkQueueSubmit = r_cast<PFN_vkQueueSubmit>(vkGetDeviceProcAddr(vk_device, "vkQueueSubmit"));
    vkQueueWaitIdle = r_cast<PFN_vkQueueWaitIdle>(vkGetDeviceProcAddr(vk_device, "vkQueueWaitIdle"));
    vkDeviceWaitIdle = r_cast<PFN_vkDeviceWaitIdle>(vkGetDeviceProcAddr(vk_device, "vkDeviceWaitIdle"));
    vkSignalSemaphore = r_cast<PFN_vkSignalSemaphore>(vkGetDeviceProcAddr(vk_device, "vkSignalSemaphore"));
    vkWaitSemaphores = r_cast<PFN_vkWaitSemaphores>(vkGetDeviceProcAddr(vk_device, "vkWaitSemaphores"));
    vkCreateEvent = r_cast<PFN_vkCreateEvent>(vkGetDeviceProcAddr(vk_device, "vkCreateEvent"));
    vkDestroyEvent = r_cast<PFN_vkDestroyEvent>(vkGetDeviceProcAddr(vk_device, "vkDestroyEvent"));
    vkSetEvent = r_cast<PFN_vkSetEvent>(vkGetDeviceProcAddr(vk_device, "vkSetEvent"));
    vkResetEvent = r_cast<PFN_vkResetEvent>(vkGetDeviceProcAddr(vk_device, "vkResetEvent"));
    vkCmdPipelineBarrier2 = r_cast<PFN_vkCmdPipelineBarrier2>(vkGetDeviceProcAddr(vk_device, "vkCmdPipelineBarrier2"));

    // Query pool functions
    vkCreateQueryPool = r_cast<PFN_vkCreateQueryPool>(vkGetDeviceProcAddr(vk_device, "vkCreateQueryPool"));
    vkDestroyQueryPool = r_cast<PFN_vkDestroyQueryPool>(vkGetDeviceProcAddr(vk_device, "vkDestroyQueryPool"));
    vkResetQueryPool = r_cast<PFN_vkResetQueryPool>(vkGetDeviceProcAddr(vk_device, "vkResetQueryPool"));
    vkGetQueryPoolResults = r_cast<PFN_vkGetQueryPoolResults>(vkGetDeviceProcAddr(vk_device, "vkGetQueryPoolResults"));

    // Additional functions
    vkGetSemaphoreCounterValue = r_cast<PFN_vkGetSemaphoreCounterValue>(vkGetDeviceProcAddr(vk_device, "vkGetSemaphoreCounterValue"));
    vkGetBufferDeviceAddress = r_cast<PFN_vkGetBufferDeviceAddress>(vkGetDeviceProcAddr(vk_device, "vkGetBufferDeviceAddress"));
    
    // Swapchain functions
    vkCreateSwapchainKHR = r_cast<PFN_vkCreateSwapchainKHR>(vkGetDeviceProcAddr(vk_device, "vkCreateSwapchainKHR"));
    vkDestroySwapchainKHR = r_cast<PFN_vkDestroySwapchainKHR>(vkGetDeviceProcAddr(vk_device, "vkDestroySwapchainKHR"));
    vkGetSwapchainImagesKHR = r_cast<PFN_vkGetSwapchainImagesKHR>(vkGetDeviceProcAddr(vk_device, "vkGetSwapchainImagesKHR"));
    vkAcquireNextImageKHR = r_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(vk_device, "vkAcquireNextImageKHR"));
    vkQueuePresentKHR = r_cast<PFN_vkQueuePresentKHR>(vkGetDeviceProcAddr(vk_device, "vkQueuePresentKHR"));

#endif
    // Query ext function pointers
    if (properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_DYNAMIC_STATE_3)
    {
        vkCmdSetRasterizationSamplesEXT = r_cast<PFN_vkCmdSetRasterizationSamplesEXT>(vkGetDeviceProcAddr(vk_device, "vkCmdSetRasterizationSamplesEXT"));
    }

    if ((instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE)
    {
        vkSetDebugUtilsObjectNameEXT = r_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(vk_device, "vkSetDebugUtilsObjectNameEXT"));
        vkCmdBeginDebugUtilsLabelEXT = r_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(vk_device, "vkCmdBeginDebugUtilsLabelEXT"));
        vkCmdEndDebugUtilsLabelEXT = r_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(vk_device, "vkCmdEndDebugUtilsLabelEXT"));
    }

    if (properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_MESH_SHADER)
    {
        vkCmdDrawMeshTasksEXT = r_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(vk_device, "vkCmdDrawMeshTasksEXT"));
        vkCmdDrawMeshTasksIndirectEXT = r_cast<PFN_vkCmdDrawMeshTasksIndirectEXT>(vkGetDeviceProcAddr(vk_device, "vkCmdDrawMeshTasksIndirectEXT"));
        vkCmdDrawMeshTasksIndirectCountEXT = r_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(vkGetDeviceProcAddr(vk_device, "vkCmdDrawMeshTasksIndirectCountEXT"));
    }

    if (properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_BASIC_RAY_TRACING)
    {
        vkGetAccelerationStructureBuildSizesKHR = r_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(vk_device, "vkGetAccelerationStructureBuildSizesKHR"));
        vkCreateAccelerationStructureKHR = r_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(vk_device, "vkCreateAccelerationStructureKHR"));
        vkDestroyAccelerationStructureKHR = r_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(vk_device, "vkDestroyAccelerationStructureKHR"));
        vkCmdWriteAccelerationStructuresPropertiesKHR = r_cast<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>(vkGetDeviceProcAddr(vk_device, "vkCmdWriteAccelerationStructuresPropertiesKHR"));
        vkCmdBuildAccelerationStructuresKHR = r_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(vk_device, "vkCmdBuildAccelerationStructuresKHR"));
        vkGetAccelerationStructureDeviceAddressKHR = r_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(vk_device, "vkGetAccelerationStructureDeviceAddressKHR"));
    }

    if (properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_RAY_TRACING_PIPELINE)
    {
        vkCreateRayTracingPipelinesKHR = r_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(vk_device, "vkCreateRayTracingPipelinesKHR"));
        vkCmdTraceRaysKHR = r_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(vk_device, "vkCmdTraceRaysKHR"));
        vkCmdTraceRaysIndirectKHR = r_cast<PFN_vkCmdTraceRaysIndirectKHR>(vkGetDeviceProcAddr(vk_device, "vkCmdTraceRaysIndirectKHR"));
        vkGetRayTracingShaderGroupHandlesKHR = r_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(    vk_device, "vkGetRayTracingShaderGroupHandlesKHR"));
    }

    vkCmdPushDescriptorSetKHR = r_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetDeviceProcAddr(vk_device, "vkCmdPushDescriptorSetKHR"));

    if(properties.nvx_binary_import)
    {
        vkCmdCuLaunchKernelNVX = r_cast<PFN_vkCmdCuLaunchKernelNVX>(vkGetDeviceProcAddr(vk_device, "vkCmdCuLaunchKernelNVX"));
        vkCreateCuFunctionNVX = r_cast<PFN_vkCreateCuFunctionNVX>(vkGetDeviceProcAddr(vk_device, "vkCreateCuFunctionNVX"));
        vkCreateCuModuleNVX = r_cast<PFN_vkCreateCuModuleNVX>(vkGetDeviceProcAddr(vk_device, "vkCreateCuModuleNVX"));
        vkDestroyCuFunctionNVX = r_cast<PFN_vkDestroyCuFunctionNVX>(vkGetDeviceProcAddr(vk_device, "vkDestroyCuFunctionNVX"));
        vkDestroyCuModuleNVX = r_cast<PFN_vkDestroyCuModuleNVX>(vkGetDeviceProcAddr(vk_device, "vkDestroyCuModuleNVX"));
    }

    if(properties.nvx_image_view_handle) {
        vkGetImageViewHandleNVX = r_cast<PFN_vkGetImageViewHandleNVX>(vkGetDeviceProcAddr(vk_device, "vkGetImageViewHandleNVX"));
        vkGetImageViewHandle64NVX = r_cast<PFN_vkGetImageViewHandle64NVX>(vkGetDeviceProcAddr(vk_device, "vkGetImageViewHandle64NVX"));
        vkGetImageViewAddressNVX = r_cast<PFN_vkGetImageViewAddressNVX>(vkGetDeviceProcAddr(vk_device, "vkGetImageViewAddressNVX"));
    }

    return true;
}

auto daxa_ImplDevice::get_queue(daxa_Queue queue) -> daxa_ImplDevice::ImplQueue &
{
    u32 offsets[3] = {
        0,
        1,
        1 + DAXA_MAX_COMPUTE_QUEUE_COUNT,
    };
    return this->queues[offsets[queue.family] + queue.index];
}

auto daxa_ImplDevice::valid_queue(daxa_Queue queue) -> bool
{
    return queue.family < DAXA_QUEUE_FAMILY_MAX_ENUM && queue.index < this->queue_families[queue.family].queue_count;
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

auto daxa_ImplDevice::new_swapchain_image(VkImage swapchain_image, VkFormat format, u32 index, ImageUsageFlags usage, ImageInfo const & image_info, ImageId * out) -> daxa_Result
{
    daxa_Result result = DAXA_RESULT_SUCCESS;

    auto slot_opt = this->gpu_sro_table.image_slots.try_create_slot();
    DAXA_DBG_ASSERT_TRUE_M(slot_opt.has_value(), "CRITICAL INTERNAL ERROR, EXCEEDED MAX IMAGES IN SWAPCHAIN CREATION");
    auto [id, ret] = slot_opt.value();
    defer
    {
        if (result != DAXA_RESULT_SUCCESS)
        {
            gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(id);
        }
    };

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
    result = static_cast<daxa_Result>(VK_CALL_D(this, vkCreateImageView, vk_device, &view_ci, nullptr, &ret.view_slot.vk_image_view));
    _DAXA_RETURN_IF_ERROR(result, result)

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
        VK_CALL_D_EXT(this, vkSetDebugUtilsObjectNameEXT, this->vk_device, &swapchain_image_name_info);

        VkDebugUtilsObjectNameInfoEXT const swapchain_image_view_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
            .objectHandle = std::bit_cast<uint64_t>(ret.view_slot.vk_image_view),
            .pObjectName = c_str_arr.data(),
        };
        VK_CALL_D_EXT(this, vkSetDebugUtilsObjectNameEXT, this->vk_device, &swapchain_image_view_name_info);
    }

    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_image(this, this->gpu_sro_table.vk_descriptor_set, ret.view_slot.vk_image_view, usage, id.index);
    }

    *out = ImageId{id};

    return result;
}

void daxa_ImplDevice::cleanup_buffer(BufferId id)
{
    auto gid = std::bit_cast<GPUResourceId>(id);
    ImplBufferSlot const & buffer_slot = this->gpu_sro_table.buffer_slots.unsafe_get(gid);
    this->buffer_device_address_buffer_host_ptr[gid.index] = 0;
    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_buffer(this, this->gpu_sro_table.vk_descriptor_set, this->vk_null_buffer, 0, VK_WHOLE_SIZE, gid.index);
    }
    if (buffer_slot.opt_memory_block != nullptr)
    {
        VK_CALL_D(this, vkDestroyBuffer, this->vk_device, buffer_slot.vk_buffer, {});
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
    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_image(
            this,
            this->gpu_sro_table.vk_descriptor_set,
            this->vk_null_image_view,
            std::bit_cast<ImageUsageFlags>(image_slot.info.usage),
            gid.index);
    }
    VK_CALL_D(this, vkDestroyImageView, vk_device, image_slot.view_slot.vk_image_view, nullptr);
    if (image_slot.swapchain_image_index == NOT_OWNED_BY_SWAPCHAIN)
    {
        if (image_slot.opt_memory_block != nullptr)
        {
            VK_CALL_D(this, vkDestroyImage, this->vk_device, image_slot.vk_image, {});
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
    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_image(this, this->gpu_sro_table.vk_descriptor_set, this->vk_null_image_view, ImageUsageFlagBits::SHADER_STORAGE | ImageUsageFlagBits::SHADER_SAMPLED, std::bit_cast<daxa::ImageViewId>(id).index);
    }
    VK_CALL_D(this, vkDestroyImageView, vk_device, image_slot.vk_image_view, nullptr);
    gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_sampler(SamplerId id)
{
    ImplSamplerSlot const & sampler_slot = this->gpu_sro_table.sampler_slots.unsafe_get(std::bit_cast<GPUResourceId>(id));
    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_sampler(this, this->gpu_sro_table.vk_descriptor_set, this->vk_null_sampler, std::bit_cast<GPUResourceId>(id).index);
    }
    VK_CALL_D(this, vkDestroySampler, this->vk_device, sampler_slot.vk_sampler, nullptr);
    gpu_sro_table.sampler_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_tlas(TlasId id)
{
    ImplTlasSlot const & tlas_slot = this->gpu_sro_table.tlas_slots.unsafe_get(std::bit_cast<GPUResourceId>(id));
    // TODO(Raytracing): Add null acceleration structure:
    // write_descriptor_set_acceleration_structure(this->vk_device, this->gpu_sro_table.vk_descriptor_set, this->vk_null_acceleration_structure, std::bit_cast<GPUResourceId>(id).index);
    VK_CALL_D_EXT(this, vkDestroyAccelerationStructureKHR, this->vk_device, tlas_slot.vk_acceleration_structure, nullptr);
    gpu_sro_table.tlas_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_blas(BlasId id)
{
    ImplBlasSlot const & blas_slot = this->gpu_sro_table.blas_slots.unsafe_get(std::bit_cast<GPUResourceId>(id));
    VK_CALL_D_EXT(this, vkDestroyAccelerationStructureKHR, this->vk_device, blas_slot.vk_acceleration_structure, nullptr);
    gpu_sro_table.blas_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
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

auto daxa_ImplDevice::slot(daxa_TlasId id) const -> ImplTlasSlot const &
{
    return gpu_sro_table.tlas_slots.unsafe_get(std::bit_cast<daxa::GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_BlasId id) const -> ImplBlasSlot const &
{
    return gpu_sro_table.blas_slots.unsafe_get(std::bit_cast<daxa::GPUResourceId>(id));
}

void daxa_ImplDevice::zero_ref_callback(ImplHandle const * handle)
{
    _DAXA_TEST_PRINT("daxa_ImplDevice::zero_ref_callback\n");
    auto self = rc_cast<daxa_Device>(handle);
    auto result = daxa_dvc_wait_idle(self);
    DAXA_DBG_ASSERT_TRUE_M(result == DAXA_RESULT_SUCCESS, "failed to wait idle");
    result = daxa_dvc_collect_garbage(self);
    DAXA_DBG_ASSERT_TRUE_M(result == DAXA_RESULT_SUCCESS, "failed to wait idle");
    for (auto & pool_pool : self->command_pool_pools)
    {
        pool_pool.cleanup(self);
    }
    vmaUnmapMemory(self->vma_allocator, self->buffer_device_address_buffer_allocation);
    vmaDestroyBuffer(self->vma_allocator, self->buffer_device_address_buffer, self->buffer_device_address_buffer_allocation);
    self->gpu_sro_table.cleanup(self);
    vmaDestroyImage(self->vma_allocator, self->vk_null_image, self->vk_null_image_vma_allocation);
    vmaDestroyBuffer(self->vma_allocator, self->vk_null_buffer, self->vk_null_buffer_vma_allocation);
    vmaDestroyAllocator(self->vma_allocator);
    VK_CALL_D(self, vkDestroySampler, self->vk_device, self->vk_null_sampler, nullptr);
    VK_CALL_D(self, vkDestroyImageView, self->vk_device, self->vk_null_image_view, nullptr);
    for (auto & queue : self->queues)
    {
        queue.cleanup(self);
    }
    VK_CALL_I(self->instance, vkDestroyDevice, self->vk_device, nullptr);
    self->instance->dec_weak_refcnt(
        daxa_ImplInstance::zero_ref_callback,
        self->instance);
    delete self;
}

template <typename T>
void zombiefy(daxa_Device self, T id, auto & slots, auto & zombies)
{
    [[maybe_unused]] auto & slot = slots.unsafe_get(std::bit_cast<GPUResourceId>(id));
    if constexpr (std::is_same_v<T, BufferId> || std::is_same_v<T, ImageId>)
    {
        if (slot.opt_memory_block != nullptr)
        {
            slot.opt_memory_block->dec_weak_refcnt(
                daxa_ImplMemoryBlock::zero_ref_callback,
                self->instance);
        }
    }
    if constexpr (std::is_same_v<T, TlasId> || std::is_same_v<T, BlasId>)
    {
        if (slot.owns_buffer)
        {
            self->zombify_buffer(slot.buffer_id);
        }
    }
    u64 const submit_timeline_value = self->global_submit_timeline.load(std::memory_order::relaxed);
    {
        std::unique_lock const lock{self->zombies_mtx};
        zombies.push_front(std::pair{submit_timeline_value, id});
    }
}

void daxa_ImplDevice::zombify_buffer(BufferId id)
{
    _DAXA_TEST_PRINT("daxa_ImplDevice::zombify_buffer\n");
    zombiefy(this, id, gpu_sro_table.buffer_slots, this->buffer_zombies);
}

void daxa_ImplDevice::zombify_image(ImageId id)
{
    _DAXA_TEST_PRINT("daxa_ImplDevice::zombify_image (%i,%i)\n", id.index, id.version);
    zombiefy(this, id, gpu_sro_table.image_slots, this->image_zombies);
}

void daxa_ImplDevice::zombify_image_view(ImageViewId id)
{
    _DAXA_TEST_PRINT("daxa_ImplDevice::zombify_image_view\n");
    zombiefy(this, id, gpu_sro_table.image_slots, this->image_view_zombies);
}

void daxa_ImplDevice::zombify_sampler(SamplerId id)
{
    _DAXA_TEST_PRINT("daxa_ImplDevice::zombify_sampler\n");
    zombiefy(this, id, gpu_sro_table.sampler_slots, this->sampler_zombies);
}

void daxa_ImplDevice::zombify_tlas(TlasId id)
{
    _DAXA_TEST_PRINT("daxa_ImplDevice::zombify_tlas\n");
    zombiefy(this, id, gpu_sro_table.tlas_slots, this->tlas_zombies);
}

void daxa_ImplDevice::zombify_blas(BlasId id)
{
    _DAXA_TEST_PRINT("daxa_ImplDevice::zombify_blas\n");
    zombiefy(this, id, gpu_sro_table.blas_slots, this->blas_zombies);
}

// --- End Internal Functions ---