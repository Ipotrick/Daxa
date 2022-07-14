#pragma once

#include <daxa/pipeline.hpp>

#include "impl_device.hpp"

namespace daxa
{
    struct ImplPipelineCompiler
    {
        std::shared_ptr<ImplDevice> impl_device;
        PipelineCompilerInfo info;

        ImplPipelineCompiler(std::shared_ptr<ImplDevice> impl_device, PipelineCompilerInfo const & info);
        ~ImplPipelineCompiler();
    };

    struct ImplComputePipeline
    {
        ComputePipelineInfo info;
        VkPipeline vk_pipeline_handle = {};

        ImplComputePipeline(std::shared_ptr<ImplDevice> impl_device, ComputePipelineInfo const & info);
        ~ImplComputePipeline();
    };
} // namespace daxa
