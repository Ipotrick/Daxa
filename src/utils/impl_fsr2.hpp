#pragma once

#include <daxa/utils/fsr2.hpp>
#include <deque>

#include "../impl_core.hpp"
#include "../impl_device.hpp"

#include <ffx-fsr2-api/ffx_fsr2.h>
#include <ffx-fsr2-api/vk/ffx_fsr2_vk.h>

namespace daxa
{
    struct ImplFsr2Context final : ManagedSharedState
    {
        Fsr2ContextInfo info;

        FfxFsr2Context fsr2_context = {};
        FfxFsr2ContextDescription fsr2_context_description = {};

        bool initialized = {};

        auto managed_cleanup() -> bool override;
        void resize(Fsr2SizeInfo const & info);
        void upscale(CommandList & command_list, Fsr2UpscaleInfo const & info);

        ImplFsr2Context(Fsr2ContextInfo const & info);
        virtual ~ImplFsr2Context() override final;
    };
} // namespace daxa
