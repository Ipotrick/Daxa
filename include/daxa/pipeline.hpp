#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    struct ShaderFile
    {
        std::filesystem::path path;
    };

    // This string will only work if it is valid GLSL
    struct ShaderCode
    {
        std::string string;
    };

    struct ShaderSPIRV
    {
        u32 const * data;
        usize size;
    };

    using ShaderSource = std::variant<std::monostate, ShaderFile, ShaderCode, ShaderSPIRV>;

    struct ShaderDefine
    {
        std::string name = {};
        std::string value = {};
    };

    enum struct ShaderLanguage
    {
        GLSL,
        HLSL,
    };

    struct ShaderModel
    {
        u32 major, minor;
    };

    struct ShaderCompileOptions
    {
        std::optional<std::string> entry_point = {};
        std::vector<std::filesystem::path> root_paths = {};
        std::optional<u32> opt_level = {};
        std::optional<ShaderModel> shader_model = {};
        std::optional<ShaderLanguage> language = {};
        std::vector<ShaderDefine> defines = {};

        void inherit(ShaderCompileOptions const & other);
    };

    struct ShaderInfo
    {
        ShaderSource source = std::monostate{};
        ShaderCompileOptions compile_options = {};
        std::string debug_name = {};
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
        friend struct PipelineCompiler;
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
        PolygonMode polygon_mode = PolygonMode::FILL;
        FaceCullFlags face_culling = FaceCullFlagBits::NONE;
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
        friend struct PipelineCompiler;
        friend struct CommandList;
        explicit RasterPipeline(ManagedPtr impl);
    };

    struct PipelineCompilerInfo
    {
        ShaderCompileOptions shader_compile_options = {};
        std::string debug_name = {};
    };

    struct PipelineCompiler : ManagedPtr
    {
        PipelineCompiler() = default;

        auto create_raster_pipeline(RasterPipelineInfo const & info) -> Result<RasterPipeline>;
        auto recreate_raster_pipeline(RasterPipeline const & pipeline) -> Result<RasterPipeline>;
        auto create_compute_pipeline(ComputePipelineInfo const & info) -> Result<ComputePipeline>;
        auto recreate_compute_pipeline(ComputePipeline const & pipeline) -> Result<ComputePipeline>;
        auto check_if_sources_changed(RasterPipeline & pipeline) -> bool;
        auto check_if_sources_changed(ComputePipeline & pipeline) -> bool;

      private:
        friend struct Device;
        explicit PipelineCompiler(ManagedPtr impl);
    };
} // namespace daxa
