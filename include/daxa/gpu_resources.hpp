#pragma once

#include <daxa/core.hpp>
#include <daxa/memory_block.hpp>

namespace daxa
{

    enum struct ImageViewType
    {
        REGULAR_1D = 0,
        REGULAR_2D = 1,
        REGULAR_3D = 2,
        CUBE = 3,
        REGULAR_1D_ARRAY = 4,
        REGULAR_2D_ARRAY = 5,
        CUBE_ARRAY = 6,
        MAX_ENUM = 0x7fffffff,
    };

    auto to_string(ImageViewType const & type) -> std::string_view;

    struct GPUResourceId
    {
        u32 index : 24 = {};
        u32 version : 8 = {};

        auto is_empty() const -> bool;

        constexpr auto operator<=>(GPUResourceId const & other) const
        {
            return std::bit_cast<u32>(*this) <=> std::bit_cast<u32>(other);
        }
        constexpr bool operator==(GPUResourceId const & other) const = default;
        constexpr bool operator!=(GPUResourceId const & other) const = default;
        constexpr bool operator<(GPUResourceId const & other) const = default;
        constexpr bool operator>(GPUResourceId const & other) const = default;
        constexpr bool operator<=(GPUResourceId const & other) const = default;
        constexpr bool operator>=(GPUResourceId const & other) const = default;
    };

    inline namespace types
    {
        struct BufferId : public GPUResourceId
        {
        };

        struct ImageViewId : public GPUResourceId
        {
        };

        template<ImageViewType VIEW_TYPE>
        struct TypedImageViewId : public ImageViewId
        {
            static constexpr inline auto view_type() -> ImageViewType { return VIEW_TYPE; }
        };

        struct ImageId : public GPUResourceId
        {
            auto default_view() const -> ImageViewId;
        };

        struct SamplerId : public GPUResourceId
        {
        };
    } // namespace types

    auto to_string(GPUResourceId const & id) -> std::string;

    struct BufferInfo
    {
        u32 size = {};
        AllocateInfo allocate_info = {};
        std::string name = {};
    };

    struct ImageInfo
    {
        u32 dimensions = 2;
        Format format = Format::R8G8B8A8_UNORM;
        ImageAspectFlags aspect = ImageAspectFlagBits::COLOR;
        Extent3D size = {0, 0, 0};
        u32 mip_level_count = 1;
        u32 array_layer_count = 1;
        u32 sample_count = 1;
        ImageUsageFlags usage = {};
        AllocateInfo allocate_info = {};
        std::string name = {};
    };

    struct ImageViewInfo
    {
        ImageViewType type = ImageViewType::REGULAR_2D;
        Format format = Format::R8G8B8A8_UNORM;
        ImageId image = {};
        ImageMipArraySlice slice = {};
        std::string name = {};
    };

    struct SamplerInfo
    {
        Filter magnification_filter = Filter::LINEAR;
        Filter minification_filter = Filter::LINEAR;
        Filter mipmap_filter = Filter::LINEAR;
        ReductionMode reduction_mode = ReductionMode::WEIGHTED_AVERAGE;
        SamplerAddressMode address_mode_u = SamplerAddressMode::CLAMP_TO_EDGE;
        SamplerAddressMode address_mode_v = SamplerAddressMode::CLAMP_TO_EDGE;
        SamplerAddressMode address_mode_w = SamplerAddressMode::CLAMP_TO_EDGE;
        f32 mip_lod_bias = 0.5f;
        bool enable_anisotropy = false;
        f32 max_anisotropy = 0.0f;
        bool enable_compare = false;
        CompareOp compare_op = CompareOp::ALWAYS;
        f32 min_lod = 0.0f;
        f32 max_lod = 1.0f;
        BorderColor border_color = BorderColor::FLOAT_TRANSPARENT_BLACK;
        bool enable_unnormalized_coordinates = false;
        std::string name = {};
    };
} // namespace daxa
