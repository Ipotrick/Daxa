#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    using ShaderBinary = std::vector<u32>;

    struct ShaderInfo
    {
        ShaderBinary binary = {};
        std::optional<std::string> entry_point = {};
    };

    struct ComputePipelineInfo
    {
        ShaderInfo shader_info = {};
        u32 push_constant_size = {};
        std::string debug_name = {};
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
        bool enable_depth_test = false;
        bool enable_depth_write = false;
        CompareOp depth_test_compare_op = CompareOp::LESS_OR_EQUAL;
        f32 min_depth_bounds = 0.0f;
        f32 max_depth_bounds = 1.0f;
    };

    struct RasterizerInfo
    {
        PrimitiveTopology primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
        bool primitive_restart_enable = false;
        PolygonMode polygon_mode = PolygonMode::FILL;
        FaceCullFlags face_culling = FaceCullFlagBits::NONE;
        FrontFaceWinding front_face_winding = FrontFaceWinding::CLOCKWISE;
        bool depth_clamp_enable = false;
        bool rasterizer_discard_enable = false;
        bool depth_bias_enable = false;
        f32 depth_bias_constant_factor = 0.0f;
        f32 depth_bias_clamp = 0.0f;
        f32 depth_bias_slope_factor = 0.0f;
        f32 line_width = 1.0f;
    };

    struct RenderAttachment
    {
        Format format = {};
        BlendInfo blend = {};
    };

    struct RasterPipelineInfo
    {
        ShaderInfo vertex_shader_info = {};
        ShaderInfo fragment_shader_info = {};
        std::vector<RenderAttachment> color_attachments = {};
        DepthTestInfo depth_test = {};
        RasterizerInfo raster = {};
        u32 push_constant_size = {};
        std::string debug_name = {};
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
