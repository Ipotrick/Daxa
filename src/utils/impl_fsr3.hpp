#pragma once

#include "../impl_core.hpp"
#include <daxa/utils/fsr3.hpp>

#include "../impl_device.hpp"

#include <ffx_api/ffx_api.h>
#include <ffx_api/vk/ffx_api_vk.h>

namespace daxa
{
    struct ImplFsr3Context final : ImplHandle
    {
        Fsr3UpscaleInfo info;

        ffxContext mFsrContext = {};
        void * mScratchBuffer;

        daxa::Device mDevice;
        bool initialized = {};

        int mRenderWidth = {};
        int mRenderHeight = {};
        int mOutputWidth = {};
        int mOutputHeight = {};

        int phase_count = {};

        void create_resizable_resources(int outputSizeX, int outputSizeY, Fsr3Context::Mode mode);
        void destroy_resizable_resources();
        void evaluate(daxa::TaskInterface const & ti, Fsr3Images const & images, Fsr3UpscaleInfo const & upscale_info);

        ImplFsr3Context(daxa::Instance & instance, daxa::Device & device);
        ~ImplFsr3Context();

        static void zero_ref_callback(ImplHandle const *);
    };
} // namespace daxa
