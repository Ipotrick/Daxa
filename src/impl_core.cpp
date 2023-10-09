#include "impl_core.hpp"

#include "impl_instance.hpp"
#include "impl_device.hpp"

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
    self->inc_weak_refcnt();
    return DAXA_RESULT_SUCCESS;
}

auto daxa_memory_info(daxa_MemoryBlock self) -> daxa_MemoryBlockInfo const *
{
    return r_cast<daxa_MemoryBlockInfo const *>(&self->info);
}

auto daxa_memory_inc_refcnt(daxa_MemoryBlock self) -> u64
{
    return self->inc_refcnt();
}

auto daxa_memory_dec_refcnt(daxa_MemoryBlock self) -> u64
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
        self->device->instance
    );
    delete self;
}

// --- End daxa_ImplMemoryBlock ---
