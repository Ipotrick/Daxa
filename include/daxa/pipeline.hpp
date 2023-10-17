#pragma once

#include <daxa/core.hpp>
#include <daxa/types.hpp>

namespace daxa
{
    struct ShaderInfo
    {
        u32 const * byte_code;
        u32 byte_code_size;
        std::string_view entry_point = "main";
    };

    struct ComputePipelineInfo
    {
        ShaderInfo shader_info = {};
        u32 push_constant_size = {};
        std::string_view name = "";
    };

    /**
     * @brief   Represents a pipeline state object, usable in recording commands.
     * 
     * THREADSAFETY: 
     * * is internally synchronized
     * * may be passed to different threads
     * * may be used by multiple threads at the same time.
    */
    struct ComputePipeline final : ManagedPtr<ComputePipeline, daxa_ComputePipeline>
    {
        ComputePipeline() = default;

        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is dropped.
        /// @return reference to info of object.
        auto info() const -> ComputePipelineInfo const &;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
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
        std::string_view name = "";
    };

    /**
     * @brief   Represents a pipeline state object, usable in recording commands.
     * 
     * THREADSAFETY: 
     * * is internally synchronized
     * * may be passed to different threads
     * * may be used by multiple threads at the same time.
    */
    struct RasterPipeline final : ManagedPtr<RasterPipeline, daxa_RasterPipeline>
    {
        RasterPipeline() = default;

        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is dropped.
        /// @return reference to info of object.
        auto info() const -> RasterPipelineInfo const &;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };
} // namespace daxa
