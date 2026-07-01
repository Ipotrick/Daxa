#pragma once

#if !DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_GLSLANG && !DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SLANG
#error "[build error] You must build Daxa with the DAXA_ENABLE_UTILS_PIPELINE_MANAGER_(GLSLANG|SLANG) CMake option enabled"
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

    // Generic parallel executor interface for pipeline compilation.
    // daxa does not depend on any specific thread pool implementation.
    // user_data: arbitrary pointer forwarded to blocking_parallel_for (e.g. a ThreadPool*).
    // blocking_parallel_for: must call task_fn(task_user_data, i, thread_index) for every i in [0, count),
    //   optionally from multiple threads, and MUST block until all invocations complete.
    //   thread_index identifies which worker executed the task (implementation-defined, e.g. 0..N-1).
    // worker_thread_count: total number of worker threads (used for "x/y active" display).
    // print_fn (optional): called under an internal mutex after each pipeline finishes.
    //   msg: null-terminated progress string, e.g. "[ 42%] OK   my_shader".
    //   thread_index: which worker thread compiled this pipeline.
    //   active_workers: how many workers are still compiling after this one finished.
    //   total_workers: worker_thread_count value passed in.
    struct PipelineManagerParallelInfo
    {
        void * user_data = {};
        void (*blocking_parallel_for)(
            void * user_data,
            u32 count,
            void * task_user_data,
            void (*task_fn)(void * task_user_data, u32 task_index, u32 thread_index)) = {};
        u32 worker_thread_count = 0;
        void * print_user_data = {};
        void (*print_fn)(void * print_user_data, char const * msg, u32 thread_index, u32 active_workers, u32 total_workers) = {};
    };

    // Results returned by compile_pipelines_parallel, in the same order as the input vectors.
    struct PipelineCompileBatch
    {
        std::vector<Result<std::shared_ptr<ComputePipeline>>> compute = {};
        std::vector<Result<std::shared_ptr<RasterPipeline>>> raster = {};
        std::vector<Result<std::shared_ptr<RayTracingPipeline>>> ray_tracing = {};
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
        PipelineManager() = default;
        PipelineManager(PipelineManagerInfo2 info);

        auto add_ray_tracing_pipeline2(RayTracingPipelineCompileInfo2 const & info) -> Result<std::shared_ptr<RayTracingPipeline>>;
        auto add_compute_pipeline2(ComputePipelineCompileInfo2 info) -> Result<std::shared_ptr<ComputePipeline>>;
        auto add_raster_pipeline2(RasterPipelineCompileInfo2 const & info) -> Result<std::shared_ptr<RasterPipeline>>;
        // Compiles all supplied pipelines in parallel using the provided thread pool abstraction.
        // All add_*_pipeline2 calls and compile_pipelines_parallel calls must be externally serialized;
        // only the internal per-pipeline work is parallelised.
        auto compile_pipelines_parallel(
            std::vector<ComputePipelineCompileInfo2> computes,
            std::vector<RasterPipelineCompileInfo2> rasters,
            std::vector<RayTracingPipelineCompileInfo2> ray_tracings,
            PipelineManagerParallelInfo parallel_info) -> PipelineCompileBatch;
        // Like reload_all() but compiles changed pipelines in parallel using the supplied executor.
        auto reload_all_parallel(PipelineManagerParallelInfo parallel_info) -> PipelineReloadResult;
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
