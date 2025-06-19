#pragma once

#include "../impl_core.hpp"
#include <daxa/utils/streamline.hpp>
#include "../impl_device.hpp"

namespace daxa 
{
    inline auto SL2DaxaResult(sl::Result r) -> daxa_Result
    {
        switch(r)
        {
            case sl::Result::eOk: return DAXA_RESULT_SUCCESS;
            case sl::Result::eErrorFeatureMissing: return DAXA_RESULT_ERROR_FEATURE_NOT_PRESENT;
            case sl::Result::eErrorFeatureNotSupported: return DAXA_RESULT_ERROR_FEATURE_NOT_PRESENT;
            case sl::Result::eErrorInvalidParameter: return DAXA_RESULT_INCOMPLETE;
            case sl::Result::eErrorInvalidState: return DAXA_RESULT_INCOMPLETE;
            case sl::Result::eErrorNotInitialized: return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;
            default: return DAXA_RESULT_ERROR_UNKNOWN;
        }
    }

    struct ImplStreamlineContext final : ImplHandle
    {
        daxa_Device device = {};
        bool pre_initialized = false;
        bool initialized = false;
        sl::FrameToken* token = nullptr;
        sl::ViewportHandle viewport{0};
        // default features
        std::vector<sl::Feature> features = {sl::kFeatureReflex, sl::kFeaturePCL};
        
        std::vector<const char*> required_device_extensions;
        std::vector<const char*> required_instance_extensions;
        u32 total_compute_queues_needed = 0;
        u32 total_graphics_queues_needed = 0;
        bool vsync_off_required = false;

        ImplStreamlineContext();
        ~ImplStreamlineContext();

        bool pre_initialize(sl::Feature const * sl_feats,
                            size_t sl_feat_count);
        bool initialize(daxa_Device dev);
        bool check_or_load_features();
        bool check_features_by_device(sl::AdapterInfo adapter);
        void shutdown();

        auto get_requested_graphics_queue_count() -> u32;
        auto get_requested_compute_queue_count() -> u32;
        auto get_required_instance_extensions() const -> std::vector<const char*> const &;
        auto get_required_device_extensions() const -> std::vector<const char*> const &;
        auto is_vsync_off_required() const -> bool;

        // Streamline's API
        auto set_constants(sl::Constants const & constants) -> daxa_Result;
        auto set_tag_for_frame(const sl::ResourceTag* resource_tags, u32 resource_count, VkCommandBuffer cmd) -> daxa_Result;

        // Reflex specific helpers
        auto set_reflex_options(sl::ReflexOptions const & options) -> daxa_Result;
        auto reflex_sleep(u32 frame_index) -> daxa_Result;
        auto reflex_get_state(sl::ReflexState & out_state) -> daxa_Result;
        auto reflex_set_camera_data(sl::ReflexCameraData const & camera_data) -> daxa_Result;
        auto reflex_get_predicted_camera_data(sl::ReflexPredictedCameraData & outCameraData) -> daxa_Result;

        // PCL specific helpers
        auto pcl_set_marker(sl::PCLMarker marker, u32 frame_index) -> daxa_Result;

        // DLSS specific helpers
        auto set_dlss_options(sl::DLSSOptions & options) -> daxa_Result;
        auto get_dlss_state(sl::DLSSState & out_state) -> daxa_Result;
        auto get_dlss_optimal_settings(sl::DLSSOptions const & options, sl::DLSSOptimalSettings & out_settings) -> daxa_Result;
        auto evaluate_dlss(CommandRecorder & recorder) -> daxa_Result;
        auto evaluate_dlss(std::vector<sl::ResourceTag> const & resource_tags, CommandRecorder & recorder) -> daxa_Result;
        auto free_resources_dlss(bool wait_idle) -> daxa_Result;

        // DLSS-RR specific helpers
        auto set_dlssd_options(sl::DLSSDOptions & options) -> daxa_Result;
        auto get_dlssd_state(sl::DLSSDState & out_state) -> daxa_Result;
        auto get_dlssd_optimal_settings(sl::DLSSDOptions const & options, sl::DLSSDOptimalSettings & out_settings) -> daxa_Result;
        auto evaluate_dlssd(CommandRecorder & recorder) -> daxa_Result;
        auto evaluate_dlssd(std::vector<sl::ResourceTag> const & resource_tags, CommandRecorder & recorder) -> daxa_Result;
        auto free_resources_dlssd(bool wait_idle) -> daxa_Result;

        // DLSS-G specific helpers
        auto set_dlssg_options(sl::DLSSGOptions & options) -> daxa_Result;
        auto get_dlssg_state(sl::DLSSGState & state, sl::DLSSGOptions const & options) -> daxa_Result;
        auto free_resources_dlssg(bool wait_idle) -> daxa_Result;

        // callback que llama delete this cuando refcount == 0
        static void zero_ref_callback(ImplHandle const * handle);
    };
}