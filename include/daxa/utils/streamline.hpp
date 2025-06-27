#pragma once

#if !STREAMLINE_ENABLED
#error "[package management error] You must build Daxa with the STREAMLINE_ENABLED CMake option enabled"
#endif

#include <daxa/utils/upscaling_common.hpp>
#include <daxa/c/types.h>

#include <sl.h>
#include <sl_dlss.h>
#include <sl_dlss_d.h>
#include <sl_reflex.h>
#include <sl_dlss_g.h>

namespace daxa
{
    struct ImplStreamlineContext;

    struct DAXA_EXPORT_CXX StreamlineContext : ManagedPtr<StreamlineContext, ImplStreamlineContext *>
    {
        // FIXME: hide SL structures
        StreamlineContext();

        bool pre_initialize(sl::Feature const * sl_feats,
                            size_t sl_feat_count);
        bool initialize(daxa::Device dev);
        bool check_or_load_features();
        bool check_features_by_device(sl::AdapterInfo adapter);
        void shutdown();
        
        auto get_requested_graphics_queue_count() -> u32;
        auto get_requested_compute_queue_count() -> u32;
        auto get_required_instance_extensions() const -> std::vector<const char*> const &;
        auto get_required_device_extensions() const -> std::vector<const char*> const &;
        auto is_vsync_off_required() const -> bool;

        // Streamline de API
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
        auto evaluate_dlss(VkCommandBuffer & cmd) -> daxa_Result;
        auto evaluate_dlss(std::vector<sl::ResourceTag> const & resource_tags, VkCommandBuffer & cmd) -> daxa_Result;
        auto free_resources_dlss(bool wait_idle) -> daxa_Result;
        
        // DLSS-RR specific helpers
        auto set_dlssd_options(sl::DLSSDOptions & options) -> daxa_Result;
        auto get_dlssd_state(sl::DLSSDState & out_state) -> daxa_Result;
        auto get_dlssd_optimal_settings(sl::DLSSDOptions const & options, sl::DLSSDOptimalSettings & out_settings) -> daxa_Result;
        auto evaluate_dlssd(VkCommandBuffer & cmd) -> daxa_Result;
        auto evaluate_dlssd(std::vector<sl::ResourceTag> const & resource_tags, VkCommandBuffer & cmd) -> daxa_Result;
        auto free_resources_dlssd(bool wait_idle) -> daxa_Result;
        
        // DLSS-G specific helpers
        auto set_dlssg_options(sl::DLSSGOptions & options) -> daxa_Result;
        auto get_dlssg_state(sl::DLSSGState & state, sl::DLSSGOptions const & options) -> daxa_Result;
        auto free_resources_dlssg(bool wait_idle) -> daxa_Result;

      protected:
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;

        template <typename T, typename H_T>
        friend struct ManagedPtr;
    };
} // namespace daxa
