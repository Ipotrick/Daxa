#if DAXA_BUILT_WITH_UTILS_FSR2

#include "impl_fsr2.hpp"

#include <cstring>
#include <utility>

namespace daxa
{
    Fsr2Context::Fsr2Context(UpscaleContextInfo const & info)
        : ManagedPtr{new ImplFsr2Context(info)}
    {
    }

    Fsr2Context::~Fsr2Context() = default;

    void Fsr2Context::resize(UpscaleSizeInfo const & info)
    {
        auto & impl = *as<ImplFsr2Context>();
        impl.resize(info);
    }

    void Fsr2Context::upscale(CommandList & command_list, UpscaleInfo const & info)
    {
        auto & impl = *as<ImplFsr2Context>();
        impl.upscale(command_list, info);
    }

    auto Fsr2Context::get_jitter(u64 index) const -> f32vec2
    {
        auto const & impl = *as<ImplFsr2Context>();
        f32vec2 result;
        i32 const jitter_phase_count = ffxFsr2GetJitterPhaseCount(static_cast<i32>(impl.info.size_info.render_size_x), static_cast<i32>(impl.info.size_info.display_size_x));
        ffxFsr2GetJitterOffset(&result.x, &result.y, static_cast<i32>(index), jitter_phase_count);
        return result;
    }

    ImplFsr2Context::ImplFsr2Context(UpscaleContextInfo a_info)
        : info{std::move(a_info)}
    {
    }

    ImplFsr2Context::~ImplFsr2Context() // NOLINT(bugprone-exception-escape)
    {
        destroy_resizable_resources();
    }

    void ImplFsr2Context::create_resizable_resources()
    {
        destroy_resizable_resources();
        fsr2_context_description.maxRenderSize.width = this->info.size_info.render_size_x;
        fsr2_context_description.maxRenderSize.height = this->info.size_info.render_size_y;
        fsr2_context_description.displaySize.width = this->info.size_info.display_size_x;
        fsr2_context_description.displaySize.height = this->info.size_info.display_size_y;

        // Setup VK interface.
        auto & impl_device = *this->info.device.as<ImplDevice>();
        auto * physical_device = impl_device.vk_physical_device;
        auto * logical_device = impl_device.vk_device;

        const usize scratch_buffer_size = ffxFsr2GetScratchMemorySizeVK(physical_device);
        scratch_buffer = malloc(scratch_buffer_size);
        {
            [[maybe_unused]] FfxErrorCode const err = ffxFsr2GetInterfaceVK(&fsr2_context_description.callbacks, scratch_buffer, scratch_buffer_size, physical_device, vkGetDeviceProcAddr);
            DAXA_DBG_ASSERT_TRUE_M(err == FFX_OK, "FSR2 Failed to create the Vulkan interface");
        }

        // set up for later, when we resize
        fsr2_context_description.device = ffxGetDeviceVK(logical_device);
        fsr2_context_description.flags = FFX_FSR2_ENABLE_AUTO_EXPOSURE;

        {
            [[maybe_unused]] FfxErrorCode const err = ffxFsr2ContextCreate(&fsr2_context, &fsr2_context_description);
            DAXA_DBG_ASSERT_TRUE_M(err == FFX_OK, "FSR2 Failed to create the FSR context");
        }
        initialized = true;
    }

    void ImplFsr2Context::destroy_resizable_resources()
    {
        if (initialized)
        {
            [[maybe_unused]] FfxErrorCode const err = ffxFsr2ContextDestroy(&fsr2_context);
            DAXA_DBG_ASSERT_TRUE_M(err == FFX_OK, "FSR2 Failed to destroy the FSR context");
            free(scratch_buffer);
        }
    }

    void ImplFsr2Context::resize(UpscaleSizeInfo const & resize_info)
    {
        this->info.size_info = resize_info;
        create_resizable_resources();
    }

    void ImplFsr2Context::upscale(CommandList & command_list, UpscaleInfo const & upscale_info)
    {
        auto & impl_command_list = *command_list.as<ImplCommandList>();
        auto & impl_device = *this->info.device.as<ImplDevice>();
        impl_command_list.flush_barriers();

        auto & color_slot = impl_device.slot(upscale_info.color);
        auto & color_view_slot = impl_device.slot(upscale_info.color.default_view());
        auto & depth_slot = impl_device.slot(upscale_info.depth);
        auto & depth_view_slot = impl_device.slot(upscale_info.depth.default_view());
        auto & motion_vectors_slot = impl_device.slot(upscale_info.motion_vectors);
        auto & motion_vectors_view_slot = impl_device.slot(upscale_info.motion_vectors.default_view());
        auto & output_slot = impl_device.slot(upscale_info.output);
        auto & output_view_slot = impl_device.slot(upscale_info.output.default_view());

        wchar_t fsr_inputcolor[] = L"FSR2_InputColor";
        wchar_t fsr_inputdepth[] = L"FSR2_InputDepth";
        wchar_t fsr_inputmotionvectors[] = L"FSR2_InputMotionVectors";
        wchar_t fsr_inputexposure[] = L"FSR2_InputExposure";
        wchar_t fsr_outputupscaledcolor[] = L"FSR2_OutputUpscaledColor";

        FfxFsr2DispatchDescription dispatch_description = {};
        dispatch_description.commandList = ffxGetCommandListVK(impl_command_list.vk_cmd_buffer);
        dispatch_description.color = ffxGetTextureResourceVK(
            &fsr2_context, color_slot.vk_image, color_view_slot.vk_image_view,
            this->info.size_info.render_size_x, this->info.size_info.render_size_y,
            static_cast<VkFormat>(color_slot.info.format), fsr_inputcolor);
        dispatch_description.depth = ffxGetTextureResourceVK(
            &fsr2_context, depth_slot.vk_image, depth_view_slot.vk_image_view,
            this->info.size_info.render_size_x, this->info.size_info.render_size_y,
            static_cast<VkFormat>(depth_slot.info.format), fsr_inputdepth);
        dispatch_description.motionVectors = ffxGetTextureResourceVK(
            &fsr2_context, motion_vectors_slot.vk_image, motion_vectors_view_slot.vk_image_view,
            this->info.size_info.render_size_x, this->info.size_info.render_size_y,
            static_cast<VkFormat>(motion_vectors_slot.info.format), fsr_inputmotionvectors);
        dispatch_description.exposure = ffxGetTextureResourceVK(&fsr2_context, nullptr, nullptr, 1, 1, VK_FORMAT_UNDEFINED, fsr_inputexposure);
        dispatch_description.output = ffxGetTextureResourceVK(
            &fsr2_context, output_slot.vk_image, output_view_slot.vk_image_view,
            this->info.size_info.display_size_x, this->info.size_info.display_size_x,
            static_cast<VkFormat>(output_slot.info.format), fsr_outputupscaledcolor,
            FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        dispatch_description.jitterOffset.x = upscale_info.jitter.x;
        dispatch_description.jitterOffset.y = upscale_info.jitter.y;
        dispatch_description.motionVectorScale.x = static_cast<f32>(this->info.size_info.render_size_x);
        dispatch_description.motionVectorScale.y = static_cast<f32>(this->info.size_info.render_size_y);
        dispatch_description.reset = upscale_info.should_reset;
        dispatch_description.enableSharpening = upscale_info.should_sharpen;
        dispatch_description.sharpness = upscale_info.sharpening;
        dispatch_description.frameTimeDelta = upscale_info.delta_time * 1000.0f;
        dispatch_description.preExposure = 1.0f;
        dispatch_description.renderSize.width = this->info.size_info.render_size_x;
        dispatch_description.renderSize.height = this->info.size_info.render_size_y;
        dispatch_description.cameraFar = upscale_info.camera_info.far_plane;
        dispatch_description.cameraNear = upscale_info.camera_info.near_plane;
        dispatch_description.cameraFovAngleVertical = upscale_info.camera_info.vertical_fov;

        [[maybe_unused]] FfxErrorCode const err = ffxFsr2ContextDispatch(&fsr2_context, &dispatch_description);
        DAXA_DBG_ASSERT_TRUE_M(err == FFX_OK, "FSR2 Failed dispatch");
    }
} // namespace daxa

#endif
