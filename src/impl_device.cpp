#include "impl_core.hpp"

#include "impl_device.hpp"

#include <unordered_map>
#include <utility>
#include "daxa/core.hpp"
#include "impl_features.hpp"
#include "impl_swapchain.hpp"
#include "impl_instance.hpp"

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
            .pQueueFamilyIndices = &self->get_queue(DAXA_QUEUE_MAIN).vk_queue_type_index,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        if (image_info.sharing_mode == daxa_SharingMode::DAXA_SHARING_MODE_CONCURRENT)
        {
            vk_image_create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            vk_image_create_info.queueFamilyIndexCount = self->valid_vk_queue_type_count;
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

auto daxa_ImplDevice::ImplQueue::initialize(VkDevice a_vk_device) -> daxa_Result
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

    vkGetDeviceQueue(a_vk_device, vk_queue_type_index, queue_index, &this->vk_queue);
    daxa_Result result = DAXA_RESULT_SUCCESS;
    if (this->vk_queue == VK_NULL_HANDLE)
    {
        result = DAXA_RESULT_ERROR_COULD_NOT_QUERY_QUEUE;
    }
    _DAXA_RETURN_IF_ERROR(result, result)

    result = static_cast<daxa_Result>(vkCreateSemaphore(a_vk_device, &vk_semaphore_create_info, nullptr, &this->gpu_queue_local_timeline));
    _DAXA_RETURN_IF_ERROR(result, result)

    return result;
}

void daxa_ImplDevice::ImplQueue::cleanup(VkDevice device)
{
    if (this->gpu_queue_local_timeline)
    {
        vkDestroySemaphore(device, this->gpu_queue_local_timeline, nullptr);
    }
}

auto daxa_ImplDevice::ImplQueue::get_oldest_pending_submit(VkDevice a_vk_device, std::optional<u64> & out) -> daxa_Result
{
    if (this->gpu_queue_local_timeline)
    {
        u64 latest_gpu = {};
        auto result = static_cast<daxa_Result>(vkGetSemaphoreCounterValue(a_vk_device, this->gpu_queue_local_timeline, &latest_gpu));
        _DAXA_RETURN_IF_ERROR(result, result);

        // WORKAROUND
        // AMD RDNA4 drivers sometimes return 0xFF.. instead of returning device lost.
        if (latest_gpu == 0xFFFFFFFFFFFFFFFF)
        {
            _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_DEVICE_LOST, DAXA_RESULT_ERROR_DEVICE_LOST);
        }

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
    auto [id, ret, hot_data] = slot_opt.value();

    defer
    {
        if (result != DAXA_RESULT_SUCCESS)
        {
            self->gpu_sro_table.buffer_slots.unsafe_destroy_zombie_slot(id);
            if (hot_data.vk_buffer)
            {
                vkDestroyBuffer(self->vk_device, hot_data.vk_buffer, nullptr);
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
        .queueFamilyIndexCount = self->valid_vk_queue_type_count, // Buffers are always shared across all queues.
        .pQueueFamilyIndices = self->valid_vk_queue_families.data(),
    };

    bool host_accessible = false;
    daxa_MemoryFlags vma_allocation_flags = opt_memory_block != nullptr ? opt_memory_block->info.flags : info->memory_flags;
    if (((vma_allocation_flags & DAXA_MEMORY_FLAG_HOST_ACCESS_RANDOM) != 0u) ||
        ((vma_allocation_flags & DAXA_MEMORY_FLAG_HOST_ACCESS_SEQUENTIAL_WRITE) != 0u))
    {
        vma_allocation_flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
        host_accessible = true;
    }

    VmaAllocationInfo vma_allocation_info = {};
    if (opt_memory_block == nullptr)
    {

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
            &hot_data.vk_buffer,
            &ret.vma_allocation,
            &vma_allocation_info));
        _DAXA_RETURN_IF_ERROR(result, result)

        if (host_accessible)
        {
            hot_data.host_address = vma_allocation_info.pMappedData;
        }
    }
    else
    {
        auto const & mem_block = *opt_memory_block;

        bool const invalidMemoryFlags = info->memory_flags != DAXA_MEMORY_FLAG_NONE;
        if (invalidMemoryFlags)
        {
            _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_ALLOC_FLAGS_MUST_BE_ZERO_ON_BLOCK_ALLOCATION, DAXA_RESULT_ERROR_ALLOC_FLAGS_MUST_BE_ZERO_ON_BLOCK_ALLOCATION);
        }

        // copy flags from memory block to buffer info.
        ret.info.memory_flags = opt_memory_block->info.flags;

        ret.opt_memory_block = opt_memory_block;
        result = static_cast<daxa_Result>(vkCreateBuffer(self->vk_device, &vk_buffer_create_info, nullptr, &hot_data.vk_buffer));
        _DAXA_RETURN_IF_ERROR(result, result)

        result = static_cast<daxa_Result>(vmaBindBufferMemory2(
            self->vma_allocator,
            mem_block.allocation,
            opt_offset,
            hot_data.vk_buffer,
            {}));
        if (result != DAXA_RESULT_SUCCESS)
        {
            vkDestroyBuffer(self->vk_device, hot_data.vk_buffer, nullptr);
        }
        _DAXA_RETURN_IF_ERROR(result, result)

        opt_memory_block->inc_weak_refcnt();
        if (host_accessible)
        {
            hot_data.host_address = static_cast<void *>(static_cast<u8 *>(opt_memory_block->alloc_info.pMappedData) + opt_offset);
        }
    }

    VkBufferDeviceAddressInfo const vk_buffer_device_address_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = hot_data.vk_buffer,
    };

    hot_data.device_address = vkGetBufferDeviceAddress(self->vk_device, &vk_buffer_device_address_info);

    self->buffer_device_address_buffer_host_ptr[id.index] = hot_data.device_address;

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE &&
        info->name.size != 0)
    {
        VkDebugUtilsObjectNameInfoEXT const buffer_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = std::bit_cast<uint64_t>(hot_data.vk_buffer),
            .pObjectName = info->name.data,
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &buffer_name_info);
    }

    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_buffer(
            self->vk_device,
            self->gpu_sro_table.vk_descriptor_set, hot_data.vk_buffer,
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
    auto [id, ret, hot_data] = slot_opt.value();

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
                vkDestroyImageView(self->vk_device, ret.view_slot.vk_image_view, nullptr);
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
        .image = {}, // FILL THIS LATER!
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
            .flags = static_cast<VmaAllocationCreateFlags>(info->memory_flags),
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        result = static_cast<daxa_Result>(vmaCreateImage(self->vma_allocator, &vk_image_create_info, &vma_allocation_create_info, &ret.vk_image, &ret.vma_allocation, nullptr));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_IMAGE);

        vk_image_view_create_info.image = ret.vk_image;
        result = static_cast<daxa_Result>(vkCreateImageView(self->vk_device, &vk_image_view_create_info, nullptr, &ret.view_slot.vk_image_view));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_DEFAULT_IMAGE_VIEW);
    }
    else
    {
        daxa_ImplMemoryBlock const & mem_block = *opt_memory_block;

        bool const invalidMemoryFlags = info->memory_flags != DAXA_MEMORY_FLAG_NONE;
        if (invalidMemoryFlags)
        {
            _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_ALLOC_FLAGS_MUST_BE_ZERO_ON_BLOCK_ALLOCATION, DAXA_RESULT_ERROR_ALLOC_FLAGS_MUST_BE_ZERO_ON_BLOCK_ALLOCATION);
        }

        // copy flags from memory block to image info.
        ret.info.memory_flags = opt_memory_block->info.flags;

        ret.opt_memory_block = opt_memory_block;
        result = static_cast<daxa_Result>(vkCreateImage(self->vk_device, &vk_image_create_info, nullptr, &ret.vk_image));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_IMAGE);

        result = static_cast<daxa_Result>(vmaBindImageMemory2(
            self->vma_allocator,
            mem_block.allocation,
            opt_offset,
            ret.vk_image,
            {}));
        if (result != DAXA_RESULT_SUCCESS)
        {
            vkDestroyImage(self->vk_device, ret.vk_image, nullptr);
        }
        _DAXA_RETURN_IF_ERROR(result, result);

        vk_image_view_create_info.image = ret.vk_image;
        result = static_cast<daxa_Result>(vkCreateImageView(self->vk_device, &vk_image_view_create_info, nullptr, &ret.view_slot.vk_image_view));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_DEFAULT_IMAGE_VIEW);

        opt_memory_block->inc_weak_refcnt();
    }

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && info->name.size != 0)
    {
        VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE,
            .objectHandle = std::bit_cast<uint64_t>(ret.vk_image),
            .pObjectName = info->name.data,
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &swapchain_image_name_info);

        VkDebugUtilsObjectNameInfoEXT const swapchain_image_view_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
            .objectHandle = std::bit_cast<uint64_t>(ret.view_slot.vk_image_view),
            .pObjectName = info->name.data,
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &swapchain_image_view_name_info);
    }

    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_image(
            self->vk_device,
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
    auto * out_id,
    bool owns_buffer_external = false) -> daxa_Result
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

    auto [id, ret, hot_data] = slot_opt.value();
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
        ret.owns_buffer = owns_buffer_external;
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
            .memory_flags = {},
            .name = std::bit_cast<daxa_SmallString>(buffer_name),
        };
        result = daxa_dvc_create_buffer(self, &cinfo, r_cast<daxa_BufferId *>(&ret.buffer_id));
        _DAXA_RETURN_IF_ERROR(result, result);
        ret.offset = 0;
        ret.owns_buffer = true;
    }
    ret.vk_buffer = self->hot_slot(ret.buffer_id).vk_buffer;

    VkAccelerationStructureCreateInfoKHR vk_create_info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .createFlags = {}, // VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR,
        .buffer = self->hot_slot(ret.buffer_id).vk_buffer,
        .offset = ret.offset,
        .size = ret.info.size,
        .type = vk_as_type,
        .deviceAddress = {},
    };
    result = static_cast<daxa_Result>(self->vkCreateAccelerationStructureKHR(self->vk_device, &vk_create_info, nullptr, &hot_data.vk_acceleration_structure));
    _DAXA_RETURN_IF_ERROR(result, result);

    auto vk_acceleration_structure_device_address_info_khr = VkAccelerationStructureDeviceAddressInfoKHR{
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
        .pNext = nullptr,
        .accelerationStructure = hot_data.vk_acceleration_structure,
    };
    hot_data.device_address = self->vkGetAccelerationStructureDeviceAddressKHR(
        self->vk_device,
        &vk_acceleration_structure_device_address_info_khr);

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && ret.info.name.size != 0)
    {
        VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR,
            .objectHandle = std::bit_cast<uint64_t>(hot_data.vk_acceleration_structure),
            .pObjectName = ret.info.name.data,
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &swapchain_image_name_info);
    }

    if (vk_as_type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR)
    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_acceleration_structure(
            self->vk_device,
            self->gpu_sro_table.vk_descriptor_set,
            hot_data.vk_acceleration_structure,
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
        u64 version_refcnt = self->gpu_sro_table.buffer_slots.version_refcnt_of_slot(bi);
        if (GpuResourcePool<>::get_refcnt(version_refcnt) > 0)
        {
            daxa::BufferId id = {bi, GpuResourcePool<>::get_version(version_refcnt)};
            auto & slot = self->gpu_sro_table.buffer_slots.unsafe_get(id);
            auto & hot_slot = self->gpu_sro_table.buffer_slots.unsafe_get_hot(id);
            if (hot_slot.vk_buffer == nullptr)
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
        u64 version_refcnt = self->gpu_sro_table.image_slots.version_refcnt_of_slot(ii);
        if (GpuResourcePool<>::get_refcnt(version_refcnt) > 0)
        {
            daxa::ImageId id = {ii, GpuResourcePool<>::get_version(version_refcnt)};
            auto & slot = self->gpu_sro_table.image_slots.unsafe_get(id);
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
        u64 version_refcnt = self->gpu_sro_table.tlas_slots.version_refcnt_of_slot(ti);
        if (GpuResourcePool<>::get_refcnt(version_refcnt) > 0)
        {
            daxa::TlasId id = {ti, GpuResourcePool<>::get_version(version_refcnt)};
            auto & slot = self->gpu_sro_table.tlas_slots.unsafe_get(id);
            auto & hot_data = self->gpu_sro_table.tlas_slots.unsafe_get_hot(id);
            if (hot_data.vk_acceleration_structure == nullptr)
            {
                continue;
            }

            u32 out_idx = report->tlas_count++;
            report->total_aliased_tlas_device_memory_use += slot.info.size;

            if (report->tlas_list != nullptr && out_idx < tlas_list_allocation_size)
            {
                report->tlas_list[out_idx] = {
                    std::bit_cast<daxa_TlasId>(id),
                    slot.info.size};
            }
        }
    }

    for (u32 bli = 0; bli < self->gpu_sro_table.blas_slots.next_index; ++bli)
    {
        u64 version_refcnt = self->gpu_sro_table.blas_slots.version_refcnt_of_slot(bli);
        if (GpuResourcePool<>::get_refcnt(version_refcnt) > 0)
        {
            daxa::BlasId id = {bli, GpuResourcePool<>::get_version(version_refcnt)};
            auto & slot = self->gpu_sro_table.blas_slots.unsafe_get(id);
            auto & hot_slot = self->gpu_sro_table.blas_slots.unsafe_get_hot(id);
            if (hot_slot.vk_acceleration_structure == nullptr)
            {
                continue;
            }

            u32 out_idx = report->blas_count++;
            report->total_aliased_blas_device_memory_use += slot.info.size;

            if (report->blas_list != nullptr && out_idx < blas_list_allocation_size)
            {
                report->blas_list[out_idx] = {
                    std::bit_cast<daxa_BlasId>(id),
                    slot.info.size};
            }
        }
    }

    for (auto v : mem_blocks)
    {
        daxa_MemoryBlock const & block = v.first;
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
        .queueFamilyIndexCount = self->valid_vk_queue_type_count, // Buffers are always shared across all queues.
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
    vkGetDeviceBufferMemoryRequirements(self->vk_device, &buffer_requirement_info, &mem_requirements);
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
    vkGetDeviceImageMemoryRequirements(self->vk_device, &image_requirement_info, &mem_requirements);
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
        .accelerationStructureSize = {},
        .updateScratchSize = {},
        .buildScratchSize = {},
    };
    self->vkGetAccelerationStructureBuildSizesKHR(
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
        .accelerationStructureSize = {},
        .updateScratchSize = {},
        .buildScratchSize = {},
    };
    self->vkGetAccelerationStructureBuildSizesKHR(
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

auto daxa_dvc_create_tlas_from_memory_block(daxa_Device self, daxa_MemoryBlockTlasInfo const * info, daxa_TlasId * out_id) -> daxa_Result
{
    auto buffer_info = daxa_BufferInfo{
        .size = info->tlas_info.size,
        .name = info->tlas_info.name,
    };
    daxa_BufferId buffer = {};
    daxa_Result result = create_buffer_helper(self, &buffer_info, &buffer, *info->memory_block, info->offset);
    _DAXA_RETURN_IF_ERROR(result, result);

    auto buffer_tlas_info = daxa_BufferTlasInfo{
        .tlas_info = info->tlas_info,
        .buffer = buffer,
        .offset = {},
    };
    u64 const buffer_offset = 0u; // the offset is used for sub allocating the memory block, the tlas is the whole buffer
    return create_acceleration_structure_helper(
        self,
        self->gpu_sro_table.tlas_slots,
        VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        info->tlas_info,
        &buffer,
        &buffer_offset,
        out_id,
        true);
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
        &info->buffer,
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
        &info->buffer,
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
    auto [id, image_slot, hot_data] = slot_opt.value();
    defer
    {
        if (result != DAXA_RESULT_SUCCESS)
        {
            self->gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(id);
            if (image_slot.view_slot.vk_image_view)
            {
                vkDestroyImageView(self->vk_device, image_slot.view_slot.vk_image_view, nullptr);
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
    _DAXA_RETURN_IF_ERROR(result, result);

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
    result = static_cast<daxa_Result>(vkCreateImageView(self->vk_device, &vk_image_view_create_info, nullptr, &ret.vk_image_view));
    _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_IMAGE_VIEW);
    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && info->name.size != 0)
    {
        VkDebugUtilsObjectNameInfoEXT const name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
            .objectHandle = std::bit_cast<uint64_t>(ret.vk_image_view),
            .pObjectName = info->name.data,
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &name_info);
    }

    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_image(
            self->vk_device,
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
    auto [id, ret, hot_data] = slot_opt.value();
    defer
    {
        if (result != DAXA_RESULT_SUCCESS)
        {
            self->gpu_sro_table.sampler_slots.unsafe_destroy_zombie_slot(id);
            if (ret.vk_sampler)
            {
                vkDestroySampler(self->vk_device, ret.vk_sampler, nullptr);
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

    result = static_cast<daxa_Result>(vkCreateSampler(self->vk_device, &vk_sampler_create_info, nullptr, &ret.vk_sampler));
    _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_SAMPLER)

    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && info->name.size != 0)
    {
        VkDebugUtilsObjectNameInfoEXT const sampler_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_SAMPLER,
            .objectHandle = std::bit_cast<uint64_t>(ret.vk_sampler),
            .pObjectName = info->name.data,
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &sampler_name_info);
    }

    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_sampler(self->vk_device, self->gpu_sro_table.vk_descriptor_set, ret.vk_sampler, id.index);
    }
    *out_id = std::bit_cast<daxa_SamplerId>(id);
    return result;
}

template <typename T>
auto template_hot_or_slot(auto & self, auto id)
{
    if constexpr (T::HAS_HOT_DATA)
    {
        return self->hot_slot(id);
    }
    else
    {
        return self->slot(id);
    }
}

#define _DAXA_DECL_COMMON_GP_RES_FUNCTIONS(name, Name, NAME, SLOT_NAME, vk_name, VK_NAME)                                \
    auto daxa_dvc_inc_refcnt_##name(daxa_Device self, daxa_##Name##Id id) -> daxa_Result                                 \
    {                                                                                                                    \
        auto success = self->gpu_sro_table.SLOT_NAME.try_inc_refcnt(std::bit_cast<GPUResourceId>(id));                   \
        if (success)                                                                                                     \
        {                                                                                                                \
            return DAXA_RESULT_SUCCESS;                                                                                  \
        }                                                                                                                \
        daxa_Result result = DAXA_RESULT_INVALID_##NAME##_ID;                                                            \
        _DAXA_RETURN_IF_ERROR(result, result);                                                                           \
        return result;                                                                                                   \
    }                                                                                                                    \
    auto daxa_dvc_destroy_##name(daxa_Device self, daxa_##Name##Id id) -> daxa_Result                                    \
    {                                                                                                                    \
        auto ret = self->gpu_sro_table.SLOT_NAME.try_dec_refcnt(std::bit_cast<GPUResourceId>(id));                       \
        if (ret != TryDecRefcntResult::ERROR_INVALID_ID)                                                                 \
        {                                                                                                                \
            if (ret == TryDecRefcntResult::SUCCESS_REFCOUNT_ZERO)                                                        \
            {                                                                                                            \
                self->zombify_##name(std::bit_cast<Name##Id>(id));                                                       \
            }                                                                                                            \
            return DAXA_RESULT_SUCCESS;                                                                                  \
        }                                                                                                                \
        daxa_Result result = DAXA_RESULT_INVALID_##NAME##_ID;                                                            \
        _DAXA_RETURN_IF_ERROR(result, result);                                                                           \
        return result;                                                                                                   \
    }                                                                                                                    \
    auto daxa_dvc_info_##name(daxa_Device self, daxa_##Name##Id id, daxa_##Name##Info * out_info) -> daxa_Result         \
    {                                                                                                                    \
        /*NOTE: THIS CAN RACE. BUT IT IS OK AS ITS A POD AND WE CHECK IF ITS VALID AFTER THE COPY!*/                     \
        auto info_copy = self->slot(id).info;                                                                            \
        if (daxa_dvc_is_##name##_valid(self, id))                                                                        \
        {                                                                                                                \
            *out_info = info_copy;                                                                                       \
            return DAXA_RESULT_SUCCESS;                                                                                  \
        }                                                                                                                \
        return DAXA_RESULT_INVALID_##NAME##_ID;                                                                          \
    }                                                                                                                    \
    auto daxa_dvc_get_vk_##name(daxa_Device self, daxa_##Name##Id id, VK_NAME * out_vk_handle) -> daxa_Result            \
    {                                                                                                                    \
        if (daxa_dvc_is_##name##_valid(self, id))                                                                        \
        {                                                                                                                \
            *out_vk_handle = template_hot_or_slot<std::remove_cvref_t<decltype(self->slot(id))>>(self, id).vk_##vk_name; \
            return DAXA_RESULT_SUCCESS;                                                                                  \
        }                                                                                                                \
        daxa_Result result = DAXA_RESULT_INVALID_##NAME##_ID;                                                            \
        _DAXA_RETURN_IF_ERROR(result, result);                                                                           \
        return result;                                                                                                   \
    }                                                                                                                    \
    auto daxa_dvc_is_##name##_valid(daxa_Device self, daxa_##Name##Id id) -> daxa_Bool8                                  \
    {                                                                                                                    \
        return std::bit_cast<daxa_Bool8>(self->gpu_sro_table.SLOT_NAME.is_id_valid(                                      \
            std::bit_cast<daxa::GPUResourceId>(id)));                                                                    \
    }

_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(buffer, Buffer, BUFFER, buffer_slots, buffer, VkBuffer)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(image, Image, IMAGE, image_slots, image, VkImage)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(image_view, ImageView, IMAGE_VIEW, image_slots, image_view, VkImageView)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(sampler, Sampler, SAMPLER, sampler_slots, sampler, VkSampler)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(tlas, Tlas, TLAS, tlas_slots, acceleration_structure, VkAccelerationStructureKHR)
_DAXA_DECL_COMMON_GP_RES_FUNCTIONS(blas, Blas, BLAS, blas_slots, acceleration_structure, VkAccelerationStructureKHR)

auto daxa_dvc_buffer_device_address(daxa_Device self, daxa_BufferId buffer, daxa_DeviceAddress * out_addr) -> daxa_Result
{
    auto const * hot_data = self->gpu_sro_table.buffer_slots.safe_get_hot(std::bit_cast<BufferId>(buffer));
    if (!hot_data)
    {
        return DAXA_RESULT_INVALID_BUFFER_ID;
    }
    *out_addr = static_cast<daxa_DeviceAddress>(hot_data->device_address);
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_buffer_host_address(daxa_Device self, daxa_BufferId buffer, void ** out_addr) -> daxa_Result
{
    auto const * hot_data = self->gpu_sro_table.buffer_slots.safe_get_hot(std::bit_cast<BufferId>(buffer));
    if (!hot_data)
    {
        return DAXA_RESULT_INVALID_BUFFER_ID;
    }
    if (hot_data->host_address == 0)
    {
        return DAXA_RESULT_BUFFER_NOT_HOST_VISIBLE;
    }
    *out_addr = hot_data->host_address;
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_tlas_device_address(daxa_Device self, daxa_TlasId tlas, daxa_DeviceAddress * out_addr) -> daxa_Result
{
    auto const * hot_data = self->gpu_sro_table.tlas_slots.safe_get_hot(std::bit_cast<TlasId>(tlas));
    if (!hot_data)
    {
        return DAXA_RESULT_INVALID_TLAS_ID;
    }
    *out_addr = static_cast<daxa_DeviceAddress>(hot_data->device_address);
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_blas_device_address(daxa_Device self, daxa_BlasId blas, daxa_DeviceAddress * out_addr) -> daxa_Result
{
    auto const * hot_data = self->gpu_sro_table.blas_slots.safe_get_hot(std::bit_cast<BlasId>(blas));
    if (!hot_data)
    {
        return DAXA_RESULT_INVALID_BLAS_ID;
    }
    *out_addr = static_cast<daxa_DeviceAddress>(hot_data->device_address);
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_buffer_device_address_to_buffer(daxa_Device self, daxa_DeviceAddress address, daxa_BufferOffsetPair * out_buffer_offset_pair) -> daxa_Result
{
    if (out_buffer_offset_pair == nullptr)
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_MEMORY_MAP_FAILED, DAXA_RESULT_ERROR_MEMORY_MAP_FAILED);
    }

    std::shared_lock lifetime_lock{self->gpu_sro_table.lifetime_lock};

    for (u32 bi = 0; bi < self->gpu_sro_table.buffer_slots.next_index; ++bi)
    {
        u64 version_refcnt = self->gpu_sro_table.buffer_slots.version_refcnt_of_slot(bi);
        if (GpuResourcePool<>::get_refcnt(version_refcnt) > 0)
        {
            daxa::BufferId id = {bi, GpuResourcePool<>::get_version(version_refcnt)};
            ImplBufferSlot const & buffer_slot = self->slot(id);
            ImplBufferSlot::HotData const & buffer_hot_data = self->hot_slot(id);

            bool const address_falls_into_buffer_memory = 
                std::bit_cast<daxa_u64>(address) >= std::bit_cast<daxa_u64>(buffer_hot_data.device_address) &&
                std::bit_cast<daxa_u64>(address) < std::bit_cast<daxa_u64>(buffer_hot_data.device_address) + buffer_slot.info.size;

            if (address_falls_into_buffer_memory)
            {
                out_buffer_offset_pair->buffer = std::bit_cast<daxa_BufferId>(id);
                out_buffer_offset_pair->offset = std::bit_cast<daxa_u64>(address) - std::bit_cast<daxa_u64>(buffer_hot_data.device_address);
                return DAXA_RESULT_SUCCESS;
            }
        }
    }

    return DAXA_RESULT_ERROR_ADDRESS_BELONGS_TO_NO_BUFFER;
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

auto daxa_dvc_get_vk_queue(daxa_Device self, daxa_Queue queue, VkQueue * vk_queue, uint32_t * vk_queue_type_index) -> daxa_Result
{
    if (!self->valid_queue(queue))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_INVALID_QUEUE, DAXA_RESULT_ERROR_INVALID_QUEUE);
    }
    auto const & daxa_queue = self->get_queue(queue);
    if (vk_queue)
        *vk_queue = daxa_queue.vk_queue;
    if (vk_queue_type_index)
        *vk_queue_type_index = daxa_queue.vk_queue_type_index;
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_wait_idle(daxa_Device self) -> daxa_Result
{
    for (u32 queue_i = 0; queue_i < static_cast<u32>(self->queues.size()); ++queue_i)
    {
        if (self->queues[queue_i].vk_queue != VK_NULL_HANDLE)
        {
            self->queues[queue_i].mtx.lock();
        }
    }

    auto result = std::bit_cast<daxa_Result>(vkDeviceWaitIdle(self->vk_device));

    for (u32 reverse_i = static_cast<u32>(self->queues.size()); reverse_i > 0; --reverse_i)
    {
        u32 const queue_i = reverse_i - 1;
        if (self->queues[queue_i].vk_queue != VK_NULL_HANDLE)
        {
            self->queues[queue_i].mtx.unlock();
        }
    }

    _DAXA_RETURN_IF_ERROR(result, result)
    return result;
}

auto daxa_dvc_queue_wait_idle(daxa_Device self, daxa_Queue queue) -> daxa_Result
{
    if (!self->valid_queue(queue))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_INVALID_QUEUE, DAXA_RESULT_ERROR_INVALID_QUEUE);
    }
    daxa_ImplDevice::ImplQueue & impl_queue = self->get_queue(queue);
    std::unique_lock queue_lock{impl_queue.mtx};

    daxa_Result result = std::bit_cast<daxa_Result>(vkQueueWaitIdle(impl_queue.vk_queue));
    _DAXA_RETURN_IF_ERROR(result, result)
    return result;
}

auto daxa_dvc_queue_count(daxa_Device self, daxa_QueueType queue_type, u32 * out_value) -> daxa_Result
{
    if (queue_type >= DAXA_QUEUE_TYPE_MAX_ENUM)
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_INVALID_QUEUE, DAXA_RESULT_ERROR_INVALID_QUEUE);
    }
    *out_value = self->queue_families[queue_type].queue_count;
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

auto queue_to_queue_index(daxa_Queue queue) -> u32
{
    u32 offsets[3] = {
        0,
        1,
        1 + DAXA_MAX_COMPUTE_QUEUE_COUNT,
    };
    return offsets[static_cast<u32>(queue.type)] + queue.index;
}

auto daxa_dvc_latest_queue_submit_index(daxa_Device self, daxa_Queue queue, daxa_u64 * submit_index) -> daxa_Result
{
    if (!self->valid_queue(queue))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_INVALID_QUEUE, DAXA_RESULT_ERROR_INVALID_QUEUE);
    }
    u32 queue_index = queue_to_queue_index(queue);
    auto & impl_queue = self->queues[queue_index];
    *submit_index = impl_queue.latest_pending_submit_timeline_value.load();
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_wait_on_submit(daxa_Device self, daxa_WaitOnSubmitInfo const * info) -> daxa_Result
{
    if (!self->valid_queue(info->queue))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_INVALID_QUEUE, DAXA_RESULT_ERROR_INVALID_QUEUE);
    }
    u32 queue_index = queue_to_queue_index(info->queue);
    auto & impl_queue = self->queues[queue_index];

    VkSemaphoreWaitInfo const vk_semaphore_wait_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .pNext = nullptr,
        .flags = {},
        .semaphoreCount = 1,
        .pSemaphores = &impl_queue.gpu_queue_local_timeline,
        .pValues = &info->queue_submit_index,
    };

    return static_cast<daxa_Result>(vkWaitSemaphores(self->vk_device, &vk_semaphore_wait_info, info->timeout));
}

auto daxa_dvc_submit_commands(daxa_Device self, daxa_CommandSubmitInfo const * info, daxa_u64 * out_submit_index) -> daxa_Result
{
    std::array<u8, 1u << 13u /*8kib*/> stack_memory;
    MemoryArena m_arena = MemoryArena{"daxa_dvc_submit_commands dyn stack memory", stack_memory};

    if (!self->valid_queue(info->queue))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_INVALID_QUEUE, DAXA_RESULT_ERROR_INVALID_QUEUE);
    }

    std::shared_lock lifetime_lock{self->gpu_sro_table.lifetime_lock};

    for (daxa_ExecutableCommandList commands : std::span{info->command_lists, info->command_list_count})
    {
        if (commands->info.queue_type != info->queue.type)
        {
            _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_CMD_LIST_SUBMIT_QUEUE_TYPE_MISMATCH, DAXA_RESULT_ERROR_CMD_LIST_SUBMIT_QUEUE_TYPE_MISMATCH);
        }
        for (BufferId id : commands->command_arena->used_buffers)
        {
            if (!daxa_dvc_is_buffer_valid(self, id))
            {
                _DAXA_RETURN_IF_ERROR(DAXA_RESULT_COMMAND_REFERENCES_INVALID_BUFFER_ID, DAXA_RESULT_COMMAND_REFERENCES_INVALID_BUFFER_ID);
            }
        }
        for (ImageId id : commands->command_arena->used_images)
        {
            if (!daxa_dvc_is_image_valid(self, id))
            {
                _DAXA_RETURN_IF_ERROR(DAXA_RESULT_COMMAND_REFERENCES_INVALID_IMAGE_ID, DAXA_RESULT_COMMAND_REFERENCES_INVALID_IMAGE_ID);
            }
        }
        for (ImageViewId id : commands->command_arena->used_image_views)
        {
            if (!daxa_dvc_is_image_view_valid(self, id))
            {
                _DAXA_RETURN_IF_ERROR(DAXA_RESULT_COMMAND_REFERENCES_INVALID_IMAGE_VIEW_ID, DAXA_RESULT_COMMAND_REFERENCES_INVALID_IMAGE_VIEW_ID);
            }
        }
        for (SamplerId id : commands->command_arena->used_samplers)
        {
            if (!daxa_dvc_is_sampler_valid(self, id))
            {
                _DAXA_RETURN_IF_ERROR(DAXA_RESULT_COMMAND_REFERENCES_INVALID_SAMPLER_ID, DAXA_RESULT_COMMAND_REFERENCES_INVALID_SAMPLER_ID);
            }
        }
    }

    daxa_ImplDevice::ImplQueue & queue = self->get_queue(info->queue);
    std::unique_lock queue_lock{queue.mtx}; // lock MUST BE before timeline fetch and until after vkQueueSubmit. Otherwise the timeline order can be broken.

    u64 const current_timeline_value = self->global_submit_timeline.fetch_add(1) + 1;
    queue.latest_pending_submit_timeline_value.store(current_timeline_value);
    if (out_submit_index != nullptr)
    {
        *out_submit_index = current_timeline_value;
    }

    for (auto const & commands : std::span{info->command_lists, info->command_list_count})
    {
        for (auto [id, index] : commands->command_arena->deferred_destructions)
        {
            [[maybe_unused]] daxa_Result _ignore = {};
            switch (index)
            {
            case DEFERRED_DESTRUCTION_BUFFER_INDEX: _ignore = daxa_dvc_destroy_buffer(self, std::bit_cast<daxa_BufferId>(id)); break;
            case DEFERRED_DESTRUCTION_IMAGE_INDEX: _ignore = daxa_dvc_destroy_image(self, std::bit_cast<daxa_ImageId>(id)); break;
            case DEFERRED_DESTRUCTION_IMAGE_VIEW_INDEX: _ignore = daxa_dvc_destroy_image_view(self, std::bit_cast<daxa_ImageViewId>(id)); break;
            case DEFERRED_DESTRUCTION_SAMPLER_INDEX: _ignore = daxa_dvc_destroy_sampler(self, std::bit_cast<daxa_SamplerId>(id)); break;
            }
        }
        commands->command_arena->deferred_destructions.clear();
    }

    ArenaDynamicArray8k<VkCommandBuffer> submit_vk_command_buffers = {&m_arena};
    for (auto const & commands : std::span{info->command_lists, info->command_list_count})
    {
        submit_vk_command_buffers.push_back(commands->command_arena->vk_command_buffer);
    }

    ArenaDynamicArray8k<VkSemaphore> submit_semaphore_signals = {&m_arena}; // All timeline semaphores come first, then binary semaphores follow.
    ArenaDynamicArray8k<u64> submit_semaphore_signal_values = {&m_arena};   // Used for timeline semaphores. Ignored (push dummy value) for binary semaphores.

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
    ArenaDynamicArray8k<VkSemaphore> submit_semaphore_waits = {&m_arena}; // All timeline semaphores come first, then binary semaphores follow.
    ArenaDynamicArray8k<VkPipelineStageFlags> submit_semaphore_wait_stage_masks = {&m_arena};
    ArenaDynamicArray8k<u64> submit_semaphore_wait_values = {&m_arena}; // Used for timeline semaphores. Ignored (push dummy value) for binary semaphores.

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

    for (auto const & queue_submit_index : std::span{info->wait_queue_submit_indices, info->wait_queue_submit_indices_count})
    {
        daxa_ImplDevice::ImplQueue & wait_queue = self->get_queue(queue_submit_index.queue);
        submit_semaphore_waits.push_back(wait_queue.gpu_queue_local_timeline);
        submit_semaphore_wait_stage_masks.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        submit_semaphore_wait_values.push_back(queue_submit_index.index);
    }

    VkTimelineSemaphoreSubmitInfo timeline_info{
        .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreValueCount = static_cast<u32>(submit_semaphore_wait_values.size()),
        .pWaitSemaphoreValues = submit_semaphore_wait_values.clone_to_contiguous().data(),
        .signalSemaphoreValueCount = static_cast<u32>(submit_semaphore_signal_values.size()),
        .pSignalSemaphoreValues = submit_semaphore_signal_values.clone_to_contiguous().data(),
    };

    VkSubmitInfo const vk_submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = r_cast<void *>(&timeline_info),
        .waitSemaphoreCount = static_cast<u32>(submit_semaphore_waits.size()),
        .pWaitSemaphores = submit_semaphore_waits.clone_to_contiguous().data(),
        .pWaitDstStageMask = submit_semaphore_wait_stage_masks.clone_to_contiguous().data(),
        .commandBufferCount = static_cast<u32>(submit_vk_command_buffers.size()),
        .pCommandBuffers = submit_vk_command_buffers.clone_to_contiguous().data(),
        .signalSemaphoreCount = static_cast<u32>(submit_semaphore_signals.size()),
        .pSignalSemaphores = submit_semaphore_signals.clone_to_contiguous().data(),
    };

    daxa_Result result = static_cast<daxa_Result>(vkQueueSubmit(queue.vk_queue, 1, &vk_submit_info, VK_NULL_HANDLE));
    _DAXA_RETURN_IF_ERROR(result, result)

    return result;
}

auto daxa_dvc_present_frame(daxa_Device self, daxa_PresentInfo const * info) -> daxa_Result
{
    if (info->queue.type != static_cast<daxa_QueueType>(info->swapchain->info.queue_type))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_PRESENT_QUEUE_TYPE_MISMATCH, DAXA_RESULT_ERROR_PRESENT_QUEUE_TYPE_MISMATCH)
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

    daxa_ImplDevice::ImplQueue & impl_queue = self->get_queue(info->queue);
    std::unique_lock queue_lock{impl_queue.mtx};

    return static_cast<daxa_Result>(vkQueuePresentKHR(impl_queue.vk_queue, &present_info));
}

auto daxa_dvc_collect_garbage(daxa_Device self) -> daxa_Result
{
    std::unique_lock lifetime_lock{self->gpu_sro_table.lifetime_lock};
    std::unique_lock lock{self->zombies_mtx};
    std::unique_lock command_pools_lock{self->commands.mtx};

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
            vkDestroyPipeline(self->vk_device, pipeline_zombie.vk_pipeline, nullptr);
        });
    check_and_cleanup_gpu_resources(
        self->semaphore_zombies,
        [&](auto & semaphore_zombie)
        {
            vkDestroySemaphore(self->vk_device, semaphore_zombie.vk_semaphore, nullptr);
        });
    check_and_cleanup_gpu_resources(
        self->split_barrier_zombies,
        [&](auto & split_barrier_zombie)
        {
            vkDestroyEvent(self->vk_device, split_barrier_zombie.vk_event, nullptr);
        });
    check_and_cleanup_gpu_resources(
        self->timeline_query_pool_zombies,
        [&](auto & timeline_query_pool_zombie)
        {
            vkDestroyQueryPool(self->vk_device, timeline_query_pool_zombie.vk_timeline_query_pool, nullptr);
        });
    check_and_cleanup_gpu_resources(
        self->memory_block_zombies,
        [&](auto & memory_block_zombie)
        {
            vmaFreeMemory(self->vma_allocator, memory_block_zombie.allocation);
        });
    check_and_cleanup_gpu_resources(
        self->command_zombies,
        [&](auto & cmd_arena)
        {
            // Will error when pool reset failed. That is a unrecoverable error, we do not need to return it.
            // In the future it would still be nice to return this if we refactor the callback hell here.
            [[maybe_unused]] auto result = self->commands.retire_arena(self->vk_device, cmd_arena, &command_pools_lock);
        });
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_properties(daxa_Device device) -> daxa_DeviceProperties const *
{
    return &device->properties;
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

auto daxa_ImplDevice::create_2(daxa_Instance instance, daxa_DeviceInfo2 const & info, ImplPhysicalDevice const & physical_device, daxa_DeviceProperties const & properties, daxa_Device out_device) -> daxa_Result
{
    using namespace daxa;
    daxa_Result result = {};

    // Set properties and feature variables:
    auto self = out_device;
    self->vk_physical_device = physical_device.vk_handle;
    self->properties = properties;
    self->instance = instance;
    self->info = std::bit_cast<DeviceInfo2>(info);

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

    // Queue Selection and Verification
    u32 vk_queue_request_count = {};
    std::array<VkDeviceQueueCreateInfo, 3> queues_ci = {};
    std::array<f32, std::max(DAXA_MAX_COMPUTE_QUEUE_COUNT, DAXA_MAX_TRANSFER_QUEUE_COUNT)> queue_priorities = {0.0f, 0.0f, 0.0f, 0.0f};
    struct QueueRequest
    {
        u32 vk_family_index;
        u32 count;
    };
    std::array<QueueRequest, 3> vk_queue_requests = {};
    {
        u32 queue_type_props_count = 0;
        std::vector<VkQueueFamilyProperties> queue_props;
        vkGetPhysicalDeviceQueueFamilyProperties(self->vk_physical_device, &queue_type_props_count, nullptr);
        queue_props.resize(queue_type_props_count);
        vkGetPhysicalDeviceQueueFamilyProperties(self->vk_physical_device, &queue_type_props_count, queue_props.data());
        std::vector<VkBool32> supports_present;
        supports_present.resize(queue_type_props_count);

        // SELECT QUEUE FAMILIES
        for (u32 i = 0; i < queue_type_props_count; i++)
        {
            bool const supports_graphics = queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
            bool const supports_compute = queue_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
            bool const supports_transfer = queue_props[i].queueFlags & VK_QUEUE_TRANSFER_BIT;
            if (self->queue_families[DAXA_QUEUE_TYPE_MAIN].vk_index == ~0u && supports_graphics && supports_compute && supports_transfer)
            {
                self->queue_families[DAXA_QUEUE_TYPE_MAIN].vk_index = i;
                self->queue_families[DAXA_QUEUE_TYPE_MAIN].queue_count = 1;
                self->queue_families[DAXA_QUEUE_TYPE_MAIN].vk_queue_type_index = i;
                self->valid_vk_queue_families[self->valid_vk_queue_type_count++] = i;
                vk_queue_requests[vk_queue_request_count++] = QueueRequest{i, 1};
            }
            if (self->queue_families[DAXA_QUEUE_TYPE_COMPUTE].vk_index == ~0u && !supports_graphics && supports_compute && supports_transfer)
            {
                self->queue_families[DAXA_QUEUE_TYPE_COMPUTE].vk_index = i;
                self->queue_families[DAXA_QUEUE_TYPE_COMPUTE].queue_count = std::min(queue_props[i].queueCount, DAXA_MAX_COMPUTE_QUEUE_COUNT);
                self->queue_families[DAXA_QUEUE_TYPE_COMPUTE].vk_queue_type_index = i;
                self->valid_vk_queue_families[self->valid_vk_queue_type_count++] = i;
                vk_queue_requests[vk_queue_request_count++] = QueueRequest{i, self->queue_families[DAXA_QUEUE_TYPE_COMPUTE].queue_count};
            }
            if (self->queue_families[DAXA_QUEUE_TYPE_TRANSFER].vk_index == ~0u && !supports_graphics && !supports_compute && supports_transfer)
            {
                self->queue_families[DAXA_QUEUE_TYPE_TRANSFER].vk_index = i;
                self->queue_families[DAXA_QUEUE_TYPE_TRANSFER].queue_count = std::min(queue_props[i].queueCount, DAXA_MAX_TRANSFER_QUEUE_COUNT);
                self->queue_families[DAXA_QUEUE_TYPE_TRANSFER].vk_queue_type_index = i;
                self->valid_vk_queue_families[self->valid_vk_queue_type_count++] = i;
                vk_queue_requests[vk_queue_request_count++] = QueueRequest{i, self->queue_families[DAXA_QUEUE_TYPE_TRANSFER].queue_count};
            }
        }

        if (self->queue_families[DAXA_QUEUE_TYPE_MAIN].vk_index == ~0u)
        {
            result = DAXA_RESULT_ERROR_NO_GRAPHICS_QUEUE_FOUND;
        }
        _DAXA_RETURN_IF_ERROR(result, result)

        for (u32 type = 0; type < vk_queue_request_count; ++type)
        {
            queues_ci[type] = VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = vk_queue_requests[type].vk_family_index,
                .queueCount = vk_queue_requests[type].count,
                .pQueuePriorities = queue_priorities.data(),
            };
        };
    }

    PhysicalDeviceFeaturesStruct enabled_features = {};
    enabled_features.initialize(physical_device.extensions);
    fill_create_features(enabled_features, properties.implicit_features, info.explicit_features);

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
    result = static_cast<daxa_Result>(vkCreateDevice(self->vk_physical_device, &device_ci, nullptr, &self->vk_device));
    _DAXA_RETURN_IF_ERROR(result, result)
    defer
    {
        if (result != DAXA_RESULT_SUCCESS && self->vk_device)
        {
            vkDestroyDevice(self->vk_device, nullptr);
        }
    };

    // Queue initialization:
    defer
    {
        if (result != DAXA_RESULT_SUCCESS)
        {
            for (auto & queue : self->queues)
            {
                queue.cleanup(self->vk_device);
            }
        }
    };
    for (u32 i = 0; i < self->queues.size(); ++i)
    {
        if (self->queues[i].queue_index >= self->queue_families[self->queues[i].type].queue_count)
        {
            continue;
        }
        auto const vk_queue_type = self->queue_families[self->queues[i].type].vk_index;
        self->queues[i].vk_queue_type_index = vk_queue_type;
        result = self->queues[i].initialize(self->vk_device);
        _DAXA_RETURN_IF_ERROR(result, result)
    }

    // Query ext function pointers
    {
        if (properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_DYNAMIC_STATE_3)
        {
            self->vkCmdSetRasterizationSamplesEXT = r_cast<PFN_vkCmdSetRasterizationSamplesEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdSetRasterizationSamplesEXT"));
        }

        if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE)
        {
            self->vkSetDebugUtilsObjectNameEXT = r_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(self->vk_device, "vkSetDebugUtilsObjectNameEXT"));
            self->vkCmdBeginDebugUtilsLabelEXT = r_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdBeginDebugUtilsLabelEXT"));
            self->vkCmdEndDebugUtilsLabelEXT = r_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdEndDebugUtilsLabelEXT"));
        }

        if (properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_MESH_SHADER)
        {
            self->vkCmdDrawMeshTasksEXT = r_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdDrawMeshTasksEXT"));
            self->vkCmdDrawMeshTasksIndirectEXT = r_cast<PFN_vkCmdDrawMeshTasksIndirectEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdDrawMeshTasksIndirectEXT"));
            self->vkCmdDrawMeshTasksIndirectCountEXT = r_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCmdDrawMeshTasksIndirectCountEXT"));
        }

        if (properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_BASIC_RAY_TRACING)
        {
            self->vkGetAccelerationStructureBuildSizesKHR = r_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(self->vk_device, "vkGetAccelerationStructureBuildSizesKHR"));
            self->vkCreateAccelerationStructureKHR = r_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(self->vk_device, "vkCreateAccelerationStructureKHR"));
            self->vkDestroyAccelerationStructureKHR = r_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(self->vk_device, "vkDestroyAccelerationStructureKHR"));
            self->vkCmdWriteAccelerationStructuresPropertiesKHR = r_cast<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>(vkGetDeviceProcAddr(self->vk_device, "vkCmdWriteAccelerationStructuresPropertiesKHR"));
            self->vkCmdBuildAccelerationStructuresKHR = r_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(self->vk_device, "vkCmdBuildAccelerationStructuresKHR"));
            self->vkGetAccelerationStructureDeviceAddressKHR = r_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(self->vk_device, "vkGetAccelerationStructureDeviceAddressKHR"));
        }

        if (properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_RAY_TRACING_PIPELINE)
        {
            self->vkCreateRayTracingPipelinesKHR = r_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(self->vk_device, "vkCreateRayTracingPipelinesKHR"));
            self->vkCmdTraceRaysKHR = r_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(self->vk_device, "vkCmdTraceRaysKHR"));
            self->vkCmdTraceRaysIndirectKHR = r_cast<PFN_vkCmdTraceRaysIndirectKHR>(vkGetDeviceProcAddr(self->vk_device, "vkCmdTraceRaysIndirectKHR"));
            self->vkGetRayTracingShaderGroupHandlesKHR = r_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(self->vk_device, "vkGetRayTracingShaderGroupHandlesKHR"));
        }

        if (properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_HOST_IMAGE_COPY)
        {
            self->vkTransitionImageLayoutEXT = r_cast<PFN_vkTransitionImageLayoutEXT>(vkGetDeviceProcAddr(self->vk_device, "vkTransitionImageLayoutEXT"));
            self->vkCopyMemoryToImageEXT = r_cast<PFN_vkCopyMemoryToImageEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCopyMemoryToImageEXT"));
            self->vkCopyImageToMemoryEXT = r_cast<PFN_vkCopyImageToMemoryEXT>(vkGetDeviceProcAddr(self->vk_device, "vkCopyImageToMemoryEXT"));
        }
    }

    VkCommandPool init_cmd_pool = {};
    VkCommandBuffer init_cmd_buffer = {};
    VkCommandPoolCreateInfo const vk_command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = self->get_queue(DAXA_QUEUE_MAIN).vk_queue_type_index,
    };

    result = static_cast<daxa_Result>(vkCreateCommandPool(self->vk_device, &vk_command_pool_create_info, nullptr, &init_cmd_pool));
    _DAXA_RETURN_IF_ERROR(result, result)
    defer
    {
        // Should always be destroyed at end of function!
        if (init_cmd_pool)
        {
            vkDestroyCommandPool(self->vk_device, init_cmd_pool, nullptr);
        }
    };

    VkCommandBufferAllocateInfo const vk_command_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = init_cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    result = static_cast<daxa_Result>(vkAllocateCommandBuffers(self->vk_device, &vk_command_buffer_allocate_info, &init_cmd_buffer));
    _DAXA_RETURN_IF_ERROR(result, result)

    VkCommandBufferBeginInfo const vk_command_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = {},
    };
    result = static_cast<daxa_Result>(vkBeginCommandBuffer(init_cmd_buffer, &vk_command_buffer_begin_info));
    _DAXA_RETURN_IF_ERROR(result, result)

    VmaVulkanFunctions const vma_vulkan_functions{
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
                vkDestroyImageView(self->vk_device, self->vk_null_image_view, nullptr);
            }
            if (self->vk_null_sampler)
            {
                vkDestroySampler(self->vk_device, self->vk_null_sampler, nullptr);
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
            .queueFamilyIndexCount = self->valid_vk_queue_type_count,
            .pQueueFamilyIndices = self->valid_vk_queue_families.data(),
        };

        VmaAllocationInfo vma_allocation_info = {};

        auto vma_allocation_flags = static_cast<VmaAllocationCreateFlags>(
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
            self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &buffer_name_info);
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
            .memory_flags = {},
        };
        VkImageCreateInfo const vk_image_create_info = initialize_image_create_info_from_image_info(
            self, *r_cast<daxa_ImageInfo const *>(&image_info));

        VmaAllocationCreateInfo const null_img_allocation_create_info{
            .flags = {},
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
            self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &image_name_info);
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

        result = static_cast<daxa_Result>(vkCreateImageView(self->vk_device, &vk_image_view_create_info, nullptr, &self->vk_null_image_view));
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
            self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &image_name_info);
        }

        VkImageMemoryBarrier vk_image_mem_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = {},
            .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout = {},
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
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
        vkCmdCopyBufferToImage(init_cmd_buffer, self->vk_null_buffer, self->vk_null_image, VK_IMAGE_LAYOUT_GENERAL, 1, &vk_buffer_image_copy);
        vk_image_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        vk_image_mem_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        vk_image_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
        vk_image_mem_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL,
        vk_image_mem_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        vk_image_mem_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        vkCmdPipelineBarrier(init_cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, {}, {}, {}, 1, &vk_image_mem_barrier);

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
        result = static_cast<daxa_Result>(vkCreateSampler(self->vk_device, &vk_sampler_create_info, nullptr, &self->vk_null_sampler));
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
            self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &sampler_name_info);
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
            .queueFamilyIndexCount = self->valid_vk_queue_type_count, // Buffers are always shared across all queues.
            .pQueueFamilyIndices = self->valid_vk_queue_families.data(),
        };

        VmaAllocationCreateInfo const bda_allocation_create_info{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = {},
            .preferredFlags = {},
            .memoryTypeBits = std::numeric_limits<u32>::max(),
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.5f,
        };

        VmaAllocationInfo bda_allocation_vma_allocation_info = {};
        result = static_cast<daxa_Result>(vmaCreateBuffer(self->vma_allocator, &bda_buffer_create_info, &bda_allocation_create_info, &self->buffer_device_address_buffer, &self->buffer_device_address_buffer_allocation, &bda_allocation_vma_allocation_info));
        _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_CREATE_BDA_BUFFER);

        self->buffer_device_address_buffer_host_ptr = static_cast<u64 *>(bda_allocation_vma_allocation_info.pMappedData);
    }

    // Set debug names:
    if ((self->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && !self->info.name.view().empty())
    {
        VkDebugUtilsObjectNameInfoEXT const device_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_DEVICE,
            .objectHandle = std::bit_cast<uint64_t>(self->vk_device),
            .pObjectName = self->info.name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_name_info);

        VkDebugUtilsObjectNameInfoEXT const device_buffer_device_address_buffer_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = std::bit_cast<uint64_t>(self->buffer_device_address_buffer),
            .pObjectName = self->info.name.c_str(),
        };
        self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &device_buffer_device_address_buffer_name_info);

        for (auto & queue : self->queues)
        {
            if (queue.vk_queue)
            {
                std::string name = {"[DAXA DEVICE] Queue "};
                name += to_string(static_cast<QueueType>(queue.type));

                VkDebugUtilsObjectNameInfoEXT const queue_name_info{
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_QUEUE,
                    .objectHandle = std::bit_cast<uint64_t>(queue.vk_queue),
                    .pObjectName = name.data(),
                };
                self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &queue_name_info);

                std::string timeline_name = {"[DAXA DEVICE] Queue timeline semaphore "};
                timeline_name += to_string(static_cast<QueueType>(queue.type));

                VkDebugUtilsObjectNameInfoEXT const timeline_semaphore_name_info{
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_SEMAPHORE,
                    .objectHandle = std::bit_cast<uint64_t>(queue.gpu_queue_local_timeline),
                    .pObjectName = timeline_name.data(),
                };
                self->vkSetDebugUtilsObjectNameEXT(self->vk_device, &timeline_semaphore_name_info);
            }
        }
    }

    result = self->gpu_sro_table.initialize(
        self->info.max_allowed_buffers,
        self->info.max_allowed_images,
        self->info.max_allowed_samplers,
        (properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_BASIC_RAY_TRACING) ? self->info.max_allowed_acceleration_structures : (~0u),
        self->vk_device,
        self->buffer_device_address_buffer,
        self->vkSetDebugUtilsObjectNameEXT);
    _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_SUBMIT_DEVICE_INIT_COMMANDS)

    result = static_cast<daxa_Result>(vkEndCommandBuffer(init_cmd_buffer));
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
    result = static_cast<daxa_Result>(vkQueueSubmit(self->get_queue(DAXA_QUEUE_MAIN).vk_queue, 1, &init_submit, {}));
    _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_SUBMIT_DEVICE_INIT_COMMANDS)

    // Wait for commands in from the init cmd list to complete.
    result = static_cast<daxa_Result>(vkDeviceWaitIdle(self->vk_device));
    _DAXA_RETURN_IF_ERROR(result, DAXA_RESULT_FAILED_TO_SUBMIT_DEVICE_INIT_COMMANDS)

    return DAXA_RESULT_SUCCESS;
}

auto daxa_ImplDevice::get_queue(daxa_Queue queue) -> daxa_ImplDevice::ImplQueue &
{
    u32 offsets[3] = {
        0,
        1,
        1 + DAXA_MAX_COMPUTE_QUEUE_COUNT,
    };
    return this->queues[offsets[queue.type] + queue.index];
}

auto daxa_ImplDevice::valid_queue(daxa_Queue queue) -> bool
{
    bool const in_range = queue.type < DAXA_QUEUE_TYPE_MAX_ENUM && queue.index < this->queue_families[queue.type].queue_count;
    if (in_range)
    {
        u32 const queue_index = queue_to_queue_index(queue);
        return this->queues[queue_index].vk_queue != VK_NULL_HANDLE;
    }
    return false;
}

auto daxa_ImplDevice::validate_image_slice(daxa_ImageMipArraySlice const & slice, daxa_ImageId image) -> daxa_ImageMipArraySlice
{
    if (slice.level_count == std::numeric_limits<u32>::max() || slice.level_count == 0)
    {
        auto & image_info = this->slot(image).info;
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

auto daxa_ImplDevice::validate_image_slice(daxa_ImageMipArraySlice const & slice, daxa_ImageViewId image_view) -> daxa_ImageMipArraySlice
{
    if (slice.level_count == std::numeric_limits<u32>::max() || slice.level_count == 0)
    {
        return this->slot(image_view).info.slice;
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
    auto [id, ret, hot_data] = slot_opt.value();
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
    result = static_cast<daxa_Result>(vkCreateImageView(vk_device, &view_ci, nullptr, &ret.view_slot.vk_image_view));
    _DAXA_RETURN_IF_ERROR(result, result)

    if ((this->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && !image_info.name.empty())
    {
        VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE,
            .objectHandle = std::bit_cast<uint64_t>(ret.vk_image),
            .pObjectName = image_info.name.c_str(),
        };
        this->vkSetDebugUtilsObjectNameEXT(this->vk_device, &swapchain_image_name_info);

        VkDebugUtilsObjectNameInfoEXT const swapchain_image_view_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
            .objectHandle = std::bit_cast<uint64_t>(ret.view_slot.vk_image_view),
            .pObjectName = image_info.name.c_str(),
        };
        this->vkSetDebugUtilsObjectNameEXT(this->vk_device, &swapchain_image_view_name_info);
    }

    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_image(this->vk_device, this->gpu_sro_table.vk_descriptor_set, ret.view_slot.vk_image_view, usage, id.index);
    }

    *out = ImageId{id};

    return result;
}

void daxa_ImplDevice::cleanup_buffer(BufferId id)
{
    auto gid = std::bit_cast<GPUResourceId>(id);
    ImplBufferSlot const & buffer_slot = this->gpu_sro_table.buffer_slots.unsafe_get(gid);
    ImplBufferSlot::HotData const & hot_data = this->gpu_sro_table.buffer_slots.unsafe_get_hot(gid);
    this->buffer_device_address_buffer_host_ptr[gid.index] = 0;
    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_buffer(this->vk_device, this->gpu_sro_table.vk_descriptor_set, this->vk_null_buffer, 0, VK_WHOLE_SIZE, gid.index);
    }
    if (buffer_slot.opt_memory_block != nullptr)
    {
        vkDestroyBuffer(this->vk_device, hot_data.vk_buffer, {});
    }
    else
    {
        vmaDestroyBuffer(this->vma_allocator, hot_data.vk_buffer, buffer_slot.vma_allocation);
    }
    gpu_sro_table.buffer_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_image(ImageId id)
{
    auto gid = std::bit_cast<GPUResourceId>(id);
    ImplImageSlot const & image_slot = gpu_sro_table.image_slots.unsafe_get(gid);
    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_image(
            this->vk_device,
            this->gpu_sro_table.vk_descriptor_set,
            this->vk_null_image_view,
            std::bit_cast<ImageUsageFlags>(image_slot.info.usage),
            gid.index);
    }
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
    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_image(this->vk_device, this->gpu_sro_table.vk_descriptor_set, this->vk_null_image_view, ImageUsageFlagBits::SHADER_STORAGE | ImageUsageFlagBits::SHADER_SAMPLED, std::bit_cast<daxa::ImageViewId>(id).index);
    }
    vkDestroyImageView(vk_device, image_slot.vk_image_view, nullptr);
    gpu_sro_table.image_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_sampler(SamplerId id)
{
    ImplSamplerSlot const & sampler_slot = this->gpu_sro_table.sampler_slots.unsafe_get(std::bit_cast<GPUResourceId>(id));
    {
        // Does not need external sync given we use update after bind.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
        write_descriptor_set_sampler(this->vk_device, this->gpu_sro_table.vk_descriptor_set, this->vk_null_sampler, std::bit_cast<GPUResourceId>(id).index);
    }
    vkDestroySampler(this->vk_device, sampler_slot.vk_sampler, nullptr);
    gpu_sro_table.sampler_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_tlas(TlasId id)
{
    ImplTlasSlot::HotData const & hot_data = this->gpu_sro_table.tlas_slots.unsafe_get_hot(std::bit_cast<GPUResourceId>(id));
    // TODO(Raytracing): Add null acceleration structure:
    // write_descriptor_set_acceleration_structure(this->vk_device, this->gpu_sro_table.vk_descriptor_set, this->vk_null_acceleration_structure, std::bit_cast<GPUResourceId>(id).index);
    this->vkDestroyAccelerationStructureKHR(this->vk_device, hot_data.vk_acceleration_structure, nullptr);
    gpu_sro_table.tlas_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
}

void daxa_ImplDevice::cleanup_blas(BlasId id)
{
    ImplBlasSlot::HotData const & hot_data = this->gpu_sro_table.blas_slots.unsafe_get_hot(std::bit_cast<GPUResourceId>(id));
    this->vkDestroyAccelerationStructureKHR(this->vk_device, hot_data.vk_acceleration_structure, nullptr);
    gpu_sro_table.blas_slots.unsafe_destroy_zombie_slot(std::bit_cast<GPUResourceId>(id));
}

auto daxa_ImplDevice::slot(daxa_BufferId buffer) const -> ImplBufferSlot const &
{
    return gpu_sro_table.buffer_slots.unsafe_get(std::bit_cast<daxa::GPUResourceId>(buffer));
}

auto daxa_ImplDevice::slot(daxa_ImageId image) const -> ImplImageSlot const &
{
    return gpu_sro_table.image_slots.unsafe_get(std::bit_cast<daxa::GPUResourceId>(image));
}

auto daxa_ImplDevice::slot(daxa_ImageViewId image_view) const -> ImplImageViewSlot const &
{
    return gpu_sro_table.image_slots.unsafe_get(std::bit_cast<daxa::GPUResourceId>(image_view)).view_slot;
}

auto daxa_ImplDevice::slot(daxa_SamplerId sampler) const -> ImplSamplerSlot const &
{
    return gpu_sro_table.sampler_slots.unsafe_get(std::bit_cast<daxa::GPUResourceId>(sampler));
}

auto daxa_ImplDevice::slot(daxa_TlasId tlas) const -> ImplTlasSlot const &
{
    return gpu_sro_table.tlas_slots.unsafe_get(std::bit_cast<daxa::GPUResourceId>(tlas));
}

auto daxa_ImplDevice::slot(daxa_BlasId blas) const -> ImplBlasSlot const &
{
    return gpu_sro_table.blas_slots.unsafe_get(std::bit_cast<daxa::GPUResourceId>(blas));
}

auto daxa_ImplDevice::hot_slot(daxa_BufferId buffer) const -> ImplBufferSlot::HotData const &
{
    return gpu_sro_table.buffer_slots.unsafe_get_hot(std::bit_cast<daxa::GPUResourceId>(buffer));
}

auto daxa_ImplDevice::hot_slot(daxa_ImageId image) const -> ImplImageSlot::HotData const &
{
    return gpu_sro_table.image_slots.unsafe_get_hot(std::bit_cast<daxa::GPUResourceId>(image));
}

auto daxa_ImplDevice::hot_slot(daxa_ImageViewId image_view) const -> ImplImageViewSlot::HotData const &
{
    return gpu_sro_table.image_slots.unsafe_get_hot(std::bit_cast<daxa::GPUResourceId>(image_view));
}

auto daxa_ImplDevice::hot_slot(daxa_SamplerId sampler) const -> ImplSamplerSlot::HotData const &
{
    return gpu_sro_table.sampler_slots.unsafe_get_hot(std::bit_cast<daxa::GPUResourceId>(sampler));
}

auto daxa_ImplDevice::hot_slot(daxa_TlasId tlas) const -> ImplTlasSlot::HotData const &
{
    return gpu_sro_table.tlas_slots.unsafe_get_hot(std::bit_cast<daxa::GPUResourceId>(tlas));
}

auto daxa_ImplDevice::hot_slot(daxa_BlasId blas) const -> ImplBlasSlot::HotData const &
{
    return gpu_sro_table.blas_slots.unsafe_get_hot(std::bit_cast<daxa::GPUResourceId>(blas));
}

void daxa_ImplDevice::zero_ref_callback(ImplHandle const * handle)
{
    auto self = rc_cast<daxa_Device>(handle);
    auto result = daxa_dvc_wait_idle(self);
    DAXA_DBG_ASSERT_TRUE_M(result == DAXA_RESULT_SUCCESS, "failed to wait idle");
    result = daxa_dvc_collect_garbage(self);
    DAXA_DBG_ASSERT_TRUE_M(result == DAXA_RESULT_SUCCESS, "failed to wait idle");
    self->commands.cleanup(self->vk_device);
    vmaDestroyBuffer(self->vma_allocator, self->buffer_device_address_buffer, self->buffer_device_address_buffer_allocation);
    self->gpu_sro_table.cleanup(self->vk_device);
    vmaDestroyImage(self->vma_allocator, self->vk_null_image, self->vk_null_image_vma_allocation);
    vmaDestroyBuffer(self->vma_allocator, self->vk_null_buffer, self->vk_null_buffer_vma_allocation);
    vmaDestroyAllocator(self->vma_allocator);
    vkDestroySampler(self->vk_device, self->vk_null_sampler, nullptr);
    vkDestroyImageView(self->vk_device, self->vk_null_image_view, nullptr);
    for (auto & queue : self->queues)
    {
        queue.cleanup(self->vk_device);
    }
    vkDestroyDevice(self->vk_device, nullptr);
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
            auto result = daxa_dvc_destroy_buffer(self, slot.buffer_id);
            DAXA_DBG_ASSERT_TRUE_M(result == DAXA_RESULT_SUCCESS, "Tlas owned buffer could not be destroyed.");
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
    zombiefy(this, id, gpu_sro_table.buffer_slots, this->buffer_zombies);
}

void daxa_ImplDevice::zombify_image(ImageId id)
{
    zombiefy(this, id, gpu_sro_table.image_slots, this->image_zombies);
}

void daxa_ImplDevice::zombify_image_view(ImageViewId id)
{
    zombiefy(this, id, gpu_sro_table.image_slots, this->image_view_zombies);
}

void daxa_ImplDevice::zombify_sampler(SamplerId id)
{
    zombiefy(this, id, gpu_sro_table.sampler_slots, this->sampler_zombies);
}

void daxa_ImplDevice::zombify_tlas(TlasId id)
{
    zombiefy(this, id, gpu_sro_table.tlas_slots, this->tlas_zombies);
}

void daxa_ImplDevice::zombify_blas(BlasId id)
{
    zombiefy(this, id, gpu_sro_table.blas_slots, this->blas_zombies);
}

auto daxa_dvc_copy_memory_to_image(daxa_Device self, daxa_MemoryToImageCopyInfo const * info) -> daxa_Result
{
    if ((self->properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_HOST_IMAGE_COPY) == 0)
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_EXTENSION_NOT_PRESENT, DAXA_RESULT_ERROR_EXTENSION_NOT_PRESENT);
    }
    if (!daxa_dvc_is_image_valid(self, info->image))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_INVALID_IMAGE_ID, DAXA_RESULT_INVALID_IMAGE_ID);
    }

    ImplImageSlot const & image = self->slot(info->image);
    VkMemoryToImageCopyEXT vk_memory_to_image_copy = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY_EXT,
        .pNext = nullptr,
        .pHostPointer = info->memory_ptr,
        .memoryRowLength = 0,
        .memoryImageHeight = 0,
        .imageSubresource = make_subresource_layers(info->image_slice, image.aspect_flags),
        .imageOffset = info->image_offset,
        .imageExtent = info->image_extent,
    };
    VkCopyMemoryToImageInfoEXT vk_memory_to_image_copy_ext = {
        .sType = VK_STRUCTURE_TYPE_COPY_MEMORY_TO_IMAGE_INFO_EXT,
        .pNext = nullptr,
        .flags = {},
        .dstImage = image.vk_image,
        .dstImageLayout = VK_IMAGE_LAYOUT_GENERAL,
        .regionCount = 1,
        .pRegions = &vk_memory_to_image_copy,
    };
    auto result = static_cast<daxa_Result>(self->vkCopyMemoryToImageEXT(self->vk_device, &vk_memory_to_image_copy_ext));
    _DAXA_RETURN_IF_ERROR(result, result);
    return DAXA_RESULT_SUCCESS;
}

auto daxa_dvc_copy_image_to_memory(daxa_Device self, daxa_ImageToMemoryCopyInfo const * info) -> daxa_Result
{
    if ((self->properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_HOST_IMAGE_COPY) == 0)
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_EXTENSION_NOT_PRESENT, DAXA_RESULT_ERROR_EXTENSION_NOT_PRESENT);
    }
    if (!daxa_dvc_is_image_valid(self, info->image))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_INVALID_IMAGE_ID, DAXA_RESULT_INVALID_IMAGE_ID);
    }

    ImplImageSlot const & image = self->slot(info->image);
    VkImageToMemoryCopyEXT vk_image_to_memory_copy = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_TO_MEMORY_COPY_EXT,
        .pNext = nullptr,
        .pHostPointer = info->memory_ptr,
        .memoryRowLength = 0,
        .memoryImageHeight = 0,
        .imageSubresource = make_subresource_layers(info->image_slice, image.aspect_flags),
        .imageOffset = info->image_offset,
        .imageExtent = info->image_extent,
    };
    VkCopyImageToMemoryInfoEXT vk_image_to_memory_copy_ext = {
        .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_MEMORY_INFO_EXT,
        .pNext = nullptr,
        .flags = {},
        .srcImage = image.vk_image,
        .srcImageLayout = VK_IMAGE_LAYOUT_GENERAL,
        .regionCount = 1,
        .pRegions = &vk_image_to_memory_copy,
    };
    auto result = static_cast<daxa_Result>(self->vkCopyImageToMemoryEXT(self->vk_device, &vk_image_to_memory_copy_ext));
    _DAXA_RETURN_IF_ERROR(result, result);
    return result;
}

auto daxa_dvc_transition_image_layout(daxa_Device self, daxa_HostImageLayoutTransitionInfo const * info) -> daxa_Result
{
    daxa_HostImageLayoutOperationInfo new_info = {};
    new_info.image = info->image;
    if (info->old_image_layout == DAXA_IMAGE_LAYOUT_UNDEFINED)
    {
        new_info.layout_operation = DAXA_IMAGE_LAYOUT_OPERATION_TO_GENERAL;
    }
    if (info->new_image_layout == DAXA_IMAGE_LAYOUT_PRESENT_SRC)
    {
        new_info.layout_operation = DAXA_IMAGE_LAYOUT_OPERATION_TO_PRESENT_SRC;
    }
    return daxa_dvc_image_layout_operation(self, &new_info);
}

auto daxa_dvc_image_layout_operation(daxa_Device self, daxa_HostImageLayoutOperationInfo const * info) -> daxa_Result
{
    if ((self->properties.implicit_features & DAXA_IMPLICIT_FEATURE_FLAG_HOST_IMAGE_COPY) == 0)
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_ERROR_EXTENSION_NOT_PRESENT, DAXA_RESULT_ERROR_EXTENSION_NOT_PRESENT);
    }
    if (!daxa_dvc_is_image_valid(self, info->image))
    {
        _DAXA_RETURN_IF_ERROR(DAXA_RESULT_INVALID_IMAGE_ID, DAXA_RESULT_INVALID_IMAGE_ID);
    }

    ImplImageSlot const & image_slot = self->slot(info->image);
    VkHostImageLayoutTransitionInfoEXT vk_host_image_layout_transition_info = {
        .sType = VK_STRUCTURE_TYPE_HOST_IMAGE_LAYOUT_TRANSITION_INFO_EXT,
        .pNext = nullptr,
        .image = image_slot.vk_image,
        .oldLayout = info->layout_operation == DAXA_IMAGE_LAYOUT_OPERATION_TO_GENERAL ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_GENERAL,
        .newLayout = info->layout_operation == DAXA_IMAGE_LAYOUT_OPERATION_TO_PRESENT_SRC ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_GENERAL,
        .subresourceRange = make_subresource_range(image_slot.view_slot.info.slice, image_slot.aspect_flags),
    };
    auto result = static_cast<daxa_Result>(self->vkTransitionImageLayoutEXT(self->vk_device, 1, &vk_host_image_layout_transition_info));
    _DAXA_RETURN_IF_ERROR(result, result);
    return result;
}

// --- End Internal Functions ---