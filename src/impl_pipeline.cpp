#include "impl_pipeline.hpp"
#include "impl_swapchain.hpp"

namespace daxa
{
    ComputePipeline::ComputePipeline(std::shared_ptr<void> a_impl) : Handle(std::move(a_impl)) {}

    PipelineCompiler::PipelineCompiler(std::shared_ptr<void> a_impl) : Handle(std::move(a_impl)) {}

    auto PipelineCompiler::create_compute_pipeline(ComputePipelineInfo const & info) -> ComputePipeline
    {
        return ComputePipeline{std::make_shared<ImplComputePipeline>(std::static_pointer_cast<ImplPipelineCompiler>(this->impl)->impl_device, info)};
    }

    ImplPipelineCompiler::ImplPipelineCompiler(std::weak_ptr<ImplDevice> impl_device, PipelineCompilerInfo const & info)
        : impl_device{impl_device}, info{info}
    {
    }

    ImplPipelineCompiler::~ImplPipelineCompiler()
    {
    }

    ImplComputePipeline::ImplComputePipeline(std::weak_ptr<ImplDevice> impl_device, ComputePipelineInfo const & info)
        : impl_device{impl_device}, info{info}
    {
        // TODO: Create a pipeline

        if (this->info.debug_name.size() > 0)
        {
            // This can only be done if the pipeline handle is valid!

            // VkDebugUtilsObjectNameInfoEXT name_info{
            //     .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            //     .pNext = nullptr,
            //     .objectType = VK_OBJECT_TYPE_PIPELINE,
            //     .objectHandle = reinterpret_cast<uint64_t>(this->vk_pipeline_handle),
            //     .pObjectName = this->info.debug_name.c_str(),
            // };
            // vkSetDebugUtilsObjectNameEXT(impl_device.lock()->vk_device_handle, &name_info);
        }
    }

    ImplComputePipeline::~ImplComputePipeline()
    {
    }
} // namespace daxa
