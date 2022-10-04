#include "impl_core.hpp"

namespace daxa
{
    ManagedPtr::ManagedPtr(ManagedSharedState * ptr) : object{ptr}
    {
        DAXA_ATOMIC_FETCH_INC(object->strong_count);
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
            DAXA_ATOMIC_FETCH_INC(this->object->strong_count);
        }
        return *this;
    }
    auto ManagedPtr::operator=(ManagedPtr && other) noexcept -> ManagedPtr &
    {
        cleanup();
        std::swap(this->object, other.object);
        return *this;
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
               b_arr_p1 <= a_arr_p1 &&
               this->image_aspect == slice.image_aspect;
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
        bool const aspect_disjoint = !((this->image_aspect & slice.image_aspect) != ImageAspectFlagBits::NONE);

        return !mip_disjoint && !arr_disjoint && !aspect_disjoint;
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
            .image_aspect = this->image_aspect & slice.image_aspect,
            .base_mip_level = max_mip_p0,
            .level_count = mip_n,
            .base_array_layer = min_arr_p1,
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

            // TODO(grundlett): [aspect] If we want to do aspect cutting, we can
            // but we would need to look into it more.
            // result_rects[0].image_aspect &= ~slice.image_aspect;

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
        // TODO(grundlett): [aspect] listed above
        // usize const aspect_mask = ((this->image_aspect & ~slice.image_aspect) != 0);
        usize const result_rect_n = rect_n.at(result_index); // * aspect_mask
        auto const & bc = bc_indices.at(result_index);
        std::get<1>(result) = result_rect_n;

        for (usize i = 0; i < result_rect_n; ++i)
        {
            auto & rect_i = std::get<0>(result)[i];
            auto const & bc_i = bc.at(i);

            rect_i = *this;
            // TODO(grundlett): [aspect] listed above
            // rect_i.image_aspect &= ~slice.image_aspect;

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
            .image_aspect = mip_array_slice.image_aspect,
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
               (this->base_array_layer + this->layer_count) <= (slice.base_array_layer + slice.layer_count) &&
               this->image_aspect == slice.image_aspect;
    }

    auto slice(ImageArraySlice const & mip_array_slice, u32 array_layer) -> ImageSlice
    {
        DAXA_DBG_ASSERT_TRUE_M(array_layer >= mip_array_slice.base_array_layer && array_layer < (mip_array_slice.base_array_layer + mip_array_slice.layer_count), "slices array layer must be contained in initial slice");

        return ImageSlice{
            .image_aspect = mip_array_slice.image_aspect,
            .mip_level = mip_array_slice.mip_level,
            .array_layer = array_layer,
        };
    }

    auto ImageSlice::contained_in(ImageMipArraySlice const & slice) const -> bool
    {
        return this->mip_level >= slice.base_mip_level &&
               this->mip_level < (slice.base_mip_level + slice.level_count) &&
               this->array_layer >= slice.base_array_layer &&
               array_layer < (slice.base_array_layer + slice.layer_count) &&
               this->image_aspect == slice.image_aspect;
    }

    auto ImageSlice::contained_in(ImageArraySlice const & slice) const -> bool
    {
        return this->mip_level == slice.mip_level &&
               this->array_layer >= slice.base_array_layer &&
               array_layer < (slice.base_array_layer + slice.layer_count) &&
               this->image_aspect == slice.image_aspect;
    }

    void ManagedPtr::cleanup()
    {
        if ((this->object != nullptr) && DAXA_ATOMIC_FETCH_DEC(this->object->strong_count) == 1)
        {
            u64 const weak_count = DAXA_ATOMIC_FETCH(this->object->weak_count);
            this->object->~ManagedSharedState();
            if (weak_count == 0)
            {
                free(this->object);
            }
            this->object = {};
        }
    }

    auto ManagedPtr::make_weak() const -> ManagedWeakPtr
    {
        return ManagedWeakPtr(this->object);
    }

    ManagedWeakPtr::ManagedWeakPtr(ManagedSharedState * ptr) : object{ptr}
    {
        DAXA_ATOMIC_FETCH_INC(object->weak_count);
    }
    ManagedWeakPtr::~ManagedWeakPtr() // NOLINT(bugprone-exception-escape)
    {
        if (object != nullptr)
        {
#if DAXA_VALIDATION
            u64 strong_count = DAXA_ATOMIC_FETCH(object->strong_count);
            DAXA_DBG_ASSERT_TRUE_M(strong_count > 0, "strong count must be greater then zero when a weak ptr is still alive!");
#endif
            DAXA_ATOMIC_FETCH_DEC(object->weak_count);
        }
    }

    ManagedWeakPtr::ManagedWeakPtr(ManagedWeakPtr const & other) { *this = other; }
    ManagedWeakPtr::ManagedWeakPtr(ManagedWeakPtr && other) noexcept { *this = std::move(other); }
    auto ManagedWeakPtr::operator=(ManagedWeakPtr const & other) -> ManagedWeakPtr &
    {
        object = other.object;
        DAXA_ATOMIC_FETCH_INC(object->weak_count);
        return *this;
    }
    auto ManagedWeakPtr::operator=(ManagedWeakPtr && other) noexcept -> ManagedWeakPtr &
    {
        std::swap(object, other.object);
        return *this;
    }

    auto operator|(Access const & a, Access const & b) -> Access
    {
        return Access{.stages = a.stages | b.stages, .type = a.type | b.type};
    }

    auto operator&(Access const & a, Access const & b) -> Access
    {
        return Access{.stages = a.stages & b.stages, .type = a.type & b.type};
    }

    auto to_string(AccessTypeFlags flags) -> std::string
    {
        if (flags == AccessTypeFlagBits::NONE)
        {
            return "NONE ";
        }

        std::string ret = {};
        if ((flags & AccessTypeFlagBits::READ) != AccessTypeFlagBits::NONE)
        {
            // if (ret.size() != 0)
            // {
            //     ret += " | ";
            // }
            ret += "READ";
        }
        if ((flags & AccessTypeFlagBits::WRITE) != AccessTypeFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "WRITE";
        }
        return ret;
    }

    auto to_string(ImageLayout layout) -> std::string_view
    {
        switch (layout)
        {
        case ImageLayout::UNDEFINED: return "UNDEFINED";
        case ImageLayout::GENERAL: return "GENERAL";
        case ImageLayout::TRANSFER_SRC_OPTIMAL: return "TRANSFER_SRC_OPTIMAL";
        case ImageLayout::TRANSFER_DST_OPTIMAL: return "TRANSFER_DST_OPTIMAL";
        case ImageLayout::READ_ONLY_OPTIMAL: return "READ_ONLY_OPTIMAL";
        case ImageLayout::ATTACHMENT_OPTIMAL: return "ATTACHMENT_OPTIMAL";
        case ImageLayout::PRESENT_SRC: return "PRESENT_SRC";
        default: DAXA_DBG_ASSERT_TRUE_M(false, "invalid ImageLayout");
        }
        return "invalid ImageLayout";
    }

    auto to_string(ImageAspectFlags aspect_flags) -> std::string
    {
        if (aspect_flags == ImageAspectFlagBits::NONE)
        {
            return "NONE ";
        }

        std::string ret = {};

        if ((aspect_flags & ImageAspectFlagBits::COLOR) != ImageAspectFlagBits::NONE)
        {
            // if (ret.size() != 0)
            // {
            //     ret += " | ";
            // }
            ret += "COLOR";
        }
        if ((aspect_flags & ImageAspectFlagBits::DEPTH) != ImageAspectFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "DEPTH";
        }
        if ((aspect_flags & ImageAspectFlagBits::STENCIL) != ImageAspectFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "STENCIL";
        }
        if ((aspect_flags & ImageAspectFlagBits::METADATA) != ImageAspectFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "METADATA";
        }
        if ((aspect_flags & ImageAspectFlagBits::PLANE_0) != ImageAspectFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "PLANE_0";
        }
        if ((aspect_flags & ImageAspectFlagBits::PLANE_1) != ImageAspectFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "PLANE_1";
        }
        if ((aspect_flags & ImageAspectFlagBits::PLANE_2) != ImageAspectFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "PLANE_2";
        }
        return ret;
    }

    auto to_string(ImageMipArraySlice image_mip_array_slice) -> std::string
    {
        std::string ret = {};
        ret += to_string(image_mip_array_slice.image_aspect);
        ret += " base_mip_level: " + std::to_string(image_mip_array_slice.base_mip_level);
        ret += " level_count: " + std::to_string(image_mip_array_slice.level_count);
        ret += " base_array_layer: " + std::to_string(image_mip_array_slice.base_array_layer);
        ret += " layer_count: " + std::to_string(image_mip_array_slice.layer_count);
        return ret;
    }

    auto to_string(ImageArraySlice image_array_slice) -> std::string
    {
        std::string ret = {};
        ret += to_string(image_array_slice.image_aspect);
        ret += " mip_level: " + std::to_string(image_array_slice.mip_level);
        ret += " base_array_layer: " + std::to_string(image_array_slice.base_array_layer);
        ret += " layer_count: " + std::to_string(image_array_slice.layer_count);
        return ret;
    }

    auto to_string(ImageSlice image_slice) -> std::string
    {
        std::string ret = {};
        ret += to_string(image_slice.image_aspect);
        ret += " mip_level: " + std::to_string(image_slice.mip_level);
        ret += " array_layer: " + std::to_string(image_slice.array_layer);
        return ret;
    }

    auto to_string(PipelineStageFlags flags) -> std::string
    {
        if (flags == PipelineStageFlagBits::NONE)
        {
            return "NONE";
        }

        std::string ret = {};

        if ((flags & PipelineStageFlagBits::TOP_OF_PIPE) != PipelineStageFlagBits::NONE)
        {
            ret += "TOP_OF_PIPE";
        }
        if ((flags & PipelineStageFlagBits::DRAW_INDIRECT) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "DRAW_INDIRECT";
        }
        if ((flags & PipelineStageFlagBits::VERTEX_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "VERTEX_SHADER";
        }
        if ((flags & PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "TESSELLATION_CONTROL_SHADER";
        }
        if ((flags & PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "TESSELLATION_EVALUATION_SHADER";
        }
        if ((flags & PipelineStageFlagBits::GEOMETRY_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "GEOMETRY_SHADER";
        }
        if ((flags & PipelineStageFlagBits::FRAGMENT_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "FRAGMENT_SHADER";
        }
        if ((flags & PipelineStageFlagBits::EARLY_FRAGMENT_TESTS) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "EARLY_FRAGMENT_TESTS";
        }
        if ((flags & PipelineStageFlagBits::LATE_FRAGMENT_TESTS) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "LATE_FRAGMENT_TESTS";
        }
        if ((flags & PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "COLOR_ATTACHMENT_OUTPUT";
        }
        if ((flags & PipelineStageFlagBits::COMPUTE_SHADER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "COMPUTE_SHADER";
        }
        if ((flags & PipelineStageFlagBits::TRANSFER) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "TRANSFER";
        }
        if ((flags & PipelineStageFlagBits::BOTTOM_OF_PIPE) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "BOTTOM_OF_PIPE";
        }
        if ((flags & PipelineStageFlagBits::HOST) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "HOST";
        }
        if ((flags & PipelineStageFlagBits::ALL_GRAPHICS) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "ALL_GRAPHICS";
        }
        if ((flags & PipelineStageFlagBits::ALL_COMMANDS) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "ALL_COMMANDS";
        }
        if ((flags & PipelineStageFlagBits::COPY) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "COPY";
        }
        if ((flags & PipelineStageFlagBits::RESOLVE) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "RESOLVE";
        }
        if ((flags & PipelineStageFlagBits::BLIT) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "BLIT";
        }
        if ((flags & PipelineStageFlagBits::CLEAR) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "CLEAR";
        }
        if ((flags & PipelineStageFlagBits::INDEX_INPUT) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "INDEX_INPUT";
        }
        if ((flags & PipelineStageFlagBits::PRE_RASTERIZATION_SHADERS) != PipelineStageFlagBits::NONE)
        {
            if (!ret.empty())
            {
                ret += " | ";
            }
            ret += "PRE_RASTERIZATION_SHADERS";
        }
        return ret;
    }

    auto to_string(Access access) -> std::string
    {
        std::string ret = {};
        ret += "{ stages: ";
        ret += to_string(access.stages);
        ret += ", accessType: ";
        ret += to_string(access.type);
        ret += " }";
        return ret;
    }
} // namespace daxa
