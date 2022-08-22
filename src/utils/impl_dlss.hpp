#pragma once

#include <daxa/utils/dlss.hpp>
#include <deque>

#include "../impl_core.hpp"
#include "../impl_device.hpp"

#include <nvsdk_ngx.h>
#include <nvsdk_ngx_helpers.h>

#include <nvsdk_ngx_vk.h>
#include <nvsdk_ngx_helpers_vk.h>

namespace daxa
{
    struct ImplDlssContext final : ManagedSharedState
    {
        UpscaleContextInfo info;

        NVSDK_NGX_Parameter * ngx_parameters = nullptr;
        NVSDK_NGX_Handle * dlss_feature = nullptr;

        bool initialized = {};

        auto managed_cleanup() -> bool override;
        void resize(UpscaleSizeInfo const & info);
        void upscale(CommandList & command_list, UpscaleInfo const & info);

        void create_resizable_resources();
        void destroy_resizable_resources();

        ImplDlssContext(UpscaleContextInfo const & info);
        virtual ~ImplDlssContext() override final;
    };
} // namespace daxa
