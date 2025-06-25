#define DAXA_SHADERLANG DAXA_SHADERLANG_SLANG
#define APPNAME "Daxa Sample: Streamline"
#include <0_common/base_app.hpp>

using namespace daxa::types;
#include "shaders/shared.inl"
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <daxa/c/device.h>
#include <sl_matrix_helpers.h>

static constexpr const char * dlss_mode_names[] = {
    "Off",
    "Max Performance",
    "Balanced",
    "Max Quality",
    "Ultra Performance",
    "Ultra Quality",
    "DLAA"
};

static constexpr const char * reflex_mode_names[] = {
    "Off",
    "Low Latency",
    "Low Latency with Boost"
};

static constexpr const char * dlss_g_mode_names[] = {
    "Off",
    "On",
    "Auto"
};

static constexpr const char * render_mode_names[] = {
    "Path Tracing",
    "Motion Vectors",
    "Depth Buffer"
};

struct App : BaseApp<App>
{
    bool my_toggle = true;
    daxa::u32 render_width = size_x, render_height = size_y;
    bool is_dlss_enabled = true;
    bool use_dlss_rr = false;
    bool use_dlss_fg = false;
    bool resources_need_resize = false;
    u32 frame_count = 0;
    bool upload_env_map = true;
    int  dlssg_multiplier  = 2;
    sl::DLSSMode dlss_mode = sl::DLSSMode::eDLAA;
    sl::ReflexMode reflex_mode = sl::ReflexMode::eOff;
    sl::DLSSGMode dlss_g_mode = sl::DLSSGMode::eOff;
    sl::DLSSGOptions dlssg_options = {};
    i32 dlss_g_num_frames = 1;  // Number of frames to generate (1 = 2x, 2 = 3x, 3 = 4x)
    i32 dlss_g_max_multiplier = 0;  // Get DLSS-G status
    void* dlss_g_inputs_fence = nullptr;
    bool mode_changed = false;
    u64 dlss_g_last_fence_value = 0;
    u64 dlss_g_current_fence_value = 0;
    sl::DLSSGStatus dlss_g_status = {};
    u64 dlss_g_vram_estimate = 0;
    bool dlss_g_enabled_last_frame = false;
    bool dlss_g_swap_chain_needs_recreation = false;
    daxa::StreamlineContext streamline = daxa_ctx.streamline();

    u32 render_mode = 0u;

    f64 ms;
    f64 avg_ms = 0.0;
    f64 highest_ms = 0.0;

    f64 frame_time_sum = 0.0;
    u32 num_accumulated_frames = 0;
    f64 average_time_update_interval = 0.5; // Update every 0.5 seconds

    f64 highest_ms_reset_timer = 0.0;
    f64 highest_ms_reset_interval = 2.0; // Reset every 2 seconds

    sl::DLSSMode dlss_last_mode = sl::DLSSMode::eOff;
    daxa::u32 dlss_last_display_width = 0;
    daxa::u32 dlss_last_display_height = 0;

    struct StreamlineResources {
        sl::Resource input;
        sl::Resource diffuse;
        sl::Resource specular;
        sl::Resource normal_roughness;
        sl::Resource motion;
        sl::Resource specular_motion;
        sl::Resource depth;
        sl::Resource output;
        bool valid = false;
    } sl_resources;
    struct Camera {
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        float fov = 60.0f;
        float near_plane = 1e-3f;
        float far_plane = 1e7f;
        
        // Mouse control
        float yaw = -90.0f;
        float pitch = 0.0f;
        float distance = 5.0f;
        glm::vec2 last_mouse_pos = glm::vec2(0.0f);
        bool mouse_dragging = false;
    } camera;

    glm::mat4 view_matrix = glm::mat4(1.0f);
    glm::mat4 proj_matrix = glm::mat4(1.0f);
    glm::mat4 prev_view_matrix = glm::mat4(1.0f);
    glm::mat4 prev_proj_matrix = glm::mat4(1.0f);

    glm::vec3 camera_forward = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 camera_right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

    // Streamline expects row-major matrices
    auto matrix_to_sl(glm::mat4 const & m) -> sl::float4x4
    {
        // Streamline expects row-major matrices
        sl::float4x4 res;
        res.row[0].x = m[0][0];
        res.row[0].y = m[1][0];
        res.row[0].z = m[2][0];
        res.row[0].w = m[3][0];
        res.row[1].x = m[0][1];
        res.row[1].y = m[1][1];
        res.row[1].z = m[2][1];
        res.row[1].w = m[3][1];
        res.row[2].x = m[0][2];
        res.row[2].y = m[1][2];
        res.row[2].z = m[2][2];
        res.row[2].w = m[3][2];
        res.row[3].x = m[0][3];
        res.row[3].y = m[1][3];
        res.row[3].z = m[2][3];
        res.row[3].w = m[3][3];
        return res;
    }

    void update_camera() {
        // Store previous matrices
        prev_proj_matrix = proj_matrix;
        prev_view_matrix = view_matrix;
        
        // Calculate camera position from spherical coordinates
        camera.position.x = camera.target.x + camera.distance * cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
        camera.position.y = camera.target.y + camera.distance * sin(glm::radians(camera.pitch));
        camera.position.z = camera.target.z + camera.distance * sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
        
        // Update view matrix
        view_matrix = glm::lookAt(camera.position, camera.target, camera.up);
        
        // Update projection matrix
        if(render_width != 0 && render_height != 0) {
            float aspect_ratio = static_cast<float>(render_width) / static_cast<float>(render_height);
            proj_matrix = glm::perspective(glm::radians(camera.fov), aspect_ratio, camera.near_plane, camera.far_plane);
        }
        
        // Calculate camera vectors
        camera_forward = glm::normalize(camera.target - camera.position);
        camera_right = glm::normalize(glm::cross(camera_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        camera_up = glm::normalize(glm::cross(camera_right, camera_forward));
    }

    // Reflex configuration
    void setup_reflex() 
    {
        sl::ReflexOptions reflex_options = {};
        reflex_options.mode = reflex_mode;
        reflex_options.useMarkersToOptimize = true;
        reflex_options.virtualKey = 0x7C;
        reflex_options.frameLimitUs = 0;

        auto result = streamline.set_reflex_options(reflex_options);
        if(result != DAXA_RESULT_SUCCESS) {
            std::cerr << "Failed to set Reflex options!" << std::endl;
            return;
        }
    }

    // DLSS configuration
    void setup_dlss()
    {
        sl::DLSSOptions dlss_options = {};
        dlss_options.mode = dlss_mode;
        dlss_options.outputWidth = size_x;
        dlss_options.outputHeight = size_y;
        // default presets
        dlss_options.colorBuffersHDR = sl::Boolean::eTrue;
        dlss_options.useAutoExposure = sl::Boolean::eFalse; // True requests exposure buffer
        dlss_options.alphaUpscalingEnabled = sl::Boolean::eFalse;
        
        // Set DLSS options
        auto result = streamline.set_dlss_options(dlss_options);
        if (result != DAXA_RESULT_SUCCESS)
        {
            std::cerr << "Failed to set DLSS options!" << std::endl;
            return;
        }
        
        bool dlss_resize_required = (dlss_mode != dlss_last_mode) || 
                                (size_x != dlss_last_display_width) || 
                                (size_y != dlss_last_display_height);

        
        if(dlss_resize_required) 
        {

            const auto prev_render_width = render_width;
            const auto prev_render_height = render_height;
            
            // Get optimal render resolution
            sl::DLSSOptimalSettings optimal_settings = {};
            result = streamline.get_dlss_optimal_settings(dlss_options, optimal_settings);
            if (result != DAXA_RESULT_SUCCESS || optimal_settings.optimalRenderWidth == 0 || optimal_settings.optimalRenderHeight == 0)
            {
                optimal_settings.optimalRenderWidth = size_x;
                optimal_settings.optimalRenderHeight = size_y;
            }

            if(is_dlss_enabled) 
            {
                render_width = optimal_settings.optimalRenderWidth;
                render_height = optimal_settings.optimalRenderHeight;
            }
            else
            {
                render_width = size_x;
                render_height = size_y;
            }
            
            // Update tracking variables
            dlss_last_mode = dlss_mode;
            dlss_last_display_width = size_x;
            dlss_last_display_height = size_y;
            // std::cout << "DLSS optimal render resolution: " << optimal_settings.optimalRenderWidth << "x" << optimal_settings.optimalRenderHeight << std::endl;

            if(prev_render_width != render_width || prev_render_height != render_height) 
                resources_need_resize = true;
        }

        if (resources_need_resize) {
            if (is_dlss_enabled) {
                streamline.free_resources_dlss(true);
                if (use_dlss_rr) {
                    streamline.free_resources_dlssd(true);
                }
#if STREALINE_FG_ENABLED == 1
                if (use_dlss_fg) {
                    streamline.free_resources_dlssg(true);
                }
#endif
            }
            resources_need_resize = false;
        }

        if(use_dlss_rr) {
            // configure RR
            sl::DLSSDOptions dlssd_options = {};
            dlssd_options.mode = dlss_options.mode;
            dlssd_options.outputWidth = size_x;
            dlssd_options.outputHeight = size_y;
            dlssd_options.colorBuffersHDR = dlss_options.colorBuffersHDR;
            dlssd_options.normalRoughnessMode = sl::DLSSDNormalRoughnessMode::ePacked;
            dlssd_options.alphaUpscalingEnabled = dlss_options.alphaUpscalingEnabled;
            result = streamline.set_dlssd_options(dlssd_options);
            if (result != DAXA_RESULT_SUCCESS)
            {
                std::cerr << "Failed to set DLSS-RR options!\n";
                return;
            }

            if(dlss_resize_required) {
                sl::DLSSDOptimalSettings dlssd_opt = {};
                result = streamline.get_dlssd_optimal_settings(dlssd_options, dlssd_opt);
                if (result != DAXA_RESULT_SUCCESS)
                {
                    std::cerr << "Failed to get DLSS-RR optimal settings!\n";
                }
                // else
                // {
                //     std::cout << "DLSS-RR optimal sharpness: "
                //             << dlssd_opt.optimalSharpness << "\n";
                // }
            }
        }
    }

    void setup_dlss_g()
    {
        if (!use_dlss_fg) {
            streamline.free_resources_dlssg(false);
            return;
        }

        dlssg_options.mode = dlss_g_mode;
        dlssg_options.numFramesToGenerate = static_cast<u32>(dlss_g_num_frames);
        
        // Important flags
        dlssg_options.flags |= sl::DLSSGFlags::eRetainResourcesWhenOff;  // Avoid stutter when switching
        
        // Buffer configuration
        dlssg_options.numBackBuffers = 3;  // triple buffering
        dlssg_options.mvecDepthWidth = render_width;
        dlssg_options.mvecDepthHeight = render_height;
        dlssg_options.colorWidth = size_x;
        dlssg_options.colorHeight = size_y;
        
        // Native formats (Vulkan)
        dlssg_options.colorBufferFormat = static_cast<u32>(daxa::Format::R8G8B8A8_UNORM);
        dlssg_options.mvecBufferFormat = static_cast<u32>(daxa::Format::R32G32_SFLOAT);
        dlssg_options.depthBufferFormat = static_cast<u32>(daxa::Format::R32_SFLOAT);
        dlssg_options.hudLessBufferFormat = static_cast<u32>(daxa::Format::R8G8B8A8_UNORM);
        dlssg_options.uiBufferFormat = static_cast<u32>(daxa::Format::R8G8B8A8_UNORM);

        auto result = streamline.set_dlssg_options(dlssg_options);
        if (result != DAXA_RESULT_SUCCESS)
        {
            std::cerr << "Failed to set DLSS-G options!" << std::endl;
            use_dlss_fg = false;
        }
    }

#if STREALINE_FG_ENABLED == 1
    void query_dlss_g_state()
    {
        if (!use_dlss_fg) return;

        sl::DLSSGState state = {};
        dlssg_options.mode = dlss_g_mode;
        dlssg_options.numFramesToGenerate = static_cast<u32>(dlss_g_num_frames);
        
        auto result = streamline.get_dlssg_state(state, dlssg_options);
        if (result == DAXA_RESULT_SUCCESS)
        {
            dlss_g_vram_estimate = state.estimatedVRAMUsageInBytes;
            dlss_g_status = state.status;
            dlss_g_max_multiplier = static_cast<i32>(state.numFramesToGenerateMax) + 1;  // +1 additional frames
            dlss_g_inputs_fence = state.inputsProcessingCompletionFence;
            dlss_g_current_fence_value = state.lastPresentInputsProcessingCompletionFenceValue;
            
            // Check the minimum swapchain size
            if (size_x < state.minWidthOrHeight || size_y < state.minWidthOrHeight) {
                std::cout << "Swapchain too small for DLSS-G. Minimum size: " << state.minWidthOrHeight << std::endl;
                dlss_g_mode = sl::DLSSGMode::eOff;
            }
        }
    }

    void queue_gpu_wait_on_sync_object(daxa::Device& device, void* sync_obj, u64 sync_obj_val)
    {
        daxa_TimelineSemaphore timeline_semaphore;
        daxa_TimelineSemaphoreInfo info{.initial_value = 0, .name = "DLSS-G timeline semaphore"};
        auto result = daxa_dvc_wrap_timeline_semaphore(
            *reinterpret_cast<daxa_Device *>(&device), 
            reinterpret_cast<VkSemaphore>(sync_obj),
            &info,
            &timeline_semaphore
        );
        if (result == DAXA_RESULT_SUCCESS) {
            additional_wait_timeline_semaphores.push_back({*reinterpret_cast<daxa::TimelineSemaphore*>(&timeline_semaphore), sync_obj_val});
        }
    }
#endif 

    void release_images()
    {
        device.destroy_image(depth_image);
        device.destroy_image(specular_motion_vector_image);
        device.destroy_image(motion_vector_image);
        device.destroy_image(output_image);
        device.destroy_image(normal_roughness_image);
        device.destroy_image(specular_albedo_image);
        device.destroy_image(diffuse_albedo_image);
        device.destroy_image(render_image);
    }

    void allocate_images() 
    {
        resources_need_resize = false;
        render_image = device.create_image({
            .format = daxa::Format::R8G8B8A8_UNORM,
            .size = {render_width, render_height, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
        });
        diffuse_albedo_image = device.create_image(daxa::ImageInfo{
            .format = daxa::Format::R8G8B8A8_UNORM,
            .size = {render_width, render_height, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .name = "diffuse_albedo_image",
        });
        specular_albedo_image = device.create_image(daxa::ImageInfo{
            .format = daxa::Format::R8G8B8A8_UNORM,
            .size = {render_width, render_height, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .name = "specular_albedo_image",
        });
        normal_roughness_image  = device.create_image(daxa::ImageInfo{
            .format = daxa::Format::R16G16B16A16_SFLOAT,
            .size = {render_width, render_height, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
            .name = "normal_roughness_image",
        }); 
        output_image = device.create_image({
            .format = daxa::Format::R8G8B8A8_UNORM,
            .size = {size_x, size_y, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
        });
        motion_vector_image = device.create_image(daxa::ImageInfo{
            .format = daxa::Format::R32G32_SFLOAT,
            .size = {render_width, render_height, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
            .name = "motion_vector_image",
        });
        specular_motion_vector_image  = device.create_image(daxa::ImageInfo{
            .format = daxa::Format::R32G32_SFLOAT,
            .size = {render_width, render_height, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
            .name = "specular_motion_vector_image",
        });
        depth_image = device.create_image(daxa::ImageInfo{
            .format = daxa::Format::R32_SFLOAT,
            .size = {render_width, render_height, 1},
            .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::COLOR_ATTACHMENT | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
            .name = "depth_image",
        });
    }

    void attach_tasks() {
        task_render_image.set_images({.images = std::array{render_image}});
        task_diffuse_albedo_image.set_images({.images = std::array{diffuse_albedo_image}});
        task_specular_albedo_image.set_images({.images = std::array{specular_albedo_image}});
        task_normal_rougness_image.set_images({.images = std::array{normal_roughness_image}});
        task_output_image.set_images({.images = std::array{output_image}}); 
        task_motion_vector_image.set_images({.images = std::array{motion_vector_image}});
        task_specular_motion_vector_image.set_images({.images = std::array{specular_motion_vector_image}});
        task_depth_image.set_images({.images = std::array{depth_image}});
    }

    
    sl::Constants setup_constants() {
        sl::Constants constants = {};
        
        // Basic camera parameters
        constants.cameraAspectRatio = static_cast<float>(render_width) / static_cast<float>(render_height);
        constants.cameraNear = camera.near_plane;
        constants.cameraFar = camera.far_plane;
        constants.cameraFOV = glm::radians(camera.fov);
        
        // Motion vector scale
        constants.mvecScale = { 1.0f / static_cast<float>(render_width), 1.0f / static_cast<float>(render_height)}; // This are scale factors used to normalize mvec (to -1,1) mvec in pixel space
        
        // Camera position and orientation
        constants.cameraPinholeOffset = sl::float2(0.0f, 0.0f);
        constants.cameraPos = sl::float3(camera.position.x, camera.position.y, camera.position.z);
        constants.cameraUp = sl::float3(camera_up.x, camera_up.y, camera_up.z);
        constants.cameraRight = sl::float3(camera_right.x, camera_right.y, camera_right.z);
        constants.cameraFwd = sl::float3(camera_forward.x, camera_forward.y, camera_forward.z);

        // Set view to clip matrices
        constants.cameraViewToClip = matrix_to_sl(proj_matrix);
        constants.clipToCameraView = matrix_to_sl(glm::inverse(proj_matrix));
        
        // Get reprojection matrix
        glm::mat4 view_inv = glm::inverse(view_matrix);
        glm::mat4 proj_inv = glm::inverse(proj_matrix);
        
        glm::mat4 reprojection_matrix = proj_inv * view_inv * prev_view_matrix * prev_proj_matrix;
        
        constants.clipToPrevClip = matrix_to_sl(reprojection_matrix);
        constants.prevClipToClip = matrix_to_sl(glm::inverse(reprojection_matrix));

        constants.clipToLensClip = matrix_to_sl(glm::mat4(1.0f));

        // Depth and motion vector settings
        constants.depthInverted = sl::Boolean::eFalse;
        constants.cameraMotionIncluded = sl::Boolean::eTrue;
        constants.motionVectors3D = sl::Boolean::eFalse;
        constants.reset = frame_count == 0 || !sl_resources.valid ? sl::Boolean::eTrue : sl::Boolean::eFalse;
        constants.motionVectorsJittered = sl::Boolean::eFalse;
        constants.motionVectorsDilated = sl::Boolean::eFalse;
        constants.motionVectorsInvalidValue = FLT_MIN;
        constants.jitterOffset = sl::float2(0.0f, 0.0f);

        return constants;
    }
    
    // Convert Daxa image to Streamline resource
    sl::Resource createStreamlineResource(daxa::Device& device, daxa::ImageId image_id, daxa::ImageViewId view_id, sl::ResourceType type, sl::BufferType buffer_type)
    {
#define HANDLE_RES(expression)          \
    {                                   \
        auto res = expression;          \
        if (res != DAXA_RESULT_SUCCESS) \
        {                               \
            return sl::Resource();      \
        }                               \
    }
        // Get the underlying VkImage handle from Daxa
        VkImage vk_image;
        HANDLE_RES(daxa_dvc_get_vk_image(*reinterpret_cast<daxa_Device *>(&device), std::bit_cast<daxa_ImageId>(image_id), &vk_image));

        // Get the underlying VkImageView handle from Daxa
        VkImageView vk_view;
        HANDLE_RES(daxa_dvc_get_vk_image_view(*reinterpret_cast<daxa_Device *>(&device), std::bit_cast<daxa_ImageViewId>(view_id), &vk_view));

        // Get the underlying vkDeviceMemory handle from Daxa
        VkDeviceMemory vk_memory;
        HANDLE_RES(daxa_dvc_get_vk_image_memory(*reinterpret_cast<daxa_Device *>(&device), std::bit_cast<daxa_ImageId>(image_id), &vk_memory));

        auto image_info = device.image_info(image_id).value();

        sl::Resource resource{};
        resource.type = type;
        resource.native = vk_image;
        resource.view = vk_view;
        resource.state = buffer_type == sl::kBufferTypeScalingOutputColor ? 
            VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_UNDEFINED;
        resource.memory = vk_memory;

        // Fill required fields from Daxa ImageInfo
        resource.width = image_info.size.x;
        resource.height = image_info.size.y;
        resource.nativeFormat = static_cast<u32>(image_info.format);
        resource.mipLevels = image_info.mip_level_count;
        resource.arrayLayers = image_info.array_layer_count;
        resource.flags = image_info.flags;
        resource.usage = image_info.usage;

        return resource;
    }

    // clang-format off
    std::shared_ptr<daxa::ComputePipeline> compute_pipeline = [this]() {
        return pipeline_manager.add_compute_pipeline2({
            .source = daxa::ShaderFile{"compute.slang"}, 
            .entry_point = "entry_streamline",
        }).value();
    }();
    // clang-format on

    daxa::BufferId gpu_input_buffer = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(GpuInput),
        .name = "gpu_input_buffer",
    });
    daxa::BufferId gpu_input_buffer_previous = device.create_buffer(daxa::BufferInfo{
        .size = sizeof(GpuInput),
        .name = "gpu_input_buffer_previous",
    });
    GpuInput gpu_input = {}, gpu_input_previous = {};
    daxa::TaskBuffer task_gpu_input_buffer{{.initial_buffers = {.buffers = std::array{gpu_input_buffer}}, .name = "input_buffer"}};
    daxa::TaskBuffer task_gpu_input_buffer_previous{{.initial_buffers = {.buffers = std::array{gpu_input_buffer_previous}}, .name = "input_buffer_previous"}};

    const u32 ENV_W = 2048;
    const u32 ENV_H = 1024;
    daxa::ImageId environment_map = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {ENV_W, ENV_H, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::TRANSFER_DST | daxa::ImageUsageFlagBits::SHADER_STORAGE,
        .name = "environment_map",
    });
    daxa::TaskImage task_environment_map_image{{.initial_images = {.images = std::array{environment_map}}, .name = "environment_map_image"}};

    daxa::ImageId render_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {render_width, render_height, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .name = "render_image",
    });
    daxa::ImageId diffuse_albedo_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {render_width, render_height, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .name = "diffuse_albedo_image",
    });
    daxa::ImageId specular_albedo_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {render_width, render_height, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .name = "specular_albedo_image",
    });
    daxa::ImageId normal_roughness_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R16G16B16A16_SFLOAT,
        .size = {render_width, render_height, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .name = "normal_roughness_image",
    }); 
    daxa::ImageId output_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {size_x, size_y, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC | daxa::ImageUsageFlagBits::TRANSFER_DST,
        .name = "output_image",
    });
    daxa::ImageId motion_vector_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R32G32_SFLOAT,
        .size = {render_width, render_height, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .name = "motion_vector_image",
    });
    daxa::ImageId specular_motion_vector_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R32G32_SFLOAT,
        .size = {render_width, render_height, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .name = "specular_motion_vector_image",
    });
    daxa::ImageId depth_image = device.create_image(daxa::ImageInfo{
        .format = daxa::Format::R32_SFLOAT,
        .size = {render_width, render_height, 1},
        .usage = daxa::ImageUsageFlagBits::SHADER_SAMPLED | daxa::ImageUsageFlagBits::SHADER_STORAGE | daxa::ImageUsageFlagBits::TRANSFER_SRC,
        .name = "depth_image",
    });

    daxa::TaskImage task_render_image{{.initial_images = {.images = std::array{render_image}}, .name = "render_image"}};
    daxa::TaskImage task_diffuse_albedo_image{{.initial_images = {.images = std::array{diffuse_albedo_image}}, .name = "diffuse_albedo_image"}};
    daxa::TaskImage task_specular_albedo_image{{.initial_images = {.images = std::array{specular_albedo_image}}, .name = "specular_albedo_image"}};
    daxa::TaskImage task_normal_rougness_image{{.initial_images = {.images = std::array{normal_roughness_image}}, .name = "task_normal_rougness_image"}};
    daxa::TaskImage task_output_image{{.initial_images = {.images = std::array{output_image}}, .name = "output_image"}};
    daxa::TaskImage task_motion_vector_image{{.initial_images = {.images = std::array{motion_vector_image}}, .name = "motion_vector_image"}};
    daxa::TaskImage task_specular_motion_vector_image{{.initial_images = {.images = std::array{specular_motion_vector_image}}, .name = "specular_motion_vector_image"}};
    daxa::TaskImage task_depth_image{{.initial_images = {.images = std::array{depth_image}}, .name = "depth_image"}};
    daxa::SamplerId sampler = device.create_sampler({.name = "sampler"});

    daxa::TimelineQueryPool timeline_query_pool = device.create_timeline_query_pool({
        .query_count = 2,
        .name = "timeline_query",
    });

    daxa::TaskGraph loop_task_graph = record_loop_task_graph();

    ~App()
    {
        device.wait_idle();
        sl::ResourceTag inputs[] = {
            sl::ResourceTag{nullptr, sl::kBufferTypeDepth, sl::ResourceLifecycle::eValidUntilPresent},
            sl::ResourceTag{nullptr, sl::kBufferTypeBackbuffer, sl::ResourceLifecycle::eValidUntilPresent},
            sl::ResourceTag{nullptr, sl::kBufferTypeMotionVectors, sl::ResourceLifecycle::eValidUntilPresent},
            sl::ResourceTag{nullptr, sl::kBufferTypeScalingInputColor, sl::ResourceLifecycle::eValidUntilPresent},
            sl::ResourceTag{nullptr, sl::kBufferTypeScalingOutputColor, sl::ResourceLifecycle::eValidUntilPresent},
            sl::ResourceTag{nullptr, sl::kBufferTypeHUDLessColor, sl::ResourceLifecycle::eValidUntilPresent} };
        streamline.set_tag_for_frame(inputs, static_cast<u32>(std::size(inputs)), nullptr);
        if (is_dlss_enabled) {
            streamline.free_resources_dlss(true);
            if (use_dlss_rr) {
                streamline.free_resources_dlssd(true);
            }
#if STREALINE_FG_ENABLED == 1
            if (use_dlss_fg) {
                streamline.free_resources_dlssg(true);
            }
#endif
        }
        streamline.shutdown();
        device.destroy_sampler(sampler);
        device.destroy_image(environment_map);
        device.destroy_buffer(gpu_input_buffer);
        device.destroy_buffer(gpu_input_buffer_previous);
        release_images();
        device.wait_idle();
        device.collect_garbage();
    }

    void ui_update()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Settings");
        ImGui::SetWindowFontScale(2.0f);
        

        ImGui::Text("Debug Options:");
        i32 prev_render_mode = static_cast<i32>(render_mode);
        if (ImGui::Combo("Render mode", &prev_render_mode, render_mode_names, IM_ARRAYSIZE(render_mode_names)))
        {
            render_mode = static_cast<u32>(prev_render_mode);
            
            // Al cambiar a un modo distinto de Path Tracing (0) forzamos DLSS = off
            if (render_mode != 0 && is_dlss_enabled)
            {
                is_dlss_enabled = false;
                resources_need_resize = true;
                sl_resources.valid   = false;
                // libera recursos
                streamline.free_resources_dlss(true);
                streamline.free_resources_dlssd(true);
        #if STREALINE_FG_ENABLED == 1
                streamline.free_resources_dlssg(true);
        #endif
                // reallocate resources
                release_images();
                allocate_images();
                attach_tasks();
            }
        }
        
        ImGui::Separator();

        // Show Engine FPS
        double fps = ms > 0.0 ? 1000.0 / ms : 0.0;
        double avg_fps = avg_ms > 0.0 ? 1000.0 / avg_ms : 0.0;
        double min_fps = highest_ms > 0.0 ? 1000.0 / highest_ms : 0.0;
        
        ImGui::Text("Current: %.1f FPS (%.2f ms)", fps, ms);
        ImGui::Text("Average: %.1f FPS (%.2f ms)", avg_fps, avg_ms);
        ImGui::Text("1%% Low:  %.1f FPS (%.2f ms)", min_fps, highest_ms);

        // Show true FPS when DLSS-FG is active
        if (use_dlss_fg && dlss_g_mode != sl::DLSSGMode::eOff)
        {
            // dlss_g_num_frames = N says that N+1 frames are generated for every input
            double gen_fps = fps * double(dlss_g_num_frames + 1);
            ImGui::Text("Generated: %.1f FPS", gen_fps);
        }

        ImGui::Separator();

        if (render_mode == 0) {
        
            if (ImGui::Checkbox("Enable DLSS", &is_dlss_enabled))
            {
                // everytime state changes, resources are invalidated
                resources_need_resize = true;
                sl_resources.valid   = false;
                if (!is_dlss_enabled)
                {
                    // native resolution
                    render_width  = size_x;
                    render_height = size_y;
                    // if DLSS is disabled, free resources
                    streamline.free_resources_dlss(true);
                    streamline.free_resources_dlssd(true);
#if STREALINE_FG_ENABLED == 1
                    streamline.free_resources_dlssg(true);
    #endif 
                    // reallocate resources
                    release_images();
                    allocate_images();
                    attach_tasks();
                }
                else
                {
                    // if DLSS is activated, re-configure it
                    setup_dlss();
                }
            }

            {
                int reflex_idx = static_cast<int>(reflex_mode);
                if (ImGui::Combo("Reflex Mode", &reflex_idx, reflex_mode_names, IM_ARRAYSIZE(reflex_mode_names)))
                {
                    reflex_mode = static_cast<sl::ReflexMode>(reflex_idx);
                    setup_reflex();
                }
            }
        }
        
        if (is_dlss_enabled)
        {
            auto prev_dlss_mode = dlss_mode;
            int mode = static_cast<int>(dlss_mode);
            if (ImGui::Combo("DLSS Mode", &mode, dlss_mode_names, IM_ARRAYSIZE(dlss_mode_names)))
            {
                auto new_dlss_mode = static_cast<sl::DLSSMode>(mode);
                mode_changed = prev_dlss_mode != new_dlss_mode;
                if(mode_changed) {
                    dlss_mode = new_dlss_mode;
                    // change mode, reconfigure 
                    resources_need_resize = true;
                    sl_resources.valid = false;
                    setup_dlss();
                }
            }

            if (dlss_mode != sl::DLSSMode::eOff)
            {
                if (ImGui::Checkbox("Enable DLSS-RR", &use_dlss_rr))
                {
                    resources_need_resize = true;
                    sl_resources.valid = false;
                    setup_dlss();
                    if (!use_dlss_rr)
                        streamline.free_resources_dlssd(true);
                }

                // DLSS Frame Generation
                ImGui::Separator();

#if STREALINE_FG_ENABLED == 1
                
                bool dlss_g_wanted = (dlss_g_mode != sl::DLSSGMode::eOff);
                bool prev_dlss_g_wanted = dlss_g_wanted;
                if (ImGui::Checkbox("Enable DLSS-FG", &use_dlss_fg))
                {
                    if (use_dlss_fg) {
                        dlss_g_mode = sl::DLSSGMode::eOn;
                    } else {
                        dlss_g_mode = sl::DLSSGMode::eOff;
                        streamline.free_resources_dlssg(true);
                    }
                    
                    // Check that we need to recreate the swapchain if the state changes
                    dlss_g_swap_chain_needs_recreation = (prev_dlss_g_wanted != (dlss_g_mode != sl::DLSSGMode::eOff));
                    
                    resources_need_resize = true;
                    sl_resources.valid = false;
                }
#endif 
                if (use_dlss_fg)
                {
                    if(reflex_mode == sl::ReflexMode::eOff) {
                        reflex_mode = sl::ReflexMode::eLowLatency;
                        setup_reflex();
                    }

                    int g_mode = static_cast<int>(dlss_g_mode);
                    if (ImGui::Combo("DLSS-G Mode", &g_mode, dlss_g_mode_names, IM_ARRAYSIZE(dlss_g_mode_names)))
                    {
                        dlss_g_mode = static_cast<sl::DLSSGMode>(g_mode);
                    }
                    
                    if (dlss_g_max_multiplier > 0)
                    {
                        int multiplier = dlss_g_num_frames + 1;
                        ImGui::Text("Frame Multiplier:");
                        ImGui::SameLine();
                        
                        for (int i = 2; i <= dlss_g_max_multiplier; i++)
                        {
                            char label[16];
                            snprintf(label, sizeof(label), "%dx", i);
                            if (ImGui::RadioButton(label, &multiplier, i))
                            {
                                dlss_g_num_frames = multiplier - 1;
                            }
                            if (i < dlss_g_max_multiplier) ImGui::SameLine();
                        }
                    }
                    
                    if (dlss_g_status != sl::DLSSGStatus::eOk)
                    {
                        ImGui::TextColored(ImVec4(1,0,0,1), "DLSS-G Status:");
                        
                        if (dlss_g_status & sl::DLSSGStatus::eFailResolutionTooLow)
                            ImGui::Text("  - Resolution too low");
                        if (dlss_g_status & sl::DLSSGStatus::eFailReflexNotDetectedAtRuntime)
                            ImGui::Text("  - Reflex not detected (must be enabled)");
                        if (dlss_g_status & sl::DLSSGStatus::eFailHDRFormatNotSupported)
                            ImGui::Text("  - HDR format not supported");
                        if (dlss_g_status & sl::DLSSGStatus::eFailCommonConstantsInvalid)
                            ImGui::Text("  - Invalid constants");
                    }
                    
                    if (dlss_g_vram_estimate > 0)
                    {
                        ImGui::Text("DLSS-G VRAM: %.1f MB", static_cast<f64>(dlss_g_vram_estimate) / (1024.0 * 1024.0));
                    }
                }
            }
        }
        
        ImGui::End();
        ImGui::Render();
    }


    void update_gpu_input() 
    {
        gpu_input_previous = gpu_input;

        gpu_input.frame_dim = {render_width, render_height};
        gpu_input.camera_pos = {camera.position.x, camera.position.y, camera.position.z};
        gpu_input.camera_right = {camera_right.x, camera_right.y, camera_right.z};
        gpu_input.camera_up = {camera_up.x, camera_up.y, camera_up.z};
        gpu_input.camera_forward = {camera_forward.x, camera_forward.y, camera_forward.z};
        gpu_input.aspect = float(render_width) / float(render_height);
        gpu_input.fov = glm::radians(camera.fov);
        gpu_input.near_plane = camera.near_plane;
        gpu_input.far_plane =  camera.far_plane;
    }

    void on_update()
    {
        auto result = streamline.reflex_sleep(frame_count);
        if (result != DAXA_RESULT_SUCCESS)
        {
            std::cerr << "Failed to begin frame!" << std::endl;
        }
        result = streamline.pcl_set_marker(sl::PCLMarker::eSimulationStart, frame_count);
        if (result != DAXA_RESULT_SUCCESS)
        {
            std::cerr << "Failed to start sim!" << std::endl;
        }

        update_camera();
        update_gpu_input();

        result = streamline.pcl_set_marker(sl::PCLMarker::eSimulationEnd, frame_count);
        if (result != DAXA_RESULT_SUCCESS)
        {
            std::cerr << "Failed to end sim!" << std::endl;
        }
        auto reloaded_result = pipeline_manager.reload_all();
        if (auto reload_err = daxa::get_if<daxa::PipelineReloadError>(&reloaded_result))
            std::cout << "Failed to reload " << reload_err->message << '\n';
        if (daxa::get_if<daxa::PipelineReloadSuccess>(&reloaded_result))
            std::cout << "Successfully reloaded!\n";

        ui_update();

        auto swapchain_image = swapchain.acquire_next_image();
        task_swapchain_image.set_images({.images = std::array{swapchain_image}});
        if (swapchain_image.is_empty())
        {
            return;
        }
        
        result = streamline.pcl_set_marker(sl::PCLMarker::eRenderSubmitStart, frame_count);
        if (result != DAXA_RESULT_SUCCESS)
        {
            std::cerr << "Failed to start rendering!" << std::endl;
        }
        
        loop_task_graph.execute({});
#if STREALINE_FG_ENABLED == 1
        additional_wait_timeline_semaphores.clear();
#endif 
        result = streamline.pcl_set_marker(sl::PCLMarker::ePresentEnd, frame_count);
        if (result != DAXA_RESULT_SUCCESS)
        {
            std::cerr << "Failed to end present!" << std::endl;
        }

        device.collect_garbage();
        ++frame_count;

        auto query_results = timeline_query_pool.get_query_results(0, 2);
        if ((query_results[1] != 0u) && (query_results[3] != 0u))
        {
            ms = static_cast<f64>(query_results[2] - query_results[0]) / 1000000.0;

            // Update the highest time
            if (ms > highest_ms) {
                highest_ms = ms;
                // Reset the timer when we find a new maximum
                highest_ms_reset_timer = 0.0;
            }
            
            // Accumulate for the average
            frame_time_sum += ms;
            num_accumulated_frames++;
            
            // Update the average if enough time has passed
            if (frame_time_sum > average_time_update_interval * 1000.0) // convert to ms
            {
                avg_ms = frame_time_sum / static_cast<f64>(num_accumulated_frames);
                num_accumulated_frames = 0;
                frame_time_sum = 0.0;
            }
            
            // Reset the highest time periodically
            highest_ms_reset_timer += delta_time;
            if (highest_ms_reset_timer > highest_ms_reset_interval)
            {
                highest_ms = ms; // Reset to current value
                highest_ms_reset_timer = 0.0;
            }
        }
    }

    void on_mouse_move(f32 x, f32 y) {
        if (!camera.mouse_dragging) return;

        glm::vec2 curr = glm::vec2(x, y);
        glm::vec2 delta = curr - camera.last_mouse_pos;
        camera.last_mouse_pos = curr;

        // NOTE: adjust sensibility
        constexpr float sensitivity = 0.1f;
        camera.yaw   += delta.x * sensitivity;
        camera.pitch -= delta.y * sensitivity;

        camera.pitch = glm::clamp(camera.pitch, -89.0f, 89.0f);
    }
    void on_mouse_button(i32 button, i32 action) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                camera.mouse_dragging = true;
                double x, y;
                glfwGetCursorPos(glfw_window_ptr, &x, &y);
                camera.last_mouse_pos = glm::vec2(x, y);
            } else if (action == GLFW_RELEASE) {
                camera.mouse_dragging = false;
            }
        }
    }
    void on_key(i32 key, i32 action) {
        constexpr float moveSpeed = 5.0f;
        constexpr float boostFactor = 10.0f;
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            bool fast = glfwGetKey(glfw_window_ptr, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
            float speed = moveSpeed * (fast ? boostFactor : 1.0f);
            float delta = delta_time * speed;
            // forward/back
            if (key == GLFW_KEY_W) camera.target += camera_forward * delta;
            if (key == GLFW_KEY_S) camera.target -= camera_forward * delta;
            // strafe
            if (key == GLFW_KEY_A) camera.target -= camera_right * delta;
            if (key == GLFW_KEY_D) camera.target += camera_right * delta;
            // up/down
            if (key == GLFW_KEY_Q) camera.target -= camera.up * delta;
            if (key == GLFW_KEY_E) camera.target += camera.up * delta;
        }
        if (action == GLFW_PRESS && key == GLFW_KEY_L)
        {
            is_dlss_enabled = !is_dlss_enabled;
            resources_need_resize = true;
            sl_resources.valid   = false;
            if (is_dlss_enabled) {
                setup_dlss();
            } else {
                streamline.free_resources_dlss(true);
                streamline.free_resources_dlssd(true);
            }
        }
    }
    void on_resize(u32 sx, u32 sy)
    {
        minimized = (sx == 0 || sy == 0);
        if (!minimized)
        {
            swapchain.resize();
            sl_resources.valid = false;
            render_width = size_x = swapchain.get_surface_extent().x;
            render_height = size_y = swapchain.get_surface_extent().y;
            setup_dlss();
            setup_dlss_g();
            release_images();
            allocate_images();
            attach_tasks();
            base_on_update();
        }
    }

    void record_tasks(daxa::TaskGraph & new_task_graph)
    {
        new_task_graph.use_persistent_image(task_render_image);
        new_task_graph.use_persistent_image(task_diffuse_albedo_image);
        new_task_graph.use_persistent_image(task_specular_albedo_image);
        new_task_graph.use_persistent_image(task_normal_rougness_image);
        new_task_graph.use_persistent_image(task_output_image);
        new_task_graph.use_persistent_buffer(task_gpu_input_buffer);
        new_task_graph.use_persistent_buffer(task_gpu_input_buffer_previous);
        new_task_graph.use_persistent_image(task_environment_map_image);
        new_task_graph.use_persistent_image(task_motion_vector_image);
        new_task_graph.use_persistent_image(task_specular_motion_vector_image);
        new_task_graph.use_persistent_image(task_depth_image);

        imgui_task_attachments.push_back(daxa::inl_attachment(daxa::TaskImageAccess::FRAGMENT_SHADER_SAMPLED, task_render_image));

        
#if STREALINE_FG_ENABLED == 1
        new_task_graph.add_task({
            .attachments = {
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_render_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_output_image),
            },
            .task = [this](daxa::TaskInterface ti)
            {
                auto last_dlss_g_fence_value = dlss_g_last_fence_value;

                query_dlss_g_state();

                auto dlss_g_fence_value = dlss_g_current_fence_value;

                setup_dlss_g();

                query_dlss_g_state();

                if (use_dlss_fg && dlss_g_inputs_fence)
                {
                    if (dlss_g_enabled_last_frame)
                    {
                        if (dlss_g_fence_value == 0 || dlss_g_fence_value > last_dlss_g_fence_value)
                        {
                            // Wait before modifying any input resources
                            queue_gpu_wait_on_sync_object(device, dlss_g_inputs_fence, dlss_g_fence_value);
                        }
                    }
                    else
                    {
                        if (dlss_g_current_fence_value < dlss_g_last_fence_value)
                        {
                            std::cerr << "DLSS-G fence value out of order: current=" 
                                    << dlss_g_current_fence_value << ", last=" << dlss_g_last_fence_value << std::endl;
                        }
                        else if (dlss_g_current_fence_value != 0)
                        {
                            std::cout << "DLSS-G was inactive in the last presenting frame" << std::endl;
                        }
                    }
                }
                
                dlss_g_last_fence_value = dlss_g_current_fence_value;
                dlss_g_enabled_last_frame = (dlss_g_mode != sl::DLSSGMode::eOff);
            },
            .name = "DLSS-G fence",
        });
#endif 
        new_task_graph.add_task({
            .attachments = {
                daxa::inl_attachment(daxa::TaskBufferAccess::HOST_TRANSFER_WRITE, task_gpu_input_buffer),
                daxa::inl_attachment(daxa::TaskBufferAccess::HOST_TRANSFER_WRITE, task_gpu_input_buffer_previous),
                daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_WRITE, task_environment_map_image),
            },
            .task = [this](daxa::TaskInterface ti)
            {
                ti.recorder.reset_timestamps({
                    .query_pool = timeline_query_pool,
                    .start_index = 0,
                    .count = timeline_query_pool.info().query_count,
                });
                ti.recorder.write_timestamp({
                    .query_pool = timeline_query_pool,
                    .pipeline_stage = daxa::PipelineStageFlagBits::BOTTOM_OF_PIPE,
                    .query_index = 0,
                });
                auto staging_gpu_input_buffer = device.create_buffer({
                    .size = sizeof(GpuInput),
                    .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .name = ("staging_gpu_input_buffer"),
                });
                ti.recorder.destroy_buffer_deferred(staging_gpu_input_buffer);
                auto * buffer_ptr = device.buffer_host_address_as<GpuInput>(staging_gpu_input_buffer).value();
                *buffer_ptr = gpu_input;
                ti.recorder.copy_buffer_to_buffer({
                    .src_buffer = staging_gpu_input_buffer,
                    .dst_buffer = gpu_input_buffer,
                    .size = sizeof(GpuInput),
                });

                auto staging_previous = device.create_buffer({
                    .size = sizeof(GpuInput),
                    .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                    .name = "staging_gpu_input_previous",
                });
                ti.recorder.destroy_buffer_deferred(staging_previous);
                auto * previous_ptr = device.buffer_host_address_as<GpuInput>(staging_previous).value();
                *previous_ptr = gpu_input_previous;
                ti.recorder.copy_buffer_to_buffer({
                    .src_buffer = staging_previous,
                    .dst_buffer = gpu_input_buffer_previous,
                    .size = sizeof(GpuInput),
                });

                if(upload_env_map) {
                    upload_env_map = false;
                    // FIXME: upload textures
                    const u32 total_size = ENV_W * ENV_H * sizeof(glm::u8vec4);
                    std::vector<glm::u8vec4> cpu_pixels(total_size);
                    for (u32 y = 0; y < ENV_H; ++y)
                    {
                        float v = float(y) / float(ENV_H - 1);
                        for (u32 x = 0; x < ENV_W; ++x)
                        {
                            // simple gradient sky: top=blue (0.2,0.5,0.8), bottom=dark (0.05,0.05,0.1)
                            glm::vec3 top   = glm::vec3(0.2f, 0.5f, 0.8f);
                            glm::vec3 bot   = glm::vec3(0.05f,0.05f,0.10f);
                            glm::vec3 col   = glm::mix(bot, top, v);
                            cpu_pixels[y * ENV_W + x] = glm::u8vec4(glm::clamp(col * 255.0f, 0.0f, 255.0f), 255u);
                        }
                    }

                    auto staging_env_map_buffer = device.create_buffer({
                        .size = total_size,
                        .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
                        .name = ("staging_env_map_buffer"),
                    });
                    ti.recorder.destroy_buffer_deferred(staging_env_map_buffer);
                    auto * buffer_image_ptr = device.buffer_host_address_as<glm::u8vec4>(staging_env_map_buffer).value();
                    std::memcpy(buffer_image_ptr, cpu_pixels.data(), total_size);
                    ti.recorder.copy_buffer_to_image({
                        .buffer = staging_env_map_buffer,
                        .image = ti.get(task_environment_map_image).ids[0],
                        .image_extent = {ENV_W, ENV_H, 1}
                    });
                }
                
                setup_reflex();
                setup_dlss();
                auto result = streamline.set_constants(setup_constants());
                if (result != DAXA_RESULT_SUCCESS)
                {
                    std::cerr << "Failed to set constants!" << std::endl;
                }

                // FIXME: query reflex stats here
            },
            .name = ("Upload Input"),
        });
        new_task_graph.add_task({
            .attachments = {
                daxa::inl_attachment(daxa::TaskBufferAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_gpu_input_buffer),
                daxa::inl_attachment(daxa::TaskBufferAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_gpu_input_buffer_previous),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_environment_map_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_render_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_diffuse_albedo_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_specular_albedo_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_normal_rougness_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_motion_vector_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_specular_motion_vector_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_depth_image),
            },
            .task = [this](daxa::TaskInterface ti)
            {
                ti.recorder.set_pipeline(*compute_pipeline);
                auto t = this->render_image.default_view();
                auto n = this->normal_roughness_image.default_view();
                auto m = this->motion_vector_image.default_view();
                auto s = this->specular_motion_vector_image.default_view();
                auto d = this->depth_image.default_view();
                auto d_a = this->diffuse_albedo_image.default_view();
                auto s_a = this->specular_albedo_image.default_view();
                auto p = ComputePush{
                    .image_id = t,
                    .diffuse_albedo_id = d_a,
                    .specular_albedo_id = s_a,
                    .normal_roughness_id = n,
                    .mvec_id = m,
                    .spec_mvec_id = s,
                    .depth_id = d,
                    .env_map = ti.get(task_environment_map_image).ids[0].default_view(),
                    .env_sampler = sampler,
                    .ptr = device.device_address(this->gpu_input_buffer).value(),
                    .prev_ptr = device.device_address(this->gpu_input_buffer_previous).value(),
                    .frame_index = frame_count,
                    .render_mode = render_mode,
                };
                ti.recorder.push_constant(p);
                ti.recorder.dispatch({(render_width + 7) / 8, (render_height + 7) / 8});
            },
            .name = ("Draw (Compute)"),
        });
        new_task_graph.add_task({
            .attachments = {
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_render_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_diffuse_albedo_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_specular_albedo_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_normal_rougness_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_motion_vector_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_specular_motion_vector_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_depth_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_output_image),
            },
            .task = [this](daxa::TaskInterface ti)
            {
                if(is_dlss_enabled) {
                    if (!sl_resources.valid || resources_need_resize) {
                        sl_resources.valid = true;

                        sl_resources.input = createStreamlineResource(device, 
                            ti.get(task_render_image).ids[0], 
                            ti.get(task_render_image).ids[0].default_view(), 
                            sl::ResourceType::eTex2d, 
                            sl::kBufferTypeScalingInputColor);

                        sl_resources.diffuse = createStreamlineResource(device, 
                            ti.get(task_diffuse_albedo_image).ids[0], 
                            ti.get(task_diffuse_albedo_image).ids[0].default_view(), 
                            sl::ResourceType::eTex2d, 
                            sl::kBufferTypeAlbedo);
                        
                        sl_resources.specular = createStreamlineResource(device, 
                            ti.get(task_specular_albedo_image).ids[0], 
                            ti.get(task_specular_albedo_image).ids[0].default_view(), 
                            sl::ResourceType::eTex2d, 
                            sl::kBufferTypeSpecularAlbedo);

                        sl_resources.normal_roughness = createStreamlineResource(device, 
                            ti.get(task_normal_rougness_image).ids[0], 
                            ti.get(task_normal_rougness_image).ids[0].default_view(), 
                            sl::ResourceType::eTex2d, 
                            sl::kBufferTypeNormalRoughness);
                        
                        sl_resources.motion = createStreamlineResource(device, 
                            ti.get(task_motion_vector_image).ids[0], 
                            ti.get(task_motion_vector_image).ids[0].default_view(),
                            sl::ResourceType::eTex2d, 
                            sl::kBufferTypeMotionVectors);
                        
                        sl_resources.specular_motion = createStreamlineResource(device, 
                            ti.get(task_specular_motion_vector_image).ids[0], 
                            ti.get(task_specular_motion_vector_image).ids[0].default_view(),
                            sl::ResourceType::eTex2d, 
                            sl::kBufferTypeSpecularMotionVectors);
                        
                        sl_resources.depth = createStreamlineResource(device, 
                            ti.get(task_depth_image).ids[0], 
                            ti.get(task_depth_image).ids[0].default_view(),
                            sl::ResourceType::eTex2d, 
                            sl::kBufferTypeDepth);
                        
                        sl_resources.output = createStreamlineResource(device, 
                            ti.get(task_output_image).ids[0], 
                            ti.get(task_output_image).ids[0].default_view(),
                            sl::ResourceType::eTex2d, 
                            sl::kBufferTypeScalingOutputColor);
                    }

                    sl::Extent input_extent(0, 0, render_width, render_height);
                    sl::Extent output_extent(0, 0, size_x, size_y);
                    
                    const sl::ResourceTag tags[] = {
                        sl::ResourceTag{
                            &sl_resources.input,
                            sl::kBufferTypeScalingInputColor,
                            sl::ResourceLifecycle::eValidUntilPresent,
                            &input_extent
                        },
                        sl::ResourceTag{
                            &sl_resources.diffuse,
                            sl::kBufferTypeAlbedo,
                            sl::ResourceLifecycle::eValidUntilPresent,
                            &input_extent
                        },
                        sl::ResourceTag{
                            &sl_resources.specular,
                            sl::kBufferTypeSpecularAlbedo,
                            sl::ResourceLifecycle::eValidUntilPresent,
                            &input_extent
                        },
                        sl::ResourceTag{
                            &sl_resources.normal_roughness,
                            sl::kBufferTypeNormalRoughness,
                            sl::ResourceLifecycle::eValidUntilPresent,
                            &input_extent
                        },
                        sl::ResourceTag{
                            &sl_resources.motion,
                            sl::kBufferTypeMotionVectors,
                            sl::ResourceLifecycle::eValidUntilPresent,
                            &input_extent
                        },
                        sl::ResourceTag{
                            &sl_resources.specular_motion,
                            sl::kBufferTypeSpecularMotionVectors,
                            sl::ResourceLifecycle::eValidUntilPresent,
                            &input_extent
                        },
                        sl::ResourceTag{
                            &sl_resources.depth,
                            sl::kBufferTypeDepth,
                            sl::ResourceLifecycle::eValidUntilPresent,
                            &input_extent
                        },
                        sl::ResourceTag{
                            &sl_resources.output,
                            sl::kBufferTypeScalingOutputColor,
                            sl::ResourceLifecycle::eValidUntilPresent,
                            &output_extent
                        },
                    };
                    
                    auto cmd = daxa_cmd_get_vk_command_buffer(*reinterpret_cast<daxa_CommandRecorder *>(&ti.recorder));
                    auto result = streamline.set_tag_for_frame(tags, static_cast<u32>(std::size(tags)), cmd);
                    if (result != DAXA_RESULT_SUCCESS)
                    {
                        std::cerr << "Failed to set tag for frame!" << std::endl;
                    }
                }
            },
            .name = "DLSS Tagging",
        });
        new_task_graph.add_task({
            .attachments = {
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_render_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_diffuse_albedo_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_specular_albedo_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_normal_rougness_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_motion_vector_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_specular_motion_vector_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_depth_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_WRITE_ONLY, task_output_image),
            },
            .task = [this](daxa::TaskInterface ti)
            {
                if (is_dlss_enabled)
                {
                    daxa_Result r;
                    if (use_dlss_rr)
                    {
                        r = streamline.evaluate_dlssd(ti.recorder);
                        if (r != DAXA_RESULT_SUCCESS)
                            std::cerr<<"Failed to evaluate DLSS-RR!\n";
                    }
                    else
                    {
                        r = streamline.evaluate_dlss(ti.recorder);
                        if (r != DAXA_RESULT_SUCCESS)
                            std::cerr<<"Failed to evaluate DLSS!\n";
                    }
                }
            },
            .name = "DLSS evaluate",
        });
#if STREALINE_FG_ENABLED == 1
        new_task_graph.add_task({
            .attachments = {
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_output_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_render_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_motion_vector_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_specular_motion_vector_image),
                daxa::inl_attachment(daxa::TaskImageAccess::COMPUTE_SHADER_STORAGE_READ_ONLY, task_depth_image),
            },
            .task = [this](daxa::TaskInterface ti)
            {         
                // Resource tag for DLSS-G
                sl::Extent null_extent{};
                
                sl::ResourceTag fg_tag = {
                    nullptr,
                    sl::kBufferTypeBackbuffer,
                    sl::ResourceLifecycle::eValidUntilPresent,
                    &null_extent
                };
                auto cmd = daxa_cmd_get_vk_command_buffer(*reinterpret_cast<daxa_CommandRecorder *>(&ti.recorder));
                auto result = streamline.set_tag_for_frame(&fg_tag, 1, cmd);
                if (result != DAXA_RESULT_SUCCESS)
                {
                    std::cerr << "Failed to set DLSS-G tag!" << std::endl;
                }
            },
            .name = "DLSS-G Setup",
        });
#endif 
        new_task_graph.add_task({
            .attachments = {
                daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_READ, task_render_image),
                daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_WRITE, task_output_image),
            },
            .task = [this](daxa::TaskInterface ti)
            {
                if(!is_dlss_enabled) {
                    ti.recorder.blit_image_to_image({
                        .src_image = ti.get(task_render_image).ids[0],
                        .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                        .dst_image = ti.get(task_output_image).ids[0],
                        .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                        .src_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                        .dst_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                    });
                }
            },
            .name = "Copy from render to output(no DLSS)",
        });
        new_task_graph.add_task({
            .attachments = {
                daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_READ, task_output_image),
                daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_WRITE, task_swapchain_image),
            },
            .task = [this](daxa::TaskInterface ti)
            {
                ti.recorder.blit_image_to_image({
                    .src_image = ti.get(task_output_image).ids[0],
                    .src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL,
                    .dst_image = ti.get(task_swapchain_image).ids[0],
                    .dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .src_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                    .dst_offsets = {{{0, 0, 0}, {static_cast<i32>(size_x), static_cast<i32>(size_y), 1}}},
                });

                ti.recorder.write_timestamp({
                    .query_pool = timeline_query_pool,
                    .pipeline_stage = daxa::PipelineStageFlagBits::BOTTOM_OF_PIPE,
                    .query_index = 1,
                });

                auto result = streamline.pcl_set_marker(sl::PCLMarker::eRenderSubmitEnd, frame_count);
                if (result != DAXA_RESULT_SUCCESS)
                {
                    std::cerr << "Failed to end rendering!" << std::endl;
                }

                result = streamline.pcl_set_marker(sl::PCLMarker::ePresentStart, frame_count);
                if (result != DAXA_RESULT_SUCCESS)
                {
                    std::cerr << "Failed to start present!" << std::endl;
                }
            },
            .name = "Blit (render to swapchain)",
        });
    }
};

auto main() -> int
{
    App app = {};
    app.setup_reflex();
    if(app.is_dlss_enabled)
        app.setup_dlss();
    app.update_camera();

    while (true)
    {
        if (app.update())
        {
            break;
        }
    }
}
