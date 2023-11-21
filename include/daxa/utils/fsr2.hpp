#pragma once

#if !DAXA_BUILT_WITH_UTILS_FSR2
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_FSR2 CMake option enabled, or request the utils-fsr2 feature in vcpkg"
#endif

#include <daxa/utils/upscaling_common.hpp>

namespace daxa
{
    struct DAXA_EXPORT_CXX Fsr2Context : ManagedPtr
    {
        Fsr2Context() = default;

        Fsr2Context(UpscaleInstanceInfo const & info);
        ~Fsr2Context();

        void resize(UpscaleSizeInfo const & info);
        void upscale(CommandRecorder & command_list, UpscaleInfo const & info);
        auto get_jitter(u64 index) const -> f32vec2;
    };
} // namespace daxa
