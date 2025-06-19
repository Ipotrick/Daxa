#ifdef STREAMLINE_ENABLED
#include "impl_streamline.hpp"
#include <daxa/c/command_recorder.h>
namespace daxa
{
    StreamlineContext::StreamlineContext()
    {
        this->object = new ImplStreamlineContext();
    }

    bool StreamlineContext::pre_initialize(sl::Feature const * sl_feats,
                                           size_t sl_feat_count)
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.pre_initialize(sl_feats, sl_feat_count);
    }
    
    auto StreamlineContext::get_requested_graphics_queue_count() -> u32  
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.get_requested_graphics_queue_count();
    }
    
    auto StreamlineContext::get_requested_compute_queue_count() -> u32  
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.get_requested_compute_queue_count();
    }

    auto StreamlineContext::get_required_instance_extensions() const -> std::vector<char const *> const &
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.required_instance_extensions;
    }

    auto StreamlineContext::get_required_device_extensions() const -> std::vector<char const *> const &
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.required_device_extensions;
    }

    auto StreamlineContext::is_vsync_off_required() const -> bool
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.vsync_off_required;
    }

    bool StreamlineContext::initialize(daxa::Device device) 
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.initialize(*r_cast<daxa_Device *>(&device));
    }
    
    bool StreamlineContext::check_or_load_features()
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.check_or_load_features();
    }

    bool StreamlineContext::check_features_by_device(sl::AdapterInfo adapter)
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.check_features_by_device(adapter);
    }

    auto StreamlineContext::set_constants(sl::Constants const & constants) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.set_constants(constants);
    }

    auto StreamlineContext::set_tag_for_frame(const sl::ResourceTag* resource_tags, u32 resource_count, VkCommandBuffer cmd) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.set_tag_for_frame(resource_tags, resource_count, cmd);
    }
    
    void StreamlineContext::shutdown()
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        impl.shutdown();
    }

    // ——————————————————————————————————————————————————————————
    //  Reflex methods
    // ——————————————————————————————————————————————————————————

    auto StreamlineContext::set_reflex_options(sl::ReflexOptions const & options) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.set_reflex_options(options);
    }

    auto StreamlineContext::reflex_sleep(u32 frame_index) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.reflex_sleep(frame_index);
    }

    auto StreamlineContext::reflex_get_state(sl::ReflexState & out_state) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.reflex_get_state(out_state);
    }

    auto StreamlineContext::reflex_set_camera_data(sl::ReflexCameraData const & camera_data) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.reflex_set_camera_data(camera_data);
    }
    
    auto StreamlineContext::reflex_get_predicted_camera_data(sl::ReflexPredictedCameraData & outCameraData) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.reflex_get_predicted_camera_data(outCameraData);
    }

    // ——————————————————————————————————————————————————————————
    //  PCL methods
    // ——————————————————————————————————————————————————————————
    auto StreamlineContext::pcl_set_marker(sl::PCLMarker marker, u32 frame_index) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.pcl_set_marker(marker, frame_index);
    }

    // ——————————————————————————————————————————————————————————
    //  DLSS (Deep Learning Super Sampling Resolution) methods
    // ——————————————————————————————————————————————————————————

    auto StreamlineContext::set_dlss_options(sl::DLSSOptions & options) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.set_dlss_options(options);
    }
    
    auto StreamlineContext::get_dlss_state(sl::DLSSState & out_state) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.get_dlss_state(out_state);
    }

    auto StreamlineContext::get_dlss_optimal_settings(sl::DLSSOptions const & options, sl::DLSSOptimalSettings & out_settings) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.get_dlss_optimal_settings(options, out_settings);
    }

    auto StreamlineContext::evaluate_dlss(CommandRecorder & recorder) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.evaluate_dlss(recorder);
    }

    auto StreamlineContext::evaluate_dlss(std::vector<sl::ResourceTag> const & resource_tags, CommandRecorder & recorder) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.evaluate_dlss(resource_tags, recorder);
    }

    auto StreamlineContext::free_resources_dlss(bool wait_idle) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.free_resources_dlss(wait_idle);
    }

    // ——————————————————————————————————————————————————————————
    //  DLSSD (Deep Learning Super Sampling Resolution + Denoising) methods
    // ——————————————————————————————————————————————————————————

    auto StreamlineContext::set_dlssd_options(sl::DLSSDOptions & options) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.set_dlssd_options(options);
    }
    
    auto StreamlineContext::get_dlssd_state(sl::DLSSDState & out_state) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.get_dlssd_state(out_state);
    }

    auto StreamlineContext::get_dlssd_optimal_settings(sl::DLSSDOptions const & options, sl::DLSSDOptimalSettings & out_settings) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.get_dlssd_optimal_settings(options, out_settings);
    }

    auto StreamlineContext::evaluate_dlssd(CommandRecorder & recorder) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.evaluate_dlssd(recorder);
    }

    auto StreamlineContext::evaluate_dlssd(std::vector<sl::ResourceTag> const & resource_tags, CommandRecorder & recorder) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.evaluate_dlssd(resource_tags, recorder);
    }

    auto StreamlineContext::free_resources_dlssd(bool wait_idle) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.free_resources_dlssd(wait_idle);
    }

    auto StreamlineContext::free_resources_dlssg(bool wait_idle)  -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.free_resources_dlssg(wait_idle);
    }
    
    auto StreamlineContext::set_dlssg_options(sl::DLSSGOptions & options) -> daxa_Result 
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.set_dlssg_options(options);
    }
    
    auto StreamlineContext::get_dlssg_state(sl::DLSSGState & state, sl::DLSSGOptions const & options) -> daxa_Result
    {
        auto & impl = *r_cast<ImplStreamlineContext *>(this->object);
        return impl.get_dlssg_state(state, options);
    }

    auto StreamlineContext::inc_refcnt(ImplHandle const * object) -> u64
    {
        return object->inc_refcnt();
    }

    auto StreamlineContext::dec_refcnt(ImplHandle const * object) -> u64
    {
        return object->dec_refcnt(
            ImplStreamlineContext::zero_ref_callback,
            nullptr);
    }

    ImplStreamlineContext::ImplStreamlineContext()
    {
    }

    ImplStreamlineContext::~ImplStreamlineContext()
    {
        shutdown();
    }

    bool ImplStreamlineContext::pre_initialize(sl::Feature const * sl_feats,
                                               size_t sl_feat_count)
    {
        if (sl_feat_count == 0)
            return false;

        // copy features
        for (size_t i = 0; i < sl_feat_count; ++i)
        {
            sl::Feature f = sl_feats[i];
            if (std::find(features.begin(), features.end(), f) == features.end())
                features.push_back(f);
        }

        // Set SL preferences
        sl::Preferences sl_pref{};
        sl_pref.showConsole = false;
        sl_pref.logLevel = sl::LogLevel::eVerbose;
        sl_pref.renderAPI = sl::RenderAPI::eVulkan;
        sl_pref.flags |= sl::PreferenceFlags::eAllowOTA | sl::PreferenceFlags::eLoadDownloadedPlugins | sl::PreferenceFlags::eUseManualHooking;
#ifdef STREAMLINE_ENABLED_FOR_FRAME
        sl_pref.flags |= sl::PreferenceFlags::eUseFrameBasedResourceTagging;
#endif
        sl_pref.engine = sl::EngineType::eCustom;
        sl_pref.engineVersion = 0;
        sl_pref.applicationId = 231313132;

        sl_pref.featuresToLoad = features.data();
        sl_pref.numFeaturesToLoad = static_cast<u32>(features.size());

        sl::Result r = slInit(sl_pref);
        pre_initialized = (r == sl::Result::eOk);

        if(!pre_initialized) 
            return pre_initialized;

        required_device_extensions.clear();
        required_instance_extensions.clear();
        total_compute_queues_needed = 0;
        total_graphics_queues_needed = 0;
        vsync_off_required = false;

        for (const sl::Feature feature : features)
        {
            sl::FeatureRequirements requirements{};
            sl::Result res = slGetFeatureRequirements(feature, requirements);
            
            if (res != sl::Result::eOk)
            {
                // std::cerr << "Streamline: Failed to get feature requirements for feature " 
                //             << static_cast<u32>(feature) << " (error: " << static_cast<int>(res) << ")" << std::endl;
                return false;
            }

            // Check if feature is supported on Vulkan
            if ((requirements.flags & sl::FeatureRequirementFlags::eVulkanSupported) == 0)
            {
                // std::cerr << "Streamline: Feature " << static_cast<u32>(feature) 
                //             << " is not supported on Vulkan" << std::endl;
                return false;
            }

            // Check if VSync needs to be disabled
            if ((requirements.flags & sl::FeatureRequirementFlags::eVSyncOffRequired) != 0)
            {
                vsync_off_required = true;
                // std::cout << "Streamline: Feature " << static_cast<u32>(feature) 
                //             << " requires VSync to be disabled" << std::endl;
            }

            // Accumulate queue requirements
            if (requirements.vkNumComputeQueuesRequired != 0)
            {
                total_compute_queues_needed += requirements.vkNumComputeQueuesRequired;
                // std::cout << "Streamline: Feature " << static_cast<u32>(feature) 
                //             << " requires " << requirements.vkNumComputeQueuesRequired 
                //             << " compute queue(s)" << std::endl;
            }
            
            if (requirements.vkNumGraphicsQueuesRequired != 0)
            {
                total_graphics_queues_needed += requirements.vkNumGraphicsQueuesRequired;
                // std::cout << "Streamline: Feature " << static_cast<u32>(feature) 
                //             << " requires " << requirements.vkNumGraphicsQueuesRequired 
                //             << " graphics queue(s)" << std::endl;
            }

            // Skip optical flow queues for now as requested
            if (requirements.vkNumOpticalFlowQueuesRequired != 0)
            {
                // std::cout << "Streamline: Warning - Feature " << static_cast<u32>(feature) 
                //             << " requires optical flow queues which are not currently supported" << std::endl;
            }

            // Collect required device extensions
            for (u32 i = 0; i < requirements.vkNumDeviceExtensions; i++)
            {
                const char* ext_name = requirements.vkDeviceExtensions[i];

                // Add extension if not already in list
                if (std::find(required_device_extensions.begin(), 
                                required_device_extensions.end(), 
                                ext_name) == required_device_extensions.end())
                {
                    required_device_extensions.push_back(ext_name);
                    // std::cout << "Streamline: Feature " << static_cast<u32>(feature) 
                    //             << " requires device extension: " << ext_name << std::endl;
                }
            }

            // Collect required instance extensions
            for (u32 i = 0; i < requirements.vkNumInstanceExtensions; i++)
            {
                const char* ext_name = requirements.vkInstanceExtensions[i];
                
                // Add extension if not already in list
                if (std::find(required_instance_extensions.begin(), 
                                required_instance_extensions.end(), 
                                ext_name) == required_instance_extensions.end())
                {
                    required_instance_extensions.push_back(ext_name);
                    // std::cout << "Streamline: Feature " << static_cast<u32>(feature) 
                    //             << " requires instance extension: " << ext_name << std::endl;
                }
            }
        }

        return true;
    }
    
    bool ImplStreamlineContext::initialize(daxa_Device dev) {
        if(!pre_initialized)
            return false;
        device = dev;
        return initialized = true;
    }

    auto ImplStreamlineContext::get_requested_graphics_queue_count() -> u32
    {
        return total_graphics_queues_needed;
    }

    auto ImplStreamlineContext::get_requested_compute_queue_count() -> u32
    {
        return total_compute_queues_needed;
    }

    bool ImplStreamlineContext::check_or_load_features()
    {
        for (const auto& feature : features)
        {   
            bool loaded = false;
            if(SL_FAILED(sl_result, slIsFeatureLoaded(feature, loaded)) || !loaded) 
            {
                if(SL_FAILED(sl_result2, slSetFeatureLoaded(feature, true))) 
                {
                    return false;
                }
            } 
        }
        return true;
    }

    bool ImplStreamlineContext::check_features_by_device(sl::AdapterInfo adapter)
    {
        for (const auto& feature : features)
        {
            if(SL_FAILED(res, slIsFeatureSupported(feature, adapter)))
            {
                return false;
            } 
        }
        return true;
    }

    auto ImplStreamlineContext::set_constants(sl::Constants const & constants) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!token)
            return DAXA_RESULT_INCOMPLETE;

        return SL2DaxaResult(slSetConstants(constants, *token, viewport));
    }

    auto ImplStreamlineContext::set_tag_for_frame(const sl::ResourceTag* resource_tags, u32 resource_count, VkCommandBuffer cmd) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!token)
            return DAXA_RESULT_INCOMPLETE;

#ifdef STREAMLINE_ENABLED_FOR_FRAME        
        return SL2DaxaResult(slSetTagForFrame(*token, viewport, resource_tags,
                                              resource_count,
                                              cmd));
#else 
        return SL2DaxaResult(slSetTag(viewport, resource_tags,
                                              resource_count,
                                              cmd));
#endif // STREAMLINE_ENABLED_FOR_FRAME
    }

    void ImplStreamlineContext::shutdown()
    {
        if (pre_initialized)
        {
            slShutdown();
            pre_initialized = false;
            initialized = false;
        }
    }

    // ——————————————————————————————————————————————————————————
    //  Reflex methods
    // ——————————————————————————————————————————————————————————

    auto ImplStreamlineContext::set_reflex_options(sl::ReflexOptions const & options) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        return SL2DaxaResult(slReflexSetOptions(options));
    }

    auto ImplStreamlineContext::reflex_sleep(u32 frame_index) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;
        
        if (SL_FAILED(result, slGetNewFrameToken(token, &frame_index)))
            return SL2DaxaResult(result);

        return SL2DaxaResult(slReflexSleep(*token));
    }

    auto ImplStreamlineContext::reflex_get_state(sl::ReflexState & out_state) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        return SL2DaxaResult(slReflexGetState(out_state));
    }

    auto ImplStreamlineContext::reflex_set_camera_data(sl::ReflexCameraData const & camera_data) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!token)
            return DAXA_RESULT_INCOMPLETE;

        return SL2DaxaResult(slReflexSetCameraData(viewport, *token, camera_data));
    }

    auto ImplStreamlineContext::reflex_get_predicted_camera_data(sl::ReflexPredictedCameraData & outCameraData) -> daxa_Result
    {
        if (!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!token)
            return DAXA_RESULT_INCOMPLETE;

        return SL2DaxaResult(slReflexGetPredictedCameraData(viewport, *token, outCameraData));
    }

    // ——————————————————————————————————————————————————————————
    //  PCL methods
    // ——————————————————————————————————————————————————————————
    auto ImplStreamlineContext::pcl_set_marker(sl::PCLMarker marker, u32 frame_index) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        sl::FrameToken* temp;
        if (SL_FAILED(result, slGetNewFrameToken(temp, &frame_index)))
        {
            return SL2DaxaResult(result);
        }
        return SL2DaxaResult(slPCLSetMarker(marker, *temp));
    }

    // ——————————————————————————————————————————————————————————
    //  DLSS (Deep Learning Super Sampling Resolution) methods
    // ——————————————————————————————————————————————————————————

    auto ImplStreamlineContext::set_dlss_options(sl::DLSSOptions & options) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
        {
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;
        }

        if (SL_FAILED(result, slDLSSSetOptions(viewport, options)))
        {
            return SL2DaxaResult(result);
        }

        return DAXA_RESULT_SUCCESS;
    }

    auto ImplStreamlineContext::get_dlss_state(sl::DLSSState & out_state) -> daxa_Result
    {
        if (!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        return SL2DaxaResult(slDLSSGetState(viewport, out_state));
    }

    auto ImplStreamlineContext::get_dlss_optimal_settings(sl::DLSSOptions const & options, sl::DLSSOptimalSettings & out_settings) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
        {
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;
        }

        if (SL_FAILED(result, slDLSSSetOptions(viewport, options)))
        {
            return SL2DaxaResult(result);
        }

        if (SL_FAILED(resul2, slDLSSGetOptimalSettings(options, out_settings)))
        {
            return SL2DaxaResult(resul2);
        }

        return DAXA_RESULT_SUCCESS;
    }

    auto ImplStreamlineContext::evaluate_dlss(CommandRecorder & recorder) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!token)
            return DAXA_RESULT_INCOMPLETE;

        sl::ViewportHandle view(viewport);
        const sl::BaseStructure * inputs[] = {&view};
        return SL2DaxaResult(slEvaluateFeature(sl::kFeatureDLSS, *token, inputs, static_cast<uint32_t>(std::size(inputs)),
                                               daxa_cmd_get_vk_command_buffer(*reinterpret_cast<daxa_CommandRecorder *>(&recorder))));
    }

    auto ImplStreamlineContext::evaluate_dlss(std::vector<sl::ResourceTag> const & resource_tags, CommandRecorder & recorder) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!token)
            return DAXA_RESULT_INCOMPLETE;
    
        std::vector<const sl::BaseStructure*> inputs;
        sl::ViewportHandle view(viewport);
        inputs.push_back(&view);
        
        for (auto& tag : resource_tags)
        {
            inputs.push_back(&tag);
        }
        
        return SL2DaxaResult(slEvaluateFeature(sl::kFeatureDLSS, *token, 
                                            inputs.data(), 
                                            static_cast<uint32_t>(inputs.size()),
                                            daxa_cmd_get_vk_command_buffer(*reinterpret_cast<daxa_CommandRecorder *>(&recorder))));
    }

    auto ImplStreamlineContext::free_resources_dlss(bool wait_idle) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;
        
        if(wait_idle) {
            auto result = daxa_dvc_wait_idle(device);
            if(result != DAXA_RESULT_SUCCESS)
                return result;
        }

        return SL2DaxaResult(slFreeResources(sl::kFeatureDLSS, viewport));
    }

    // ——————————————————————————————————————————————————————————
    //  DLSSD (Deep Learning Super Sampling Resolution + Denoising) methods
    // ——————————————————————————————————————————————————————————

    auto ImplStreamlineContext::set_dlssd_options(sl::DLSSDOptions & options) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (SL_FAILED(result, slDLSSDSetOptions(viewport, options)))
        {
            return SL2DaxaResult(result);
        }
        return DAXA_RESULT_SUCCESS;
    }
    
    auto ImplStreamlineContext::get_dlssd_state(sl::DLSSDState & out_state) -> daxa_Result
    {
        if (!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        return SL2DaxaResult(slDLSSDGetState(viewport, out_state));
    }

    auto ImplStreamlineContext::get_dlssd_optimal_settings(sl::DLSSDOptions const & options, sl::DLSSDOptimalSettings & out_settings) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (SL_FAILED(result, slDLSSDSetOptions(viewport, const_cast<sl::DLSSDOptions&>(options))))
        {
            return SL2DaxaResult(result);
        }

        if (SL_FAILED(result2, slDLSSDGetOptimalSettings(options, out_settings)))
        {
            return SL2DaxaResult(result2);
        }
        return DAXA_RESULT_SUCCESS;
    }

    auto ImplStreamlineContext::evaluate_dlssd(CommandRecorder & recorder) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!token)
            return DAXA_RESULT_INCOMPLETE;

        sl::ViewportHandle view(viewport);
        const sl::BaseStructure * inputs[] = { &view };

        return SL2DaxaResult(
            slEvaluateFeature(
                sl::kFeatureDLSS_RR,
                *token,
                inputs,
                static_cast<uint32_t>(std::size(inputs)),
                daxa_cmd_get_vk_command_buffer(*reinterpret_cast<daxa_CommandRecorder *>(&recorder))
            )
        );
    }

    auto ImplStreamlineContext::evaluate_dlssd(std::vector<sl::ResourceTag> const & resource_tags, CommandRecorder & recorder) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!token)
            return DAXA_RESULT_INCOMPLETE;

        std::vector<const sl::BaseStructure*> inputs;
        inputs.reserve(1 + resource_tags.size());
        sl::ViewportHandle view(viewport);
        inputs.push_back(&view);
        for (auto & tag : resource_tags)
            inputs.push_back(&tag);

        return SL2DaxaResult(
            slEvaluateFeature(
                sl::kFeatureDLSS_RR,
                *token,
                inputs.data(),
                static_cast<uint32_t>(inputs.size()),
                daxa_cmd_get_vk_command_buffer(*reinterpret_cast<daxa_CommandRecorder *>(&recorder))
            )
        );
    }

    auto ImplStreamlineContext::free_resources_dlssd(bool wait_idle) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (wait_idle)
        {
            auto res = daxa_dvc_wait_idle(device);
            if (res != DAXA_RESULT_SUCCESS)
                return res;
        }

        return SL2DaxaResult(slFreeResources(sl::kFeatureDLSS_RR, viewport));
    }

    
    auto ImplStreamlineContext::set_dlssg_options(sl::DLSSGOptions & options) -> daxa_Result 
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        return SL2DaxaResult(slDLSSGSetOptions(viewport, options));
    }
    
    auto ImplStreamlineContext::get_dlssg_state(sl::DLSSGState & state, sl::DLSSGOptions const & options) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        return SL2DaxaResult(slDLSSGGetState(viewport, state, &options));
    }

    auto ImplStreamlineContext::free_resources_dlssg(bool wait_idle) -> daxa_Result
    {
        if(!initialized)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (!device->instance->sl_enabled)
            return DAXA_RESULT_ERROR_INITIALIZATION_FAILED;

        if (wait_idle)
        {
            auto res = daxa_dvc_wait_idle(device);
            if (res != DAXA_RESULT_SUCCESS)
                return res;
        }

        return SL2DaxaResult(slFreeResources(sl::kFeatureDLSS_G, viewport));
    }

    void ImplStreamlineContext::zero_ref_callback(ImplHandle const * handle)
    {
        auto self = r_cast<ImplStreamlineContext const *>(handle);
        delete self;
    }
} // namespace daxa
#endif // STREAMLINE_ENABLED