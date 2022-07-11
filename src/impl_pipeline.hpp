#pragma once 

#include <daxa/pipeline.hpp>

#include "impl_context.hpp"

namespace daxa {
    struct ImplComputePipeline {
        VkShaderModule vk_shader_module;
        VkPipeline vk_pipeline;

        ImplComputePipeline(ComputePipelineInfo const &info);
        ~ImplComputePipeline();
    };
}
