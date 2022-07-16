#pragma once

#include <daxa/pipeline.hpp>
#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    struct ImplPipelineCompiler
    {
        std::weak_ptr<ImplDevice> impl_device;
        PipelineCompilerInfo info;

        ImplPipelineCompiler(std::weak_ptr<ImplDevice> impl_device, PipelineCompilerInfo const & info);
        ~ImplPipelineCompiler();
    };

    struct ImplComputePipeline
    {
        std::weak_ptr<ImplDevice> impl_device;
        ComputePipelineInfo info;
        VkPipeline vk_pipeline_handle = {};

        ImplComputePipeline(std::weak_ptr<ImplDevice> impl_device, ComputePipelineInfo const & info);
        ~ImplComputePipeline();
    };
} // namespace daxa
