#pragma once

#include <daxa/pipeline.hpp>

#include "impl_device.hpp"

namespace daxa
{
    struct ImplPipelineCompiler
    {
        ImplPipelineCompiler(PipelineCompilerInfo const & info, std::shared_ptr<ImplDevice> impl_device);
        ~ImplPipelineCompiler();
    };

    struct ImplComputePipeline
    {
        VkPipeline vk_pipeline_handle = {};

        ImplComputePipeline(ComputePipelineInfo const & info);
        ~ImplComputePipeline();
    };
} // namespace daxa
