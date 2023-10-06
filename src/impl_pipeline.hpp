#pragma once

#include <daxa/pipeline.hpp>

#include "impl_core.hpp"

using namespace daxa;

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
    std::string name_storage;

    ~ImplPipeline();
};

struct daxa_ImplRasterPipeline final : ImplPipeline
{
    daxa_RasterPipelineInfo info;

    daxa_ImplRasterPipeline(daxa_Device a_device, daxa_RasterPipelineInfo a_info);
};

struct daxa_ImplComputePipeline final : ImplPipeline
{
    daxa_ComputePipelineInfo info;

    daxa_ImplComputePipeline(daxa_Device a_device, daxa_ComputePipelineInfo a_info);
};
