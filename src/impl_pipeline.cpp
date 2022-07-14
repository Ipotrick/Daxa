#include "impl_pipeline.hpp"
#include "impl_swapchain.hpp"

namespace daxa
{
    ComputePipeline::ComputePipeline(std::shared_ptr<void> impl) : Handle(impl) {}

    PipelineCompiler::PipelineCompiler(std::shared_ptr<void> impl) : Handle(impl) {}

    auto PipelineCompiler::create_compute_pipeline(ComputePipelineInfo const & info) -> ComputePipeline
    {
        return ComputePipeline{std::make_shared<ImplComputePipeline>(info)};
    }

    ImplPipelineCompiler::ImplPipelineCompiler(std::shared_ptr<ImplDevice> impl_device, PipelineCompilerInfo const & info)
    {
    }

    ImplPipelineCompiler::~ImplPipelineCompiler()
    {
    }

    ImplComputePipeline::ImplComputePipeline(ComputePipelineInfo const & info)
    {
    }

    ImplComputePipeline::~ImplComputePipeline()
    {
    }
} // namespace daxa
