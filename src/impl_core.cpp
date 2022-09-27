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

    ManagedPtr::ManagedPtr(const ManagedPtr & other) { *this = other; }
    ManagedPtr::ManagedPtr(ManagedPtr && other) { *this = std::move(other); }
    ManagedPtr & ManagedPtr::operator=(const ManagedPtr & other)
    {
        cleanup();
        this->object = other.object;
        if (this->object)
        {
            DAXA_ATOMIC_FETCH_INC(this->object->strong_count);
        }
        return *this;
    }
    ManagedPtr & ManagedPtr::operator=(ManagedPtr && other)
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
        bool const aspect_disjoint = !(this->image_aspect & slice.image_aspect);

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

        u32 const mip_case = (b_mip_p1 < a_mip_p1) + (b_mip_p0 > a_mip_p0) * 2;
        u32 const arr_case = (b_arr_p1 < a_arr_p1) + (b_arr_p0 > a_arr_p0) * 2;

        // clang-format off

        #define NULL_I {0, 0}
        struct RectBCIndices {
            usize mip_i;
            usize arr_i;
        };

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
        std::array<usize, 16> rect_n {
            0, 1, 1, 2,
            1, 2, 2, 3,
            1, 2, 2, 3,
            2, 3, 3, 4,
        };

        // ██░░  ░░██  ░░░░  █░░█
        // TODO(grundlett): X-axis were double checked (I think they're fine)
        // Y-axis ARE ALL BROKEN! ACTUALLY LEGITIMATELY KILL ME
        std::array<std::array<RectBCIndices, 4>, 16> bc_indices = {{
            {{NULL_I, NULL_I, NULL_I, NULL_I}},   {{{0, 2}, NULL_I, NULL_I, NULL_I}},   {{{1, 2}, NULL_I, NULL_I, NULL_I}},   {{{1, 2}, {0, 2}, NULL_I, NULL_I}},
            {{{2, 0}, NULL_I, NULL_I, NULL_I}},   {{{0, 1}, {2, 0}, NULL_I, NULL_I}},   {{{1, 1}, {2, 1}, NULL_I, NULL_I}},   {{{1, 1}, {0, 1}, {2, 0}, NULL_I}},
            {{{2, 1}, NULL_I, NULL_I, NULL_I}},   {{{2, 1}, {0, 0}, NULL_I, NULL_I}},   {{{2, 1}, {1, 0}, NULL_I, NULL_I}},   {{{2, 1}, {1, 0}, {0, 0}, NULL_I}},
            {{{2, 1}, {2, 0}, NULL_I, NULL_I}},   {{{2, 1}, {0, 3}, {2, 0}, NULL_I}},   {{{2, 1}, {1, 3}, {2, 0}, NULL_I}},   {{{2, 1}, {1, 3}, {0, 3}, {2, 0}}},
        }};
        // clang-format on

        std::tuple<std::array<ImageMipArraySlice, 4>, usize> result = {};
        struct BaseAndCount
        {
            u32 base;
            u32 count;
        };
        std::array<BaseAndCount, 3> mip_bc{
            BaseAndCount{.base = b_mip_p1 + 1, .count = (a_mip_p1 + 1) - (b_mip_p1 + 1)},
            BaseAndCount{.base = a_mip_p0, .count = b_mip_p0 + 1 - a_mip_p0},
            BaseAndCount{.base = a_mip_p0, .count = a_mip_p0 + 1 - a_mip_p0},
        };
        std::array<BaseAndCount, 4> arr_bc{
            BaseAndCount{.base = b_arr_p1 + 1, .count = (a_arr_p1 + 1) - (b_arr_p1 + 1)},
            BaseAndCount{.base = a_arr_p0, .count = b_arr_p0 + 1 - a_arr_p0},
            BaseAndCount{.base = a_arr_p0, .count = a_arr_p0 + 1 - a_arr_p0},
            BaseAndCount{.base = b_arr_p0 + 1, .count = b_arr_p0 + 1 - b_arr_p0},
        };

        switch (arr_case)
        {
        case 0: // A-D
            switch (mip_case)
            {
            case 0: // A
            {
                result = {{}, 0};
            }
            break;
            case 1: // B
            {
                result = {{*this, {}, {}, {}}, 1};

                std::get<0>(result)[0].base_mip_level = mip_bc[0].base;
                std::get<0>(result)[0].level_count = mip_bc[0].count;
            }
            break;
            case 2: // C
            {
                result = {{*this, {}, {}, {}}, 1};

                std::get<0>(result)[0].base_mip_level = mip_bc[1].base;
                std::get<0>(result)[0].level_count = mip_bc[1].count;
            }
            break;
            case 3: // D
            {
                result = {{*this, *this, {}, {}}, 2};

                std::get<0>(result)[0].base_mip_level = mip_bc[1].base;
                std::get<0>(result)[0].level_count = mip_bc[1].count;

                std::get<0>(result)[1].base_mip_level = mip_bc[0].base;
                std::get<0>(result)[1].level_count = mip_bc[0].count;
            }
            break;
            }
            break;
        case 1: // E-H
            switch (mip_case)
            {
            case 0: // E
            {
                result = {{*this, {}, {}, {}}, 1};

                std::get<0>(result)[0].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[0].layer_count = arr_bc[0].count;
            }
            break;
            case 1: // F
            {
                result = {{*this, *this, {}, {}}, 2};

                std::get<0>(result)[0].base_mip_level = mip_bc[0].base;
                std::get<0>(result)[0].level_count = mip_bc[0].count;
                std::get<0>(result)[0].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[0].layer_count = arr_bc[0].count;

                std::get<0>(result)[1].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[1].layer_count = arr_bc[0].count;
            }
            break;
            case 2: // G
            {
                result = {{*this, *this, {}, {}}, 2};

                std::get<0>(result)[1].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[1].layer_count = arr_bc[0].count;
            }
            break;
            case 3: // H
            {
                result = {{*this, *this, *this, {}}, 3};

                std::get<0>(result)[2].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[2].layer_count = arr_bc[0].count;
            }
            break;
            }
            break;
        case 2: // I-L
            switch (mip_case)
            {
            case 0: // I
            {
                result = {{*this, {}, {}, {}}, 1};

                std::get<0>(result)[0].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[0].layer_count = arr_bc[0].count;
            }
            break;
            case 1: // J
            {
                result = {{*this, *this, {}, {}}, 2};

                std::get<0>(result)[0].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[0].layer_count = arr_bc[0].count;

                std::get<0>(result)[1].base_mip_level = mip_bc[0].base;
                std::get<0>(result)[1].level_count = mip_bc[0].count;
            }
            break;
            case 2: // K
            {
                result = {{*this, *this, {}, {}}, 2};

                std::get<0>(result)[0].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[0].layer_count = arr_bc[0].count;
            }
            break;
            case 3: // L
            {
                result = {{*this, *this, *this, {}}, 3};

                std::get<0>(result)[0].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[0].layer_count = arr_bc[0].count;
            }
            break;
            }
            break;
        case 3: // M-P
            switch (mip_case)
            {
            case 0: // M
            {
                result = {{*this, *this, {}, {}}, 2};

                std::get<0>(result)[0].base_array_layer = arr_bc[1].base;
                std::get<0>(result)[0].layer_count = arr_bc[1].count;

                std::get<0>(result)[1].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[1].layer_count = arr_bc[0].count;
            }
            break;
            case 1: // N
            {
                result = {{*this, *this, *this, {}}, 3};

                std::get<0>(result)[0].base_array_layer = arr_bc[1].base;
                std::get<0>(result)[0].layer_count = arr_bc[1].count;

                std::get<0>(result)[1].base_mip_level = mip_bc[0].base;
                std::get<0>(result)[1].level_count = mip_bc[0].count;

                std::get<0>(result)[2].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[2].layer_count = arr_bc[0].count;
            }
            break;
            case 2: // O
            {
                result = {{*this, *this, *this, {}}, 3};

                std::get<0>(result)[0].base_array_layer = arr_bc[1].base;
                std::get<0>(result)[0].layer_count = arr_bc[1].count;

                std::get<0>(result)[2].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[2].layer_count = arr_bc[0].count;
            }
            break;
            case 3: // P
            {
                result = {{*this, *this, *this, *this}, 4};

                std::get<0>(result)[0].base_array_layer = arr_bc[1].base;
                std::get<0>(result)[0].layer_count = arr_bc[1].count;

                std::get<0>(result)[3].base_array_layer = arr_bc[0].base;
                std::get<0>(result)[3].layer_count = arr_bc[0].count;
            }
            break;
            }
            break;
        }

        return result;
    }

    auto ImageArraySlice::slice(ImageMipArraySlice const & mipArraySlice, u32 mip_level) -> ImageArraySlice
    {
        DAXA_DBG_ASSERT_TRUE_M(mip_level >= mipArraySlice.base_mip_level && mip_level < (mipArraySlice.base_mip_level + mipArraySlice.level_count), "slices mip level must be contained in initial slice");

        return ImageArraySlice{
            .image_aspect = mipArraySlice.image_aspect,
            .mip_level = mip_level,
            .base_array_layer = mipArraySlice.base_array_layer,
            .layer_count = mipArraySlice.layer_count,
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

    auto slice(ImageArraySlice const & mipArraySlice, u32 array_layer) -> ImageSlice
    {
        DAXA_DBG_ASSERT_TRUE_M(array_layer >= mipArraySlice.base_array_layer && array_layer < (mipArraySlice.base_array_layer + mipArraySlice.layer_count), "slices array layer must be contained in initial slice");

        return ImageSlice{
            .image_aspect = mipArraySlice.image_aspect,
            .mip_level = mipArraySlice.mip_level,
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
        if (this->object && DAXA_ATOMIC_FETCH_DEC(this->object->strong_count) == 1)
        {
            u64 weak_count = DAXA_ATOMIC_FETCH(this->object->weak_count);
            this->object->~ManagedSharedState();
            if (weak_count == 0)
            {
                free(this->object);
            }
            this->object = {};
        }
    }

    ManagedWeakPtr ManagedPtr::make_weak()
    {
        return ManagedWeakPtr(this->object);
    }

    ManagedWeakPtr::ManagedWeakPtr(ManagedSharedState * ptr) : object{ptr}
    {
        DAXA_ATOMIC_FETCH_INC(object->weak_count);
    }
    ManagedWeakPtr::~ManagedWeakPtr()
    {
        if (object)
        {
#if DAXA_VALIDATION
            u64 strong_count = DAXA_ATOMIC_FETCH(object->strong_count);
            DAXA_DBG_ASSERT_TRUE_M(strong_count > 0, "strong count must be greater then zero when a weak ptr is still alive!");
#endif
            DAXA_ATOMIC_FETCH_DEC(object->weak_count);
        }
    }

    ManagedWeakPtr::ManagedWeakPtr(const ManagedWeakPtr & other) { *this = other; }
    ManagedWeakPtr::ManagedWeakPtr(ManagedWeakPtr && other) { *this = std::move(other); }
    ManagedWeakPtr & ManagedWeakPtr::operator=(const ManagedWeakPtr & other)
    {
        object = other.object;
        DAXA_ATOMIC_FETCH_INC(object->weak_count);
        return *this;
    }
    ManagedWeakPtr & ManagedWeakPtr::operator=(ManagedWeakPtr && other)
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

    auto to_string(AccessTypeFlags flags) -> std::string_view
    {
        switch (flags)
        {
        case AccessTypeFlagBits::NONE: return "NONE";
        case AccessTypeFlagBits::READ: return "READ";
        case AccessTypeFlagBits::WRITE: return "WRITE";
        case AccessTypeFlagBits::READ_WRITE: return "READ_WRITE";
        default: DAXA_DBG_ASSERT_TRUE_M(false, "invalid AccessTypeFlags");
        }
        return "invalid AccessTypeFlags";
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

    auto to_string(PipelineStageFlags flags) -> std::string
    {
        if (flags == PipelineStageFlagBits::NONE)
        {
            return "NONE";
        }

        std::string ret = {};

        if (flags & PipelineStageFlagBits::TOP_OF_PIPE)
        {
            ret += "TOP_OF_PIPE";
        }
        if (flags & PipelineStageFlagBits::DRAW_INDIRECT)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "DRAW_INDIRECT";
        }
        if (flags & PipelineStageFlagBits::VERTEX_SHADER)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "VERTEX_SHADER";
        }
        if (flags & PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "TESSELLATION_CONTROL_SHADER";
        }
        if (flags & PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "TESSELLATION_EVALUATION_SHADER";
        }
        if (flags & PipelineStageFlagBits::GEOMETRY_SHADER)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "GEOMETRY_SHADER";
        }
        if (flags & PipelineStageFlagBits::FRAGMENT_SHADER)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "FRAGMENT_SHADER";
        }
        if (flags & PipelineStageFlagBits::EARLY_FRAGMENT_TESTS)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "EARLY_FRAGMENT_TESTS";
        }
        if (flags & PipelineStageFlagBits::LATE_FRAGMENT_TESTS)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "LATE_FRAGMENT_TESTS";
        }
        if (flags & PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "COLOR_ATTACHMENT_OUTPUT";
        }
        if (flags & PipelineStageFlagBits::COMPUTE_SHADER)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "COMPUTE_SHADER";
        }
        if (flags & PipelineStageFlagBits::TRANSFER)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "TRANSFER";
        }
        if (flags & PipelineStageFlagBits::BOTTOM_OF_PIPE)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "BOTTOM_OF_PIPE";
        }
        if (flags & PipelineStageFlagBits::HOST)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "HOST";
        }
        if (flags & PipelineStageFlagBits::ALL_GRAPHICS)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "ALL_GRAPHICS";
        }
        if (flags & PipelineStageFlagBits::ALL_COMMANDS)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "ALL_COMMANDS";
        }
        if (flags & PipelineStageFlagBits::COPY)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "COPY";
        }
        if (flags & PipelineStageFlagBits::RESOLVE)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "RESOLVE";
        }
        if (flags & PipelineStageFlagBits::BLIT)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "BLIT";
        }
        if (flags & PipelineStageFlagBits::CLEAR)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "CLEAR";
        }
        if (flags & PipelineStageFlagBits::INDEX_INPUT)
        {
            if (ret.size() != 0)
            {
                ret += " | ";
            }
            ret += "INDEX_INPUT";
        }
        if (flags & PipelineStageFlagBits::PRE_RASTERIZATION_SHADERS)
        {
            if (ret.size() != 0)
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
