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
        explicit ImplPipeline(ManagedWeakPtr a_impl_device);

        ManagedWeakPtr impl_device;
        VkPipeline vk_pipeline = {};
        VkPipelineLayout vk_pipeline_layout = {};

        virtual ~ImplPipeline() override;
    };

    struct ImplRasterPipeline final : ImplPipeline
    {
        RasterPipelineInfo info;

        ImplRasterPipeline(ManagedWeakPtr a_impl_device, RasterPipelineInfo a_info);
    };

    struct ImplComputePipeline final : ImplPipeline
    {
        ComputePipelineInfo info;

        ImplComputePipeline(ManagedWeakPtr a_impl_device, ComputePipelineInfo a_info);
    };
} // namespace daxa
