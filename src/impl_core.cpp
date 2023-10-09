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

// TODO(capi): move this to cpp wrapper!
namespace daxa
{
    ManagedPtr::ManagedPtr(daxa_Handle ptr)
        : object{ptr}
    {
        daxa_inc_refcnt(object);
    }
    ManagedPtr::~ManagedPtr()
    {
        cleanup();
    }

    ManagedPtr::ManagedPtr(ManagedPtr const & other) { *this = other; }
    ManagedPtr::ManagedPtr(ManagedPtr && other) noexcept { *this = std::move(other); }
    auto ManagedPtr::operator=(ManagedPtr const & other) -> ManagedPtr &
    {
        cleanup();
        this->object = other.object;
        if (this->object != nullptr)
        {
            daxa_inc_refcnt(this->object);
        }
        return *this;
    }
    auto ManagedPtr::operator=(ManagedPtr && other) noexcept -> ManagedPtr &
    {
        cleanup();
        std::swap(this->object, other.object);
        return *this;
    }

    void ManagedPtr::cleanup()
    {
        if (this->object != nullptr)
        {
            daxa_dec_refcnt(object);
            this->object = {};
        }
    }

    auto ManagedPtr::is_valid() const -> bool
    {
        return this->object != nullptr;
    }

    ManagedPtr::operator bool() const
    {
        return this->is_valid();
    }

    auto ImageMipArraySlice::contains(ImageMipArraySlice const & slice) const -> bool
    {
        u32 const a_mip_p0 = this->base_mip_level;
        u32 const a_mip_p1 = this->base_mip_level + this->level_count - 1;
        u32 const b_mip_p0 = slice.base_mip_level;
        u32 const b_mip_p1 = slice.base_mip_level + slice.level_count - 1;

        u32 const a_arr_p0 = this->base_array_layer;
        u32 const a_arr_p1 = this->base_array_layer + this->layer_count - 1;
        u32 const b_arr_p0 = slice.base_array_layer;
        u32 const b_arr_p1 = slice.base_array_layer + slice.layer_count - 1;

        return b_mip_p0 >= a_mip_p0 &&
               b_mip_p1 <= a_mip_p1 &&
               b_arr_p0 >= a_arr_p0 &&
               b_arr_p1 <= a_arr_p1;
    }
    auto ImageMipArraySlice::intersects(ImageMipArraySlice const & slice) const -> bool
    {
        u32 const a_mip_p0 = this->base_mip_level;
        u32 const a_mip_p1 = this->base_mip_level + this->level_count - 1;
        u32 const b_mip_p0 = slice.base_mip_level;
        u32 const b_mip_p1 = slice.base_mip_level + slice.level_count - 1;

        u32 const a_arr_p0 = this->base_array_layer;
        u32 const a_arr_p1 = this->base_array_layer + this->layer_count - 1;
        u32 const b_arr_p0 = slice.base_array_layer;
        u32 const b_arr_p1 = slice.base_array_layer + slice.layer_count - 1;

        bool const mip_disjoint = (a_mip_p1 < b_mip_p0) || (b_mip_p1 < a_mip_p0);
        bool const arr_disjoint = (a_arr_p1 < b_arr_p0) || (b_arr_p1 < a_arr_p0);

        return !mip_disjoint && !arr_disjoint;
    }
    auto ImageMipArraySlice::intersect(ImageMipArraySlice const & slice) const -> ImageMipArraySlice
    {
        u32 const a_mip_p0 = this->base_mip_level;
        u32 const a_mip_p1 = this->base_mip_level + this->level_count - 1;
        u32 const b_mip_p0 = slice.base_mip_level;
        u32 const b_mip_p1 = slice.base_mip_level + slice.level_count - 1;
        u32 const max_mip_p0 = std::max(a_mip_p0, b_mip_p0);
        u32 const min_mip_p1 = std::min(a_mip_p1, b_mip_p1);

        u32 const a_arr_p0 = this->base_array_layer;
        u32 const a_arr_p1 = this->base_array_layer + this->layer_count - 1;
        u32 const b_arr_p0 = slice.base_array_layer;
        u32 const b_arr_p1 = slice.base_array_layer + slice.layer_count - 1;
        u32 const max_arr_p0 = std::max(a_arr_p0, b_arr_p0);
        u32 const min_arr_p1 = std::min(a_arr_p1, b_arr_p1);

        // NOTE(grundlett): This multiplication at the end is to cancel out
        // the potential underflow of unsigned integers. Since the p1 could
        // could technically be less than the p0, this means that after doing
        // p1 + 1 - p0, you should get a "negative" number.
        u32 const mip_n = (min_mip_p1 + 1 - max_mip_p0) * static_cast<u32>(max_mip_p0 <= min_mip_p1);
        u32 const arr_n = (min_arr_p1 + 1 - max_arr_p0) * static_cast<u32>(max_arr_p0 <= min_arr_p1);

        return ImageMipArraySlice{
            .base_mip_level = max_mip_p0,
            .level_count = mip_n,
            .base_array_layer = max_arr_p0,
            .layer_count = arr_n,
        };
    }
    auto ImageMipArraySlice::subtract(ImageMipArraySlice const & slice) const -> std::tuple<std::array<ImageMipArraySlice, 4>, usize>
    {
        u32 const a_mip_p0 = this->base_mip_level;
        u32 const a_mip_p1 = this->base_mip_level + this->level_count - 1;
        u32 const b_mip_p0 = slice.base_mip_level;
        u32 const b_mip_p1 = slice.base_mip_level + slice.level_count - 1;

        u32 const a_arr_p0 = this->base_array_layer;
        u32 const a_arr_p1 = this->base_array_layer + this->layer_count - 1;
        u32 const b_arr_p0 = slice.base_array_layer;
        u32 const b_arr_p1 = slice.base_array_layer + slice.layer_count - 1;

        u32 const mip_case = static_cast<u32>(b_mip_p1 < a_mip_p1) + static_cast<u32>(b_mip_p0 > a_mip_p0) * 2;
        u32 const arr_case = static_cast<u32>(b_arr_p1 < a_arr_p1) + static_cast<u32>(b_arr_p0 > a_arr_p0) * 2;

        std::tuple<std::array<ImageMipArraySlice, 4>, usize> result = {};
        if (!this->intersects(slice))
        {
            auto & [result_rects, result_n] = result;
            result_n = 1;
            result_rects[0] = *this;
            return result;
        }

        // clang-format off
        //
        //     mips ➡️
        // arrays       0              1          2            3
        //  ⬇️
        //
        //           ▓▓▓▓▓▓▓▓▓▓     ▓▓▓▓             ▓▓▓▓       ▓▓  
        //  0      A ▓▓██████▓▓   B ▓▓██░░░░   C ░░░░██▓▓   D ░░██░░
        //           ▓▓██████▓▓     ▓▓██░░░░     ░░░░██▓▓     ░░██░░
        //           ▓▓██████▓▓     ▓▓██░░░░     ░░░░██▓▓     ░░██░░
        //           ▓▓▓▓▓▓▓▓▓▓     ▓▓▓▓             ▓▓▓▓       ▓▓  
        //
        //           ▓▓▓▓▓▓▓▓▓▓     ▓▓▓▓             ▓▓▓▓       ▓▓  
        //  1      E ▓▓██████▓▓   F ▓▓██░░░░   G ░░░░██▓▓   H ░░██░░
        //             ░░░░░░         ░░░░░░     ░░░░░░       ░░░░░░
        //             ░░░░░░         ░░░░░░     ░░░░░░       ░░░░░░
        //
        //  3      I   ░░░░░░     J   ░░░░░░   K ░░░░░░     L ░░░░░░
        //             ░░░░░░         ░░░░░░     ░░░░░░       ░░░░░░
        //           ▓▓██████▓▓     ▓▓██░░░░     ░░░░██▓▓     ░░██░░
        //           ▓▓▓▓▓▓▓▓▓▓     ▓▓▓▓             ▓▓▓▓       ▓▓  
        //
        //  2      M   ░░░░░░     N   ░░░░░░   O ░░░░░░     P ░░░░░░
        //           ▓▓██████▓▓     ▓▓██░░░░     ░░░░██▓▓     ░░██░░
        //             ░░░░░░         ░░░░░░     ░░░░░░       ░░░░░░
        //
        // clang-format on

        // clang-format off
        static constexpr std::array<usize, 16> rect_n {
            0, 1, 1, 2,
            1, 2, 2, 3,
            1, 2, 2, 3,
            2, 3, 3, 4,
        };

        #define NO_RBC {0, 0}
        struct RectBCIndices {
            usize mip_i;
            usize arr_i;
        };
        //   0      1      2      3      4      5
        // b1>a1  a0>b0  a0>a1  a0>b1  b0>b1  b0>a1
        static constexpr std::array<std::array<RectBCIndices, 4>, 16> bc_indices = {{
            {{NO_RBC, NO_RBC, NO_RBC, NO_RBC}},   {{{0, 2}, NO_RBC, NO_RBC, NO_RBC}},   {{{1, 2}, NO_RBC, NO_RBC, NO_RBC}},   {{{1, 2}, {0, 2}, NO_RBC, NO_RBC}},
            {{{2, 0}, NO_RBC, NO_RBC, NO_RBC}},   {{{0, 3}, {2, 0}, NO_RBC, NO_RBC}},   {{{1, 3}, {2, 0}, NO_RBC, NO_RBC}},   {{{1, 3}, {0, 3}, {2, 0}, NO_RBC}},
            {{{2, 1}, NO_RBC, NO_RBC, NO_RBC}},   {{{2, 1}, {0, 5}, NO_RBC, NO_RBC}},   {{{2, 1}, {1, 5}, NO_RBC, NO_RBC}},   {{{2, 1}, {1, 5}, {0, 5}, NO_RBC}},
            {{{2, 1}, {2, 0}, NO_RBC, NO_RBC}},   {{{2, 1}, {0, 4}, {2, 0}, NO_RBC}},   {{{2, 1}, {1, 4}, {2, 0}, NO_RBC}},   {{{2, 1}, {1, 4}, {0, 4}, {2, 0}}},
        }};
        // clang-format on

        struct BaseAndCount
        {
            u32 base;
            u32 count;
        };
        std::array<BaseAndCount, 3> const mip_bc{
            BaseAndCount{.base = b_mip_p1 + 1, .count = (a_mip_p1 + 1) - (b_mip_p1 + 1)}, // b1 -> a1
            BaseAndCount{.base = a_mip_p0, .count = b_mip_p0 - a_mip_p0},                 // a0 -> b0
            BaseAndCount{.base = a_mip_p0, .count = (a_mip_p1 + 1) - a_mip_p0},           // a0 -> a1
        };
        std::array<BaseAndCount, 6> const arr_bc{
            BaseAndCount{.base = b_arr_p1 + 1, .count = (a_arr_p1 + 1) - (b_arr_p1 + 1)}, // b1 -> a1
            BaseAndCount{.base = a_arr_p0, .count = b_arr_p0 - a_arr_p0},                 // a0 -> b0
            BaseAndCount{.base = a_arr_p0, .count = (a_arr_p1 + 1) - a_arr_p0},           // a0 -> a1
            BaseAndCount{.base = a_arr_p0, .count = (b_arr_p1 + 1) - a_arr_p0},           // a0 -> b1
            BaseAndCount{.base = b_arr_p0, .count = (b_arr_p1 + 1) - b_arr_p0},           // b0 -> b1
            BaseAndCount{.base = b_arr_p0, .count = (a_arr_p1 + 1) - b_arr_p0},           // b0 -> a1
        };

        usize const result_index = mip_case + arr_case * 4;
        usize const result_rect_n = rect_n.at(result_index);
        auto const & bc = bc_indices.at(result_index);
        std::get<1>(result) = result_rect_n;

        for (usize i = 0; i < result_rect_n; ++i)
        {
            auto & rect_i = std::get<0>(result)[i];
            auto const & bc_i = bc.at(i);
            rect_i = *this;
            rect_i.base_mip_level = mip_bc.at(bc_i.mip_i).base;
            rect_i.level_count = mip_bc.at(bc_i.mip_i).count;
            rect_i.base_array_layer = arr_bc.at(bc_i.arr_i).base;
            rect_i.layer_count = arr_bc.at(bc_i.arr_i).count;
        }

        return result;
    }

    auto ImageArraySlice::slice(ImageMipArraySlice const & mip_array_slice, u32 mip_level) -> ImageArraySlice
    {
        DAXA_DBG_ASSERT_TRUE_M(mip_level >= mip_array_slice.base_mip_level && mip_level < (mip_array_slice.base_mip_level + mip_array_slice.level_count), "slices mip level must be contained in initial slice");

        return ImageArraySlice{
            .mip_level = mip_level,
            .base_array_layer = mip_array_slice.base_array_layer,
            .layer_count = mip_array_slice.layer_count,
        };
    }

    auto ImageArraySlice::contained_in(ImageMipArraySlice const & slice) const -> bool
    {
        return this->mip_level >= slice.base_mip_level &&
               this->mip_level < (slice.base_mip_level + slice.level_count) &&
               this->base_array_layer >= slice.base_array_layer &&
               (this->base_array_layer + this->layer_count) <= (slice.base_array_layer + slice.layer_count);
    }

    auto slice(ImageArraySlice const & mip_array_slice, u32 array_layer) -> ImageSlice
    {
        DAXA_DBG_ASSERT_TRUE_M(array_layer >= mip_array_slice.base_array_layer && array_layer < (mip_array_slice.base_array_layer + mip_array_slice.layer_count), "slices array layer must be contained in initial slice");

        return ImageSlice{
            .mip_level = mip_array_slice.mip_level,
            .array_layer = array_layer,
        };
    }

    auto ImageSlice::contained_in(ImageMipArraySlice const & slice) const -> bool
    {
        return this->mip_level >= slice.base_mip_level &&
               this->mip_level < (slice.base_mip_level + slice.level_count) &&
               this->array_layer >= slice.base_array_layer &&
               array_layer < (slice.base_array_layer + slice.layer_count);
    }

    auto ImageSlice::contained_in(ImageArraySlice const & slice) const -> bool
    {
        return this->mip_level == slice.mip_level &&
               this->array_layer >= slice.base_array_layer &&
               array_layer < (slice.base_array_layer + slice.layer_count);
    }

    auto operator|(Access const & a, Access const & b) -> Access
    {
        return Access{.stages = a.stages | b.stages, .type = a.type | b.type};
    }

    auto operator&(Access const & a, Access const & b) -> Access
    {
        return Access{.stages = a.stages & b.stages, .type = a.type & b.type};
    }

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
} // namespace daxa
