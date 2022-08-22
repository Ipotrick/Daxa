#if DAXA_BUILT_WITH_UTILS

#include "impl_dlss.hpp"

#include <cstring>
#include <locale>
#include <codecvt>

std::string ws2s(const std::wstring & wstr)
{
    std::string result;
    result.resize(wstr.size());
    for (daxa::usize i = 0; i < wstr.size(); ++i)
        result[i] = static_cast<char>(wstr[i]);
    return result;
}

namespace daxa
{
    DlssContext::DlssContext(UpscaleContextInfo const & info)
        : ManagedPtr{new ImplDlssContext(info)}
    {
    }

    DlssContext::~DlssContext() {}

    auto ImplDlssContext::managed_cleanup() -> bool
    {
        return true;
    }

    void DlssContext::resize(UpscaleSizeInfo const & info)
    {
        auto & impl = *as<ImplDlssContext>();
        impl.resize(info);
    }

    void DlssContext::upscale(CommandList & command_list, UpscaleInfo const & info)
    {
        auto & impl = *as<ImplDlssContext>();
        impl.upscale(command_list, info);
    }

    auto DlssContext::get_jitter(u64 index) const -> f32vec2
    {
        auto & impl = *as<ImplDlssContext>();
        f32vec2 result = {0, 0};

        // TODO: implement halton sequence jitter

        return result;
    }

    ImplDlssContext::ImplDlssContext(UpscaleContextInfo const & info)
        : info{info}
    {
        NVSDK_NGX_Result result;

        auto & impl_device = *this->info.device.as<ImplDevice>();
        auto vkdevice = impl_device.vk_device;
        auto vkphysicaldevice = impl_device.vk_physical_device;
        auto vkinstance = impl_device.impl_ctx.as<ImplContext>()->vk_instance;

        result = NVSDK_NGX_VULKAN_Init(231313132, L".", vkinstance, vkphysicaldevice, vkdevice);
        if (NVSDK_NGX_FAILED(result))
        {
            std::string err_msg;
            if (result == NVSDK_NGX_Result_FAIL_FeatureNotSupported || result == NVSDK_NGX_Result_FAIL_PlatformError)
            {
                err_msg = "NVIDIA NGX not available on this hardware/platform, code = " + std::to_string(result) + ", info: " + ws2s(GetNGXResultAsString(result));
                DAXA_DBG_ASSERT_TRUE_M(false, err_msg);
            }
            else
            {
                err_msg = "Failed to initialize NVIDIA NGX, code = " + std::to_string(result) + ", info: " + ws2s(GetNGXResultAsString(result));
                DAXA_DBG_ASSERT_TRUE_M(false, err_msg);
            }
        }

        result = NVSDK_NGX_VULKAN_GetCapabilityParameters(&ngx_parameters);
        DAXA_DBG_ASSERT_TRUE_M(!NVSDK_NGX_FAILED(result), "Failed to get NGX capability parameters");
    }

    ImplDlssContext::~ImplDlssContext()
    {
        destroy_resizable_resources();
    }

    void ImplDlssContext::create_resizable_resources()
    {
        destroy_resizable_resources();

        unsigned int creation_node_mask = 1;
        unsigned int visibility_node_mask = 1;
        NVSDK_NGX_Result result = NVSDK_NGX_Result_Fail;

        int dlss_create_feature_flags = NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
        NVSDK_NGX_DLSS_Create_Params dlss_create_params{
            .Feature = {
                .InWidth = info.size_info.render_size_x,
                .InHeight = info.size_info.render_size_y,
                .InTargetWidth = info.size_info.display_size_x,
                .InTargetHeight = info.size_info.display_size_y,
            },
            .InFeatureCreateFlags = dlss_create_feature_flags,
        };

        auto cmd_list = info.device.create_command_list({.debug_name = "DLSS features command list"});
        auto & impl_cmd_list = *cmd_list.as<ImplCommandList>();
        result = NGX_VULKAN_CREATE_DLSS_EXT(impl_cmd_list.vk_cmd_buffer, creation_node_mask, visibility_node_mask, &dlss_feature, ngx_parameters, &dlss_create_params);
        cmd_list.complete();

        if (NVSDK_NGX_FAILED(result))
        {
            std::string err_msg = "Failed to create NVIDIA NGX features, code = " + std::to_string(result) + ", info: " + ws2s(GetNGXResultAsString(result));
            DAXA_DBG_ASSERT_TRUE_M(false, err_msg);
        }

        info.device.submit_commands({
            .command_lists = {cmd_list},
        });
        info.device.wait_idle();
        initialized = true;
    }

    void ImplDlssContext::destroy_resizable_resources()
    {
        if (initialized)
        {
            NVSDK_NGX_Result result = (dlss_feature != nullptr) ? NVSDK_NGX_VULKAN_ReleaseFeature(dlss_feature) : NVSDK_NGX_Result_Success;
            if (NVSDK_NGX_FAILED(result))
            {
                std::string err_msg = "Failed to release NVIDIA NGX features, code = " + std::to_string(result) + ", info: " + ws2s(GetNGXResultAsString(result));
                DAXA_DBG_ASSERT_TRUE_M(false, err_msg);
            }
        }
    }

    void ImplDlssContext::resize(UpscaleSizeInfo const & info)
    {
        this->info.size_info = info;
        create_resizable_resources();
    }

    void ImplDlssContext::upscale(CommandList & command_list, UpscaleInfo const & info)
    {
        auto & impl_command_list = *command_list.as<ImplCommandList>();
        auto & impl_device = *this->info.device.as<ImplDevice>();
        impl_command_list.flush_barriers();

        auto get_nvsdk_texture_resource = [&](ImageId image)
        {
            auto & image_slot = impl_device.slot(image);
            auto & image_view_slot = impl_device.slot(image.default_view());
            return NVSDK_NGX_Create_ImageView_Resource_VK(
                image_view_slot.vk_image_view,
                image_slot.vk_image,
                {
                    image_view_slot.info.slice.image_aspect,
                    image_view_slot.info.slice.base_mip_level,
                    image_view_slot.info.slice.level_count,
                    image_view_slot.info.slice.base_array_layer,
                    image_view_slot.info.slice.layer_count,
                },
                static_cast<VkFormat>(image_slot.info.format),
                image_slot.info.size[0],
                image_slot.info.size[1],
                false);
        };

        NVSDK_NGX_Resource_VK unresolved_color_resource = get_nvsdk_texture_resource(info.color);
        NVSDK_NGX_Resource_VK resolved_color_resource = get_nvsdk_texture_resource(info.output);
        NVSDK_NGX_Resource_VK motion_vectors_resource = get_nvsdk_texture_resource(info.motion_vectors);
        NVSDK_NGX_Resource_VK depth_resource = get_nvsdk_texture_resource(info.depth);

        NVSDK_NGX_Result result;

        NVSDK_NGX_VK_DLSS_Eval_Params vk_dlss_eval_params = {
            .Feature = {
                .pInColor = &unresolved_color_resource,
                .pInOutput = &resolved_color_resource,
                .InSharpness = info.sharpening,
            },
            .pInDepth = &depth_resource,
            .pInMotionVectors = &motion_vectors_resource,
            .InJitterOffsetX = info.jitter.x,
            .InJitterOffsetY = info.jitter.y,
            .InRenderSubrectDimensions = {this->info.size_info.render_size_x, this->info.size_info.render_size_y},
            .InReset = info.should_reset,
            .InMVScaleX = static_cast<f32>(this->info.size_info.render_size_x),
            .InMVScaleY = static_cast<f32>(this->info.size_info.render_size_y),
            .InColorSubrectBase = {0, 0},
            .InDepthSubrectBase = {0, 0},
            .InMVSubrectBase = {0, 0},
            .InTranslucencySubrectBase = {0, 0},
        };

        result = NGX_VULKAN_EVALUATE_DLSS_EXT(impl_command_list.vk_cmd_buffer, dlss_feature, ngx_parameters, &vk_dlss_eval_params);

        if (NVSDK_NGX_FAILED(result))
        {
            std::string err_msg = "Failed to initialize NVIDIA NGX, code = " + std::to_string(result) + ", info: " + ws2s(GetNGXResultAsString(result));
            DAXA_DBG_ASSERT_TRUE_M(false, err_msg);
        }
    }
} // namespace daxa

#endif
