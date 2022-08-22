#pragma once

#if !DAXA_BUILT_WITH_UTILS
#error "[package management error] You must build Daxa with the UTILS option enabled"
#endif

#include <daxa/utils/upscaling_common.hpp>

namespace daxa
{
    struct DlssContext : ManagedPtr
    {
        DlssContext(UpscaleContextInfo const & info);
        ~DlssContext();

        void resize(UpscaleSizeInfo const & info);
        void upscale(CommandList & command_list, UpscaleInfo const & info);
        auto get_jitter(u64 index) const -> f32vec2;
    };
} // namespace daxa
