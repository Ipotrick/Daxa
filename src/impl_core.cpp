#include "impl_core.hpp"

#include "impl_instance.hpp"
#include "impl_device.hpp"

// --- Begin Helpers ---

auto is_depth_format(Format format) -> bool
{
    switch (format)
    {
    case Format::D16_UNORM: return true;
    case Format::X8_D24_UNORM_PACK32: return true;
    case Format::D32_SFLOAT: return true;
    case Format::S8_UINT: return true;
    case Format::D16_UNORM_S8_UINT: return true;
    case Format::D24_UNORM_S8_UINT: return true;
    case Format::D32_SFLOAT_S8_UINT: return true;
    default: return false;
    }
}

auto is_stencil_format(Format format) -> bool
{
    switch (format)
    {
    case Format::S8_UINT: return true;
    case Format::D16_UNORM_S8_UINT: return true;
    case Format::D24_UNORM_S8_UINT: return true;
    case Format::D32_SFLOAT_S8_UINT: return true;
    default: return false;
    }
}

auto infer_aspect_from_format(Format format) -> VkImageAspectFlags
{
    if (is_depth_format(format) || is_stencil_format(format))
    {
        return (is_depth_format(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0) | (is_stencil_format(format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
    }
    return VK_IMAGE_ASPECT_COLOR_BIT;
}

auto make_subresource_range(ImageMipArraySlice const & slice, VkImageAspectFlags aspect) -> VkImageSubresourceRange
{
    return VkImageSubresourceRange{
        .aspectMask = aspect,
        .baseMipLevel = slice.base_mip_level,
        .levelCount = slice.level_count,
        .baseArrayLayer = slice.base_array_layer,
        .layerCount = slice.layer_count,
    };
}

auto make_subresource_layers(ImageArraySlice const & slice, VkImageAspectFlags aspect) -> VkImageSubresourceLayers
{
    return VkImageSubresourceLayers{
        .aspectMask = aspect,
        .mipLevel = slice.mip_level,
        .baseArrayLayer = slice.base_array_layer,
        .layerCount = slice.layer_count,
    };
}

// --- End Helpers ---

// --- Begin API Functions ---

auto daxa_inc_refcnt(daxa_Handle handle) -> u64
{
    return std::atomic_ref{handle->strong_count}.fetch_add(1, std::memory_order::relaxed);
}

auto daxa_dec_refcnt(daxa_Handle handle) -> u64
{
    return std::atomic_ref{handle->strong_count}.fetch_sub(1, std::memory_order::relaxed);
}

// --- End API Functions ---

// --- Begin daxa_ImplHandle ---

auto daxa_ImplHandle::inc_refcnt() -> u64
{
    return std::atomic_ref{this->strong_count}.fetch_add(1, std::memory_order::relaxed);
}

auto daxa_ImplHandle::dec_refcnt(void (*zero_ref_callback)(daxa_ImplHandle *), daxa_Instance instance) -> u64
{
    auto prev = std::atomic_ref{this->strong_count}.fetch_add(1, std::memory_order::relaxed);
    if (prev == 1)
    {
        auto weak = this->get_weak_refcnt();
        if (weak == 0)
        {
            zero_ref_callback(this);
        }
        else if ((instance->info.flags & InstanceFlagBits::PARENT_MUST_OUTLIVE_CHILD) != InstanceFlagBits::NONE)
        {
            DAXA_DBG_ASSERT_TRUE_M(false, "not all children have been destroyed prior to destroying object");
        }
    }
    return prev;
}

auto daxa_ImplHandle::get_refcnt() -> u64
{
    return std::atomic_ref{this->strong_count}.load(std::memory_order::relaxed);
}

auto daxa_ImplHandle::inc_weak_refcnt() -> u64
{
    return std::atomic_ref{this->weak_count}.fetch_add(1, std::memory_order::relaxed);
}

auto daxa_ImplHandle::dec_weak_refcnt(void (*zero_ref_callback)(daxa_ImplHandle *), daxa_Instance) -> u64
{
    auto prev = std::atomic_ref{this->weak_count}.fetch_sub(1, std::memory_order::relaxed);
    if (prev == 1)
    {
        auto strong = this->get_refcnt();
        if (strong == 0)
        {
            zero_ref_callback(this);
        }
    }
    return prev;
}

auto daxa_ImplHandle::get_weak_refcnt() -> u64
{
    return std::atomic_ref{this->weak_count}.load(std::memory_order::relaxed);
}

// --- End daxa_ImplHandle ---

// --- Begin daxa_ImplMemoryBlock ---

auto daxa_dvc_create_memory(daxa_Device self, daxa_MemoryBlockInfo const * info, daxa_MemoryBlock * out_memory_block) -> daxa_Result
{
    if (info->requirements.memoryTypeBits == 0)
    {
        DAXA_DBG_ASSERT_TRUE_M(false, "memory_type_bits must be non zero");
        return DAXA_RESULT_ERROR_UNKNOWN;
    }

    VmaAllocationCreateInfo create_info{
        .flags = info->flags,
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = {}, // TODO: idk what this is...
        .preferredFlags = {},
        .memoryTypeBits = {}, // TODO: idk what this is....
        .pool = {},
        .pUserData = {},
        .priority = 0.5f,
    };
    VmaAllocation allocation = {};
    VmaAllocationInfo allocation_info = {};
    auto result = vmaAllocateMemory(self->vma_allocator, &info->requirements, &create_info, &allocation, &allocation_info);
    if (result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(result);
    }

    *out_memory_block = new daxa_ImplMemoryBlock{};
    // TODO(general): memory block is missing a name.
    (**out_memory_block).device = self;
    (**out_memory_block).info = std::bit_cast<daxa::MemoryBlockInfo>(*info);
    (**out_memory_block).allocation = allocation;
    (**out_memory_block).alloc_info = allocation_info;
    daxa_memory_block_inc_refcnt(*out_memory_block);
    self->inc_weak_refcnt();
    return DAXA_RESULT_SUCCESS;
}

auto daxa_memory_block_info(daxa_MemoryBlock self) -> daxa_MemoryBlockInfo const *
{
    return r_cast<daxa_MemoryBlockInfo const *>(&self->info);
}

auto daxa_memory_block_inc_refcnt(daxa_MemoryBlock self) -> u64
{
    return self->inc_refcnt();
}

auto daxa_memory_block_dec_refcnt(daxa_MemoryBlock self) -> u64
{
    return self->dec_refcnt(
        &daxa_ImplMemoryBlock::zero_ref_callback,
        self->device->instance);
}

void daxa_ImplMemoryBlock::zero_ref_callback(daxa_ImplHandle * handle)
{
    auto self = r_cast<daxa_MemoryBlock>(handle);
    // TODO: Does this make sense without a zombie?
    // Destruction not deferred.
    vmaFreeMemory(self->device->vma_allocator, self->allocation);
    self->device->dec_weak_refcnt(
        daxa_ImplDevice::zero_ref_callback,
        self->device->instance);
    delete self;
}

// --- End daxa_ImplMemoryBlock ---
