#pragma once

#include <daxa/core.hpp>
#include <daxa/c/types.h>

namespace daxa
{
    struct ShaderInfo
    {
        u32 const * byte_code;
        usize byte_code_size;
        daxa_StringView entry_point = {};
    };

    struct ComputePipelineInfo
    {
        ShaderInfo shader_info = {};
        u32 push_constant_size = {};
        daxa_StringView name = {};
    };

    struct ComputePipeline : ManagedPtr
    {
        ComputePipeline() = default;

        auto info() const -> ComputePipelineInfo const &;

      private:
        friend struct Device;
        friend struct CommandList;
        explicit ComputePipeline(ManagedPtr impl);
    };

    struct DepthTestInfo
    {
        Format depth_attachment_format = Format::UNDEFINED;
        bool enable_depth_write = {};
        CompareOp depth_test_compare_op = CompareOp::LESS_OR_EQUAL;
        f32 min_depth_bounds = 0.0f;
        f32 max_depth_bounds = 1.0f;
    };

    struct ConservativeRasterInfo
    {
        ConservativeRasterizationMode mode = {};
        float size = {};
    };

    struct RasterizerInfo
    {
        PrimitiveTopology primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
        bool primitive_restart_enable = {};
        PolygonMode polygon_mode = PolygonMode::FILL;
        FaceCullFlags face_culling = FaceCullFlagBits::NONE;
        FrontFaceWinding front_face_winding = FrontFaceWinding::CLOCKWISE;
        bool depth_clamp_enable = {};
        bool rasterizer_discard_enable = {};
        bool depth_bias_enable = {};
        f32 depth_bias_constant_factor = 0.0f;
        f32 depth_bias_clamp = 0.0f;
        f32 depth_bias_slope_factor = 0.0f;
        f32 line_width = 1.0f;
        Optional<ConservativeRasterInfo> conservative_raster_info = {};
    };

    struct RenderAttachment
    {
        Format format = {};
        Optional<BlendInfo> blend = {};
    };

    struct TesselationInfo
    {
        u32 control_points = 3;
        TesselationDomainOrigin origin = {};
    };

    struct RasterPipelineInfo
    {
        Optional<ShaderInfo> mesh_shader_info = {};
        Optional<ShaderInfo> vertex_shader_info = {};
        Optional<ShaderInfo> tesselation_control_shader_info = {};
        Optional<ShaderInfo> tesselation_evaluation_shader_info = {};
        Optional<ShaderInfo> fragment_shader_info = {};
        Optional<ShaderInfo> task_shader_info = {};
        FixedList<RenderAttachment, 8> color_attachments = {};
        Optional<DepthTestInfo> depth_test = {};
        Optional<TesselationInfo> tesselation = {};
        RasterizerInfo raster = {};
        u32 push_constant_size = {};
        std::string_view name = {};
    };

    struct RasterPipeline : ManagedPtr
    {
        RasterPipeline() = default;

        auto info() const -> RasterPipelineInfo const &;

      private:
        friend struct Device;
        friend struct CommandList;
        explicit RasterPipeline(ManagedPtr impl);
    };
} // namespace daxa
