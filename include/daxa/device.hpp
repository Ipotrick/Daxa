#pragma once

#include <daxa/core.hpp>
#include <daxa/gpu_resources.hpp>
#include <daxa/pipeline.hpp>
#include <daxa/swapchain.hpp>
#include <daxa/command_list.hpp>
#include <daxa/sync.hpp>

namespace daxa
{
    enum struct DeviceType
    {
        OTHER = 0,
        INTEGRATED_GPU = 1,
        DISCRETE_GPU = 2,
        VIRTUAL_GPU = 3,
        CPU = 4,
        MAX_ENUM = 0x7fffffff,
    };

    struct DeviceLimits
    {
        u32 max_image_dimension1d;
        u32 max_image_dimension2d;
        u32 max_image_dimension3d;
        u32 max_image_dimension_cube;
        u32 max_image_array_layers;
        u32 max_texel_buffer_elements;
        u32 max_uniform_buffer_range;
        u32 max_storage_buffer_range;
        u32 max_push_constants_size;
        u32 max_memory_allocation_count;
        u32 max_sampler_allocation_count;
        u64 buffer_image_granularity;
        u64 sparse_address_space_size;
        u32 max_bound_descriptor_sets;
        u32 max_per_stage_descriptor_samplers;
        u32 max_per_stage_descriptor_uniform_buffers;
        u32 max_per_stage_descriptor_storage_buffers;
        u32 max_per_stage_descriptor_sampled_images;
        u32 max_per_stage_descriptor_storage_images;
        u32 max_per_stage_descriptor_input_attachments;
        u32 max_per_stage_resources;
        u32 max_descriptor_set_samplers;
        u32 max_descriptor_set_uniform_buffers;
        u32 max_descriptor_set_uniform_buffers_dynamic;
        u32 max_descriptor_set_storage_buffers;
        u32 max_descriptor_set_storage_buffers_dynamic;
        u32 max_descriptor_set_sampled_images;
        u32 max_descriptor_set_storage_images;
        u32 max_descriptor_set_input_attachments;
        u32 max_vertex_input_attributes;
        u32 max_vertex_input_bindings;
        u32 max_vertex_input_attribute_offset;
        u32 max_vertex_input_binding_stride;
        u32 max_vertex_output_components;
        u32 max_tessellation_generation_level;
        u32 max_tessellation_patch_size;
        u32 max_tessellation_control_per_vertex_input_components;
        u32 max_tessellation_control_per_vertex_output_components;
        u32 max_tessellation_control_per_patch_output_components;
        u32 max_tessellation_control_total_output_components;
        u32 max_tessellation_evaluation_input_components;
        u32 max_tessellation_evaluation_output_components;
        u32 max_geometry_shader_invocations;
        u32 max_geometry_input_components;
        u32 max_geometry_output_components;
        u32 max_geometry_output_vertices;
        u32 max_geometry_total_output_components;
        u32 max_fragment_input_components;
        u32 max_fragment_output_attachments;
        u32 max_fragment_dual_src_attachments;
        u32 max_fragment_combined_output_resources;
        u32 max_compute_shared_memory_size;
        u32 max_compute_work_group_count[3];
        u32 max_compute_work_group_invocations;
        u32 max_compute_work_group_size[3];
        u32 sub_pixel_precision_bits;
        u32 sub_texel_precision_bits;
        u32 mipmap_precision_bits;
        u32 max_draw_indexed_index_value;
        u32 max_draw_indirect_count;
        f32 max_sampler_lod_bias;
        f32 max_sampler_anisotropy;
        u32 max_viewports;
        u32 max_viewport_dimensions[2];
        f32 viewport_bounds_range[2];
        u32 viewport_sub_pixel_bits;
        usize min_memory_map_alignment;
        u64 min_texel_buffer_offset_alignment;
        u64 min_uniform_buffer_offset_alignment;
        u64 min_storage_buffer_offset_alignment;
        i32 min_texel_offset;
        u32 max_texel_offset;
        i32 min_texel_gather_offset;
        u32 max_texel_gather_offset;
        f32 min_interpolation_offset;
        f32 max_interpolation_offset;
        u32 sub_pixel_interpolation_offset_bits;
        u32 max_framebuffer_width;
        u32 max_framebuffer_height;
        u32 max_framebuffer_layers;
        u32 framebuffer_color_sample_counts;
        u32 framebuffer_depth_sample_counts;
        u32 framebuffer_stencil_sample_counts;
        u32 framebuffer_no_attachments_sample_counts;
        u32 max_color_attachments;
        u32 sampled_image_color_sample_counts;
        u32 sampled_image_integer_sample_counts;
        u32 sampled_image_depth_sample_counts;
        u32 sampled_image_stencil_sample_counts;
        u32 storage_image_sample_counts;
        u32 max_sample_mask_words;
        u32 timestamp_compute_and_graphics;
        f32 timestamp_period;
        u32 max_clip_distances;
        u32 max_cull_distances;
        u32 max_combined_clip_and_cull_distances;
        u32 discrete_queue_priorities;
        f32 point_size_range[2];
        f32 line_width_range[2];
        f32 point_size_granularity;
        f32 line_width_granularity;
        u32 strict_lines;
        u32 standard_sample_locations;
        u64 optimal_buffer_copy_offset_alignment;
        u64 optimal_buffer_copy_row_pitch_alignment;
        u64 non_coherent_atom_size;
    };

    struct DeviceProperties
    {
        u32 vulkan_api_version = {};
        u32 driver_version = {};
        u32 vendor_id = {};
        u32 device_id = {};
        DeviceType device_type = {};
        u8 device_name[256U] = {};
        u8 pipeline_cache_uuid[16U] = {};
        DeviceLimits limits = {};
    };

    struct MeshShaderDeviceProperties
    {
        u32 max_task_work_group_total_count = {};
        u32 max_task_work_group_count[3] = {};
        u32 max_task_work_group_invocations = {};
        u32 max_task_work_group_size[3] = {};
        u32 max_task_payload_size = {};
        u32 max_task_shared_memory_size = {};
        u32 max_task_payload_and_shared_memory_size = {};
        u32 max_mesh_work_group_total_count = {};
        u32 max_mesh_work_group_count[3] = {};
        u32 max_mesh_work_group_invocations = {};
        u32 max_mesh_work_group_size[3] = {};
        u32 max_mesh_shared_memory_size = {};
        u32 max_mesh_payload_and_shared_memory_size = {};
        u32 max_mesh_output_memory_size = {};
        u32 max_mesh_payload_and_output_memory_size = {};
        u32 max_mesh_output_components = {};
        u32 max_mesh_output_vertices = {};
        u32 max_mesh_output_primitives = {};
        u32 max_mesh_output_layers = {};
        u32 max_mesh_multiview_view_count = {};
        u32 mesh_output_per_vertex_granularity = {};
        u32 mesh_output_per_primitive_granularity = {};
        u32 max_preferred_task_work_group_invocations = {};
        u32 max_preferred_mesh_work_group_invocations = {};
        u32 prefers_local_invocation_vertex_output = {};
        u32 prefers_local_invocation_primitive_output = {};
        u32 prefers_compact_vertex_output = {};
        u32 prefers_compact_primitive_output = {};
    };

    auto default_device_score(DeviceProperties const & device_props) -> i32;

    struct DeviceFlagsProperties
    {
        using Data = u32;
    };
    using DeviceFlags = Flags<DeviceFlagsProperties>;
    struct DeviceFlagBits
    {
        static inline constexpr DeviceFlags NONE = {0x00000000};
        static inline constexpr DeviceFlags BUFFER_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT = {0x1 << 0};
        static inline constexpr DeviceFlags CONSERVATIVE_RASTERIZATION = {0x1 << 1};
        static inline constexpr DeviceFlags MESH_SHADER_BIT = {0x1 << 2};
        static inline constexpr DeviceFlags SHADER_ATOMIC64 = {0x1 << 3};
        static inline constexpr DeviceFlags IMAGE_ATOMIC64 = {0x1 << 4};
        static inline constexpr DeviceFlags VK_MEMORY_MODEL = {0x1 << 5};
    };

    struct DeviceInfo
    {
        i32 (*selector)(DeviceProperties const & properties) = default_device_score;
        DeviceFlags flags =
            DeviceFlagBits::BUFFER_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT |
            DeviceFlagBits::SHADER_ATOMIC64 |
            DeviceFlagBits::IMAGE_ATOMIC64;
        // Make sure your device actually supports the max numbers, as device creation will fail otherwise.
        u32 max_allowed_images = 10'000;
        u32 max_allowed_buffers = 10'000;
        u32 max_allowed_samplers = 400;
        std::string_view name = "";
    };

    struct CommandSubmitInfo
    {
        PipelineStageFlags wait_stages = {};
        std::span<CommandList const> command_lists = {};
        std::span<BinarySemaphore const> wait_binary_semaphores = {};
        std::span<BinarySemaphore const> signal_binary_semaphores = {};
        std::span<std::pair<TimelineSemaphore, u64>> wait_timeline_semaphores = {};
        std::span<std::pair<TimelineSemaphore, u64>> signal_timeline_semaphores = {};
    };

    struct PresentInfo
    {
        std::span<BinarySemaphore const> wait_binary_semaphores = {};
        Swapchain swapchain;
    };

    // TODO: change info functions to return a Optional<Info>
    /**
     * @brief   Device represents a logical device that may be a virtual or physical gpu.
     *          Device manages all general gpu operations that are not handled by other objects.
     *          All objects connected to the device are created by it.
     *
     * THREADSAFETY:
     * * is internally synchronized
     * * can be passed between different threads
     * * may be accessed by multiple threads at the same time
     * * WARNING: there are exceptions to this, those are mentioned above those functions. (TODO REMOVE THE EXCEPTIONS TO THIS)
     */
    struct Device final : ManagedPtr<Device, daxa_Device>
    {
        Device() = default;

        auto create_memory(MemoryBlockInfo const & info) -> MemoryBlock;
        auto get_memory_requirements(BufferInfo const & info) const -> MemoryRequirements;
        auto get_memory_requirements(ImageInfo const & info) const -> MemoryRequirements;

        auto create_buffer(BufferInfo const & info) -> BufferId;
        auto create_image(ImageInfo const & info) -> ImageId;
        auto create_image_view(ImageViewInfo const & info) -> ImageViewId;
        auto create_sampler(SamplerInfo const & info) -> SamplerId;

        void destroy_buffer(BufferId id);
        void destroy_image(ImageId id);
        void destroy_image_view(ImageViewId id);
        void destroy_sampler(SamplerId id);

        /// @brief  Daxa stores each create info and keeps it up to date if the object changes
        ///         This is also the case for gpu resources (buffer, image(view), sampler).
        /// @param id of the object.
        /// WARNING:
        /// * Calling this function with an invalid id will result in a reference pointing to unknown data.
        /// * It is guaranteed that calling this function with an invalid id WILL NOT corrupt or invalidate the device.
        /// THREADSAFETY:
        /// * as long as the id is valid, the returned reference will point to the objects info.
        /// * object MUST NOT be destroyed until the last time the reference is used to read the info.
        /// * destroying the object and still reading the info can lead to race conditions.
        /// * calling the function with an invalid id can lead to race conditions.
        /// @return a reference to the info.
        auto info_buffer(BufferId id) const -> BufferInfo const &;

        /// @brief  Daxa stores each create info and keeps it up to date if the object changes
        ///         This is also the case for gpu resources (buffer, image(view), sampler).
        /// @param id of the object.
        /// WARNING:
        /// * Calling this function with an invalid id will result in a reference pointing to unknown data.
        /// * It is guaranteed that calling this function with an invalid id WILL NOT corrupt or invalidate the device.
        /// THREADSAFETY:
        /// * as long as the id is valid, the returned reference will point to the objects info.
        /// * object MUST NOT be destroyed until the last time the reference is used to read the info.
        /// * destroying the object and still reading the info can lead to race conditions.
        /// * calling the function with an invalid id can lead to race conditions.
        /// @return a reference to the info.
        auto info_image(ImageId id) const -> ImageInfo const &;

        /// @brief  Daxa stores each create info and keeps it up to date if the object changes
        ///         This is also the case for gpu resources (buffer, image(view), sampler).
        /// @param id of the object.
        /// WARNING:
        /// * Calling this function with an invalid id will result in a reference pointing to unknown data.
        /// * It is guaranteed that calling this function with an invalid id WILL NOT corrupt or invalidate the device.
        /// THREADSAFETY:
        /// * as long as the id is valid, the returned reference will point to the objects info.
        /// * object MUST NOT be destroyed until the last time the reference is used to read the info.
        /// * destroying the object and still reading the info can lead to race conditions.
        /// * calling the function with an invalid id can lead to race conditions.
        /// @return a reference to the info.
        auto info_image_view(ImageViewId id) const -> ImageViewInfo const &;

        /// @brief  Daxa stores each create info and keeps it up to date if the object changes
        ///         This is also the case for gpu resources (buffer, image(view), sampler).
        /// @param id of the object.
        /// WARNING:
        /// * Calling this function with an invalid id will result in a reference pointing to unknown data.
        /// * It is guaranteed that calling this function with an invalid id WILL NOT corrupt or invalidate the device.
        /// THREADSAFETY:
        /// * as long as the id is valid, the returned reference will point to the objects info.
        /// * object MUST NOT be destroyed until the last time the reference is used to read the info.
        /// * destroying the object and still reading the info can lead to race conditions.
        /// * calling the function with an invalid id can lead to race conditions.
        /// @return a reference to the info.
        auto info_sampler(SamplerId id) const -> SamplerInfo const &;

        /// @brief  Will describe if a given id is valid.
        ///         An id is valid as long as it was created by the device and not yet destroyed.
        /// @param id or the object.
        /// @return validity of id
        auto is_id_valid(ImageId id) const -> bool;

        /// @brief  Will describe if a given id is valid.
        ///         An id is valid as long as it was created by the device and not yet destroyed.
        /// @param id of the object.
        /// @return validity of id
        auto is_id_valid(ImageViewId id) const -> bool;

        /// @brief  Will describe if a given id is valid.
        ///         An id is valid as long as it was created by the device and not yet destroyed.
        /// @param id of the object.
        /// @return validity of id
        auto is_id_valid(BufferId id) const -> bool;

        /// @brief  Will describe if a given id is valid.
        ///         An id is valid as long as it was created by the device and not yet destroyed.
        /// @param id of the object.
        /// @return validity of id
        auto is_id_valid(SamplerId id) const -> bool;

        auto get_device_address(BufferId id) const -> BufferDeviceAddress;
        auto get_host_address(BufferId id) const -> void *;
        template <typename T>
        auto get_host_address_as(BufferId id) const -> T *
        {
            return static_cast<T *>(get_host_address(id));
        }

        auto create_raster_pipeline(RasterPipelineInfo const & info) -> RasterPipeline;
        auto create_compute_pipeline(ComputePipelineInfo const & info) -> ComputePipeline;

        auto create_swapchain(SwapchainInfo const & info) -> Swapchain;
        auto create_command_list(CommandListInfo const & info) -> CommandList;
        auto create_binary_semaphore(BinarySemaphoreInfo const & info) -> BinarySemaphore;
        auto create_timeline_semaphore(TimelineSemaphoreInfo const & info) -> TimelineSemaphore;
        auto create_event(EventInfo const & info) -> Event;
        auto create_timeline_query_pool(TimelineQueryPoolInfo const & info) -> TimelineQueryPool;

        /// THREADSAFETY:
        /// * reference MUST NOT be read after the device is destroyed.
        /// @return reference to info of object.
        auto info() const -> DeviceInfo const &;
        void wait_idle();

        void submit_commands(CommandSubmitInfo const & submit_info);
        void present_frame(PresentInfo const & info);

        /// @brief  Actually destroyes all resources that are ready to be destroyed.
        ///         When calling destroy, or removing all references to an object, it is zombiefied not really destroyed.
        ///         A zombie lifes until the gpu catches up to the point of zombiefication.
        /// NOTE:
        /// * this function will block until it gains an exlusive resource lock.
        /// * command lists may hold shared lifetime locks, those must all unlock before an exclusive lock can be made.
        /// * look at CommandList for more info on this
        /// * SoftwareCommandList is excempt from this limitation,
        ///   you can freely record those in parallel with collect_garbage.
        void collect_garbage();

        /// THREADSAFETY:
        /// * reference MUST NOT be read after the device is destroyed.
        /// @return reference to device properties
        auto properties() const -> DeviceProperties const &;
        auto get_supported_present_modes(NativeWindowHandle native_handle, NativeWindowPlatform native_platform) const -> std::vector<PresentMode>;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };
} // namespace daxa
