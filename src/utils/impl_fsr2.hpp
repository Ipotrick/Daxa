#pragma once

#include "../impl_core.hpp"
#include <daxa/utils/fsr2.hpp>

#include "../impl_device.hpp"

#include <ffx-fsr2-api/ffx_fsr2.h>
#include <ffx-fsr2-api/vk/ffx_fsr2_vk.h>

namespace daxa
{
    struct ImplFsr2Context final : ImplHandle
    {
        UpscaleInstanceInfo info;

        FfxFsr2Context fsr2_context = {};
        FfxFsr2ContextDescription fsr2_context_description = {};

        void * scratch_buffer = {};
        bool initialized = {};

        void resize(UpscaleSizeInfo const & resize_info);
        void upscale(CommandRecorder & command_list, UpscaleInfo const & upscale_info);

        void create_resizable_resources();
        void destroy_resizable_resources();

        ImplFsr2Context(UpscaleInstanceInfo a_info);
        ~ImplFsr2Context();

        static void zero_ref_callback(ImplHandle const *);
    };
} // namespace daxa
