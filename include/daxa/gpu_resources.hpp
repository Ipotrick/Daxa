#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    struct GPUResourceId
    {
        u32 index : 24;
        u32 version : 8;

        auto is_empty() const -> bool;
        auto operator<=>(GPUResourceId const & other) const = default;
    };

    inline namespace types
    {
        struct BufferId : public GPUResourceId
        {
        };

        struct ImageViewId : public GPUResourceId
        {
        };

        struct ImageId : public GPUResourceId
        {
            auto default_view() const -> ImageViewId;
        };

        struct SamplerId : public GPUResourceId
        {
        };
    } // namespace types

    auto to_string(types::ImageId image_id) -> std::string;

    auto to_string(types::BufferId buffer_id) -> std::string;

    struct BufferInfo
    {
        MemoryFlags memory_flags = {};
        u32 size = {};
        std::string debug_name = {};
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
        MemoryFlags memory_flags = {};
        std::string debug_name = {};
    };

    struct ImageViewInfo
    {
        ImageViewType type = ImageViewType::REGULAR_2D;
        Format format = Format::R8G8B8A8_UNORM;
        ImageId image = {};
        ImageMipArraySlice slice = {};
        std::string debug_name = {};
    };

    struct SamplerInfo
    {
        Filter magnification_filter = Filter::LINEAR;
        Filter minification_filter = Filter::LINEAR;
        Filter mipmap_filter = Filter::LINEAR;
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
        bool enable_unnormalized_coordinates = false;
        std::string debug_name = {};
    };
} // namespace daxa
