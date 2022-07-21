#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    struct GPUResourceId
    {
        u32 index : 24;
        u32 version : 8;

        auto is_empty() const -> bool;
    };

    struct BufferInfo
    {
        MemoryFlags memory_flags = {};
        u32 size = {};
        std::string debug_name = {};
    };

    struct BufferId : public GPUResourceId
    {
    };

    struct ImageInfo
    {
        u32 dimensions = 2;
        Format format = Format::R8G8B8A8_SRGB;
        ImageAspectFlags aspect = ImageAspectFlagBits::COLOR;
        std::array<u32, 3> size = {0, 0, 0};
        u32 mip_level_count = 1;
        u32 array_layer_count = 1;
        u32 sample_count = 1;
        ImageUsageFlags usage = {};
        MemoryFlags memory_flags = {};
        std::string debug_name = {};
    };

    struct ImageId : public GPUResourceId
    {
    };

    struct ImageViewInfo
    {
        ImageViewType type = ImageViewType::REGULAR_2D;
        Format format = Format::R8G8B8A8_SRGB;
        ImageId image = {};
        ImageMipArraySlice slice = {};
        std::string debug_name = {};
    };

    struct ImageViewId : public GPUResourceId
    {
    };

    struct SamplerInfo
    {
        Filter magnification_filter = Filter::LINEAR;
        Filter minification_filter = Filter::LINEAR;
        Filter mipmap_filter = Filter::LINEAR;
        SamplerAddressMode adress_mode_u = SamplerAddressMode::CLAMP_TO_EDGE;
        SamplerAddressMode adress_mode_v = SamplerAddressMode::CLAMP_TO_EDGE;
        SamplerAddressMode adress_mode_w = SamplerAddressMode::CLAMP_TO_EDGE;
        f32 mip_lod_bias = 0.5f;
        bool enable_anisotropy = false;
        f32 max_anisotropy = 0.0f;
        bool enable_compare = false;
        CompareOp compareOp = CompareOp::ALWAYS;
        f32 min_lod = 0.0f;
        f32 max_lod = -1.0f;
        bool enable_unnormalized_coordinates = false;
        std::string debug_name = {};
    };

    struct SamplerId : public GPUResourceId
    {
    };
} // namespace daxa
