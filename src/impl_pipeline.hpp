#pragma once

#include <daxa/pipeline.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    static inline constexpr usize pipeline_manager_MAX_ATTACHMENTS = 16;

    struct PipelineZombie
    {
        VkPipeline vk_pipeline = {};
    };

    struct ImplPipeline : ManagedSharedState
    {
        explicit ImplPipeline(daxa_Device a_device);

        daxa_Device device = {};
        VkPipeline vk_pipeline = {};
        VkPipelineLayout vk_pipeline_layout = {};

        virtual ~ImplPipeline() override;
    };

    struct ImplRasterPipeline final : ImplPipeline
    {
        RasterPipelineInfo info;

        ImplRasterPipeline(daxa_Device a_device, RasterPipelineInfo a_info);
    };

    struct ImplComputePipeline final : ImplPipeline
    {
        ComputePipelineInfo info;

        ImplComputePipeline(daxa_Device a_device, ComputePipelineInfo a_info);
    };
} // namespace daxa
