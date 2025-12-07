#pragma once

#if !DAXA_BUILT_WITH_UTILS_FSR3
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_FSR3 CMake option enabled"
#endif

#include <daxa/daxa.hpp>
#include "task_graph_types.hpp"

namespace daxa
{
    struct Fsr3Images
    {
        daxa::TaskImageView mRt;
        daxa::TaskImageView mRtBeforeFog;
        daxa::TaskImageView mTransparencyOverlay;
        daxa::TaskImageView mMotion;
        daxa::TaskImageView mDepth;
        daxa::TaskImageView mNormal;
        daxa::TaskImageView mDiffuse;
        daxa::TaskImageView mSpecular;
        daxa::TaskImageView mSpecularMv;
        daxa::TaskImageView mExposure;
        daxa::TaskImageView mOutput;
    };

    struct Fsr3UpscaleInfo
    {
        daxa_f32vec2 jitter = {};
        f32 near_plane = {};
        f32 far_plane = {};
        f32 vertical_fov = {};
        f32 pre_exposure = 1.0f;
        f32 dt = 1.0f / 60.0f;
        bool debug = {};
    };

    struct ImplFsr3Context;
    struct DAXA_EXPORT_CXX Fsr3Context : ManagedPtr<Fsr3Context, ImplFsr3Context *>
    {
        enum Mode
        {
            MODE_ULTRA_PERF,
            MODE_PERF,
            MODE_BALANCED,
            MODE_QUALITY,
            MODE_NATIVE_AA,
            MODE_COUNT,
        };
        static constexpr char const * MODE_STRINGS[MODE_COUNT] = {
            "Ultra Performance",
            "Performance",
            "Balanced",
            "Quality",
            "Native AA",
        };

        Fsr3Context() = default;

        Fsr3Context(daxa::Instance & instance, daxa::Device & device);

        void resize(int outputSizeX, int outputSizeY, Mode mode);
        void evaluate(daxa::TaskInterface const & ti, Fsr3Images const & images, Fsr3UpscaleInfo const & upscale_info);

        [[nodiscard]] auto get_jitter(u64 index) const -> daxa_f32vec2;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };
} // namespace daxa
