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
