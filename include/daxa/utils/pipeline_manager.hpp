#pragma once

#if !DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_GLSLANG && !DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_DXC
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_PIPELINE_MANAGER_(GLSLANG|DXC) CMake option enabled, or request the utils-pipeline-manager-(glslang|dxc) feature in vcpkg"
#endif

#include <daxa/device.hpp>

namespace daxa
{
    struct ShaderFile
    {
        std::filesystem::path path;
    };

    // This string will only work if it is valid GLSL/HLSL
    struct ShaderCode
    {
        std::string string;
    };

    using ShaderSource = std::variant<std::monostate, ShaderFile, ShaderCode, ShaderByteCode>;

    struct ShaderDefine
    {
        std::string name = {};
        std::string value = {};
    };

    enum struct ShaderLanguage
    {
        GLSL,
        HLSL,
        MAX_ENUM = 0x7fffffff,
    };

    struct ShaderModel
    {
        u32 major, minor;
    };

    struct ShaderCompileOptions
    {
        std::optional<std::string> entry_point = {};
        std::vector<std::filesystem::path> root_paths = {};
        std::optional<std::filesystem::path> write_out_preprocessed_code = {};
        std::optional<std::filesystem::path> write_out_shader_binary = {};
        std::optional<ShaderLanguage> language = {};
        std::vector<ShaderDefine> defines = {};
        std::optional<bool> enable_debug_info = {};

        void inherit(ShaderCompileOptions const & other);
    };

    struct ShaderCompileInfo
    {
        ShaderSource source = std::monostate{};
        ShaderCompileOptions compile_options = {};
    };

    struct ComputePipelineCompileInfo
    {
        ShaderCompileInfo shader_info = {};
        u32 push_constant_size = {};
        std::string name = {};
    };

    struct RasterPipelineCompileInfo
    {
        ShaderCompileInfo vertex_shader_info = {};
        std::optional<ShaderCompileInfo> tesselation_control_shader_info = {};
        std::optional<ShaderCompileInfo> tesselation_evaluation_shader_info = {};
        std::optional<ShaderCompileInfo> fragment_shader_info = {};
        std::vector<RenderAttachment> color_attachments = {};
        DepthTestInfo depth_test = {};
        RasterizerInfo raster = {};
        TesselationInfo tesselation = {};
        u32 push_constant_size = {};
        std::string name = {};
    };

    struct PipelineManagerInfo
    {
        Device device;
        ShaderCompileOptions shader_compile_options = {};
        std::string name = {};
    };

    struct VirtualIncludeInfo
    {
        std::string name = {};
        std::string contents = {};
    };

    struct PipelineReloadSuccess
    {
    };
    struct PipelineReloadError
    {
        std::string message;
    };
    using NoPipelineChanged = std::monostate;

    using PipelineReloadResult = std::variant<NoPipelineChanged, PipelineReloadSuccess, PipelineReloadError>;

    struct PipelineManager : ManagedPtr
    {
        PipelineManager() = default;

        PipelineManager(PipelineManagerInfo info);

        auto add_compute_pipeline(ComputePipelineCompileInfo const & info) -> Result<std::shared_ptr<ComputePipeline>>;
        auto add_raster_pipeline(RasterPipelineCompileInfo const & info) -> Result<std::shared_ptr<RasterPipeline>>;
        void remove_compute_pipeline(std::shared_ptr<ComputePipeline> const & pipeline);
        void remove_raster_pipeline(std::shared_ptr<RasterPipeline> const & pipeline);
        void add_virtual_include_file(VirtualIncludeInfo const & info);
        auto reload_all() -> PipelineReloadResult;
    };
} // namespace daxa
