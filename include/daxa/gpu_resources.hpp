#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    struct GPUResourceId
    {
        u32 index : 24;
        u32 version : 8;
    };

    struct BufferInfo
    {
    };

    struct BufferId : public GPUResourceId
    {
    };

    struct ImageInfo
    {
        u32 dimensions = 2;
        Format format = Format::R8G8B8A8_SRGB;
        ImageAspectFlags aspect = ImageAspectFlagBits::COLOR;
        std::array<u32, 3> size = {0,0,0};
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
    };

    struct ImageViewId : public GPUResourceId
    {
    };

    struct SamplerInfo
    {
    };

    struct SamplerId : public GPUResourceId
    {
    };
} // namespace daxa
