#include "impl_pipeline.hpp"
#include "impl_swapchain.hpp"

namespace daxa {
    ImplComputePipeline::ImplComputePipeline(ComputePipelineInfo const &info) {
        
    }

    ImplComputePipeline::~ImplComputePipeline() {
        // vkDestroyShaderModule(device, vk_shader_module, nullptr);
        // vkDestroyPipeline(device, vk_pipeline, nullptr);
    }
}