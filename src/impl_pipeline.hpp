#pragma once

#include <daxa/pipeline.hpp>

#include "impl_core.hpp"

using namespace daxa;

static inline constexpr usize pipeline_manager_MAX_ATTACHMENTS = 16;

struct PipelineZombie
{
    VkPipeline vk_pipeline = {};
};

struct ImplPipeline : daxa_ImplHandle
{
    std::string info_name = {};
    daxa_Device device = {};
    VkPipeline vk_pipeline = {};
    VkPipelineLayout vk_pipeline_layout = {};
};

struct daxa_ImplRasterPipeline final : ImplPipeline
{
    daxa_RasterPipelineInfo info = {};

    static auto create(daxa_Device device, daxa_RasterPipelineInfo const * a_info, daxa_RasterPipeline pipeline) -> daxa_Result;
};

struct daxa_ImplComputePipeline final : ImplPipeline
{
    daxa_ComputePipelineInfo info = {};

    static auto create(daxa_Device a_device, daxa_ComputePipelineInfo const * a_info, daxa_ComputePipeline pipeline) -> daxa_Result;
};
