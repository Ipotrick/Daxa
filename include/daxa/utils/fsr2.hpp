#pragma once

#if !DAXA_BUILT_WITH_UTILS_FSR2
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_FSR2 CMake option enabled, or request the utils-fsr2 feature in vcpkg"
#endif

#include <daxa/utils/upscaling_common.hpp>

namespace daxa
{
    struct ImplFsr2Context;
    struct DAXA_EXPORT_CXX Fsr2Context : ManagedPtr<Fsr2Context, ImplFsr2Context *>
    {
        Fsr2Context() = default;

        Fsr2Context(UpscaleInstanceInfo const & info);

        void resize(UpscaleSizeInfo const & info);
        void upscale(CommandRecorder & command_list, UpscaleInfo const & info);
        auto get_jitter(u64 index) const -> daxa_f32vec2;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };
} // namespace daxa
