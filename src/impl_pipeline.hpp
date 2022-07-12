#pragma once

#include <daxa/pipeline.hpp>

#include "impl_context.hpp"

namespace daxa
{
    struct ImplComputePipeline
    {
        VkPipeline vk_pipeline_handle = {};

        ImplComputePipeline(ComputePipelineInfo const & info);
        ~ImplComputePipeline();
    };
} // namespace daxa
