#pragma once

#include <memory>

#include <daxa/gpu_resources.hpp>
#include <daxa/pipeline.hpp>

namespace daxa
{
    struct DeviceInfo
    {
    };

    struct Device : public Handle
    {
        auto create_buffer(BufferInfo const & info) -> BufferId;
        auto create_image(ImageInfo const & info) -> ImageId;
        auto create_image_view(ImageViewInfo const & info) -> ImageViewId;
        auto create_sampler(SamplerInfo const & info) -> SamplerId;

        void destroy_buffer(BufferId id);
        void destroy_image(ImageId id);
        void destroy_image(ImageViewId id);
        void destroy_sampler(SamplerId id);

        auto info_buffer(BufferId id) const -> BufferInfo;
        auto info_image(ImageId id) const -> ImageInfo;
        auto info_image_view(ImageViewId id) const -> ImageViewInfo;
        auto info_sampler(SamplerId id) const -> SamplerInfo;

        auto create_pipeline_compiler(PipelineCompilerInfo const & info) -> PipelineCompiler;
        // auto create_swapchain(SwapchainInfo const & info) -> Swapchain;
        // auto create_semaphore(SemaphoreInfo const & info) -> Semaphore;
        // auto create_signal(SignalInfo const & info) -> Signal;

        void wait_idle();
        auto info() const -> DeviceInfo;

        void submit_commands();
        void present_frame();
        void end_command_batch();

      private:
        Device(std::shared_ptr<void> impl);
    };
} // namespace daxa
