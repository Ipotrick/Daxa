#pragma once

#include <daxa/core.hpp>
#include <daxa/gpu_resources.hpp>
#include <daxa/pipeline.hpp>
#include <daxa/swapchain.hpp>
#include <daxa/command_list.hpp>
#include <daxa/semaphore.hpp>
#include <daxa/split_barrier.hpp>
#include <daxa/timeline_query.hpp>

namespace daxa
{
    enum struct DeviceType
    {
        OTHER = 0,
        INTEGRATED_GPU = 1,
        DISCRETE_GPU = 2,
        VIRTUAL_GPU = 3,
        CPU = 4,
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

    static inline auto default_device_score(DeviceProperties const & device_props) -> i32
    {
        i32 score = 0;
        switch (device_props.device_type)
        {
        case daxa::DeviceType::DISCRETE_GPU: score += 10000; break;
        case daxa::DeviceType::VIRTUAL_GPU: score += 1000; break;
        case daxa::DeviceType::INTEGRATED_GPU: score += 100; break;
        default: break;
        }
        return score;
    }

    struct DeviceInfo
    {
        std::function<i32(DeviceProperties const &)> selector = default_device_score;
        std::string debug_name = {};
    };

    struct CommandSubmitInfo
    {
        PipelineStageFlags src_stages = {};
        std::vector<CommandList> command_lists = {};
        std::vector<BinarySemaphore> wait_binary_semaphores = {};
        std::vector<BinarySemaphore> signal_binary_semaphores = {};
        std::vector<std::pair<TimelineSemaphore, u64>> wait_timeline_semaphores = {};
        std::vector<std::pair<TimelineSemaphore, u64>> signal_timeline_semaphores = {};
    };

    struct PresentInfo
    {
        std::vector<BinarySemaphore> wait_binary_semaphores = {};
        Swapchain swapchain;
    };

    struct Device : ManagedPtr
    {
        Device() = default;

        auto create_buffer(BufferInfo const & info) -> BufferId;
        auto create_image(ImageInfo const & info) -> ImageId;
        auto create_image_view(ImageViewInfo const & info) -> ImageViewId;
        auto create_sampler(SamplerInfo const & info) -> SamplerId;
        auto create_timeline_query_pool(TimelineQueryPoolInfo const & info) -> TimelineQueryPool;

        void destroy_buffer(BufferId id);
        void destroy_image(ImageId id);
        void destroy_image_view(ImageViewId id);
        void destroy_sampler(SamplerId id);

        auto info_buffer(BufferId id) const -> BufferInfo;
        auto get_device_address(BufferId id) const -> BufferDeviceAddress;
        auto get_host_address(BufferId id) const -> void *;
        template <typename T>
        auto get_host_address_as(BufferId id) const -> T *
        {
            return static_cast<T *>(get_host_address(id));
        }
        auto info_image(ImageId id) const -> ImageInfo;
        auto info_image_view(ImageViewId id) const -> ImageViewInfo;
        auto info_sampler(SamplerId id) const -> SamplerInfo;

        // auto create_pipeline_manager(PipelineManagerInfo const & info) -> PipelineManager;
        auto create_raster_pipeline(RasterPipelineInfo const & info) -> RasterPipeline;
        auto create_compute_pipeline(ComputePipelineInfo const & info) -> ComputePipeline;

        auto create_swapchain(SwapchainInfo const & info) -> Swapchain;
        auto create_command_list(CommandListInfo const & info) -> CommandList;
        auto create_binary_semaphore(BinarySemaphoreInfo const & info) -> BinarySemaphore;
        auto create_timeline_semaphore(TimelineSemaphoreInfo const & info) -> TimelineSemaphore;
        auto create_split_barrier(SplitBarrierInfo const & info) -> SplitBarrierState;

        auto info() const -> DeviceInfo const &;
        auto properties() const -> DeviceProperties const &;
        void wait_idle();

        void submit_commands(CommandSubmitInfo const & submit_info);
        void present_frame(PresentInfo const & info);
        void collect_garbage();

        auto is_id_valid(ImageId id) const -> bool;
        auto is_id_valid(BufferId id) const -> bool;
        auto is_id_valid(SamplerId id) const -> bool;

      private:
        friend struct Context;
        Device(ManagedPtr impl);
    };
} // namespace daxa
