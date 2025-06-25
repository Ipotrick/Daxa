#pragma once

#if !DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_GLSLANG && !DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SLANG
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_PIPELINE_MANAGER_(GLSLANG|SLANG) CMake option enabled, or request the utils-pipeline-manager-(glslang|slang) feature in vcpkg"
#endif

#include <daxa/device.hpp>

#include <filesystem>
#include <functional>

namespace daxa
{
    struct ShaderFile
    {
        std::filesystem::path path;
    };

    // This string will only work if it is valid GLSL/SLANG
    struct ShaderCode
    {
        std::string string;
    };

    using ShaderSource = Variant<Monostate, ShaderFile, ShaderCode>;

    struct ShaderDefine
    {
        std::string name = {};
        std::string value = {};
    };

    enum struct ShaderLanguage
    {
        GLSL,
        SLANG,
        MAX_ENUM = 0x7fffffff,
    };

    struct ShaderModel
    {
        u32 major, minor;
    };

#if !DAXA_REMOVE_DEPRECATED
    struct [[deprecated("Use ShaderCompileInfo2 instead, API:3.0.4")]] ShaderCompileOptions
    {
        std::optional<std::string> entry_point = {};
        std::vector<std::filesystem::path> root_paths = {};
        std::optional<std::filesystem::path> write_out_preprocessed_code = {};
        std::optional<std::filesystem::path> write_out_shader_binary = {};
        std::optional<std::filesystem::path> spirv_cache_folder = {};
        std::optional<ShaderLanguage> language = {};
        std::vector<ShaderDefine> defines = {};
        std::optional<bool> enable_debug_info = {};
        std::optional<ShaderCreateFlags> create_flags = {};
        std::optional<u32> required_subgroup_size = {};

        void inherit(ShaderCompileOptions const & other);
    };

    struct [[deprecated("Use ShaderCompileInfo2 instead, API:3.0.4")]] ShaderCompileInfo
    {
        ShaderSource source = Monostate{};
        ShaderCompileOptions compile_options = {};
    };

    struct [[deprecated("Use RayTracingPipelineCompileInfo2 instead, API:3.0.4")]] RayTracingPipelineCompileInfo
    {
        std::vector<ShaderCompileInfo> ray_gen_infos = {};
        std::vector<ShaderCompileInfo> intersection_infos = {};
        std::vector<ShaderCompileInfo> any_hit_infos = {};
        std::vector<ShaderCompileInfo> callable_infos = {};
        std::vector<ShaderCompileInfo> closest_hit_infos = {};
        std::vector<ShaderCompileInfo> miss_hit_infos = {};
        std::vector<RayTracingShaderGroupInfo> shader_groups_infos = {};
        u32 max_ray_recursion_depth = {};
        u32 push_constant_size = {};
        std::string name = {};
    };

    struct [[deprecated("Use ComputePipelineCompileInfo2 instead, API:3.0.4")]] ComputePipelineCompileInfo
    {
        ShaderCompileInfo shader_info = {};
        u32 push_constant_size = {};
        std::string name = {};
    };

    struct [[deprecated("Use RasterPipelineCompileInfo2 instead, API:3.0.4")]] RasterPipelineCompileInfo
    {
        Optional<ShaderCompileInfo> mesh_shader_info = {};
        Optional<ShaderCompileInfo> vertex_shader_info = {};
        Optional<ShaderCompileInfo> tesselation_control_shader_info = {};
        Optional<ShaderCompileInfo> tesselation_evaluation_shader_info = {};
        Optional<ShaderCompileInfo> fragment_shader_info = {};
        Optional<ShaderCompileInfo> task_shader_info = {};
        std::vector<RenderAttachment> color_attachments = {};
        Optional<DepthTestInfo> depth_test = {};
        RasterizerInfo raster = {};
        TesselationInfo tesselation = {};
        u32 push_constant_size = {};
        std::string name = {};
    };

    struct [[deprecated("Use PipelineManagerInfo2 instead, API:3.0.4")]] PipelineManagerInfo
    {
        Device device;
        ShaderCompileOptions shader_compile_options = {};
        bool register_null_pipelines_when_first_compile_fails = false;
        std::function<void(std::string &, std::filesystem::path const & path)> custom_preprocessor = {};
        std::string name = {};
    };
#endif

    struct ShaderCompileInfo2
    {
        ShaderSource source = Monostate{};
        std::optional<std::string> entry_point = {};
        std::optional<ShaderLanguage> language = {};
        std::vector<ShaderDefine> defines = {};
        std::optional<bool> enable_debug_info = {};
        std::optional<ShaderCreateFlags> create_flags = {};
        std::optional<u32> required_subgroup_size = {};
    };

    struct RayTracingPipelineCompileInfo2
    {        
        std::vector<ShaderCompileInfo2> ray_gen_infos = {};
        std::vector<ShaderCompileInfo2> intersection_infos = {};
        std::vector<ShaderCompileInfo2> any_hit_infos = {};
        std::vector<ShaderCompileInfo2> callable_infos = {};
        std::vector<ShaderCompileInfo2> closest_hit_infos = {};
        std::vector<ShaderCompileInfo2> miss_hit_infos = {};
        std::vector<RayTracingShaderGroupInfo> shader_groups_infos = {};
        u32 max_ray_recursion_depth = {};
        u32 push_constant_size = DAXA_MAX_PUSH_CONSTANT_BYTE_SIZE;
        std::string name = {};
    };

    // Future replacement for ComputePipelineCompileInfo on daxa 3.1 release.
    struct ComputePipelineCompileInfo2
    {
        ShaderSource source = Monostate{};
        std::optional<std::string> entry_point = {};
        std::optional<ShaderLanguage> language = {};
        std::vector<ShaderDefine> defines = {};
        std::optional<bool> enable_debug_info = {};
        std::optional<ShaderCreateFlags> create_flags = {};
        std::optional<u32> required_subgroup_size = {};
        u32 push_constant_size = DAXA_MAX_PUSH_CONSTANT_BYTE_SIZE;        
        std::string name = {};
    };

    struct RasterPipelineCompileInfo2
    {
        Optional<ShaderCompileInfo2> mesh_shader_info = {};
        Optional<ShaderCompileInfo2> vertex_shader_info = {};
        Optional<ShaderCompileInfo2> tesselation_control_shader_info = {};
        Optional<ShaderCompileInfo2> tesselation_evaluation_shader_info = {};
        Optional<ShaderCompileInfo2> fragment_shader_info = {};
        Optional<ShaderCompileInfo2> task_shader_info = {};
        std::vector<RenderAttachment> color_attachments = {};
        Optional<DepthTestInfo> depth_test = {};
        RasterizerInfo raster = {};
        TesselationInfo tesselation = {};
        u32 push_constant_size = DAXA_MAX_PUSH_CONSTANT_BYTE_SIZE;
        std::string name = {};
    };

    struct PipelineManagerInfo2
    {
        Device device;
        std::vector<std::filesystem::path> root_paths = {};
        std::optional<std::filesystem::path> write_out_preprocessed_code = {};
        std::optional<std::filesystem::path> write_out_spirv = {};
        std::optional<std::filesystem::path> spirv_cache_folder = {};
        bool register_null_pipelines_when_first_compile_fails = false;
        std::function<void(std::string &, std::filesystem::path const & path)> custom_preprocessor = {};
        std::optional<std::string> default_entry_point = {};
        std::optional<ShaderLanguage> default_language = {};
        std::vector<ShaderDefine> default_defines = {};
        std::optional<bool> default_enable_debug_info = {};
        std::optional<ShaderCreateFlags> default_create_flags = {};
        std::optional<u32> default_required_subgroup_size = {};
        std::string name = {};
    };

    struct VirtualFileInfo
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
    using NoPipelineChanged = Monostate;

    using PipelineReloadResult = Variant<NoPipelineChanged, PipelineReloadSuccess, PipelineReloadError>;

    struct ImplPipelineManager;
    struct DAXA_EXPORT_CXX PipelineManager : ManagedPtr<PipelineManager, ImplPipelineManager *>
    {
#if !DAXA_REMOVE_DEPRECATED
        [[deprecated("Use PipelineManager(PipelineManagerInfo2) instead, API:3.0.4")]] PipelineManager(PipelineManagerInfo info);
        [[deprecated("Use add_ray_tracing_pipeline2 instead, API:3.0.4")]] auto add_ray_tracing_pipeline(RayTracingPipelineCompileInfo const & info) -> Result<std::shared_ptr<RayTracingPipeline>>;
        [[deprecated("Use add_compute_pipeline2 instead, API:3.0.4")]] auto add_compute_pipeline(ComputePipelineCompileInfo info) -> Result<std::shared_ptr<ComputePipeline>>;
        [[deprecated("Use add_raster_pipeline2 instead, API:3.0.4")]] auto add_raster_pipeline(RasterPipelineCompileInfo const & info) -> Result<std::shared_ptr<RasterPipeline>>;
#endif

        PipelineManager() = default;
        PipelineManager(PipelineManagerInfo2 info);

        auto add_ray_tracing_pipeline2(RayTracingPipelineCompileInfo2 const & info) -> Result<std::shared_ptr<RayTracingPipeline>>;
        auto add_compute_pipeline2(ComputePipelineCompileInfo2 info) -> Result<std::shared_ptr<ComputePipeline>>;
        auto add_raster_pipeline2(RasterPipelineCompileInfo2 const & info) -> Result<std::shared_ptr<RasterPipeline>>;
        void remove_ray_tracing_pipeline(std::shared_ptr<RayTracingPipeline> const & pipeline);
        void remove_compute_pipeline(std::shared_ptr<ComputePipeline> const & pipeline);
        void remove_raster_pipeline(std::shared_ptr<RasterPipeline> const & pipeline);
        void add_virtual_file(VirtualFileInfo const & info);
        auto reload_all() -> PipelineReloadResult;
        auto all_pipelines_valid() const -> bool;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };
} // namespace daxa
