#pragma once

#include <daxa/pipeline.hpp>

#include "impl_device.hpp"

namespace daxa
{
    struct ImplPipelineCompiler
    {
        ImplPipelineCompiler(std::shared_ptr<ImplDevice> impl_device, PipelineCompilerInfo const & info);
        ~ImplPipelineCompiler();
    };

    struct ImplComputePipeline
    {
        VkPipeline vk_pipeline_handle = {};

        ImplComputePipeline(ComputePipelineInfo const & info);
        ~ImplComputePipeline();
    };
} // namespace daxa
