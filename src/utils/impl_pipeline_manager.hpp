#pragma once

#include "../impl_core.hpp"

#include <daxa/utils/pipeline_manager.hpp>

#undef Bool

#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SLANG
#include <slang.h>
#include <slang-com-ptr.h>
#endif

#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_GLSLANG
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Include/ResourceLimits.h>
#endif

#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SPIRV_VALIDATION
#include <spirv-tools/libspirv.hpp>
#endif

#include <shared_mutex>
#include <unordered_map>
#include <optional>

namespace daxa
{
    struct ImplDevice;

    using ShaderFileTimeSet = std::map<std::filesystem::path, std::chrono::file_clock::time_point>;

    struct VirtualFileState
    {
        std::string contents;
        std::chrono::file_clock::time_point timestamp;
    };

    using VirtualFileSet = std::map<std::string, VirtualFileState>;

    struct ImplPipelineManager final : ImplHandle
    {
        enum class ShaderStage
        {
            COMP,
            VERT,
            FRAG,
            TESS_CONTROL,
            TESS_EVAL,
            TASK,
            MESH,
            RAY_GEN,
            RAY_INTERSECT,
            RAY_ANY_HIT,
            RAY_CLOSEST_HIT,
            RAY_MISS,
            RAY_CALLABLE,
        };

        PipelineManagerInfo2 info = {};

        std::vector<std::filesystem::path> current_seen_shader_files = {};
        // These two are per-compile-call state. Made static thread_local so that concurrent
        // per-pipeline compiles inside compile_pipelines_parallel each get their own copy.
        static thread_local ShaderFileTimeSet * current_observed_hotload_files;
        static thread_local ShaderCompileInfo2 const * current_shader_info;
        static thread_local bool tl_last_spirv_from_cache;

        VirtualFileSet virtual_files = {};

        // Populated only during compile_pipelines_parallel; caches file contents so that
        // header files included by many pipelines are only read from disk once.
        struct FileCacheEntry
        {
            ShaderCode code = {};
            std::filesystem::file_time_type write_time = {};
        };
        struct FileCache
        {
            std::shared_mutex mtx = {};
            std::unordered_map<std::string, FileCacheEntry> files = {};
        };
        std::optional<FileCache> parallel_file_cache = {};
        // Set for the duration of compile_pipelines_parallel / reload_all_parallel.
        // Allows create_raster/rt_pipeline to fan out their per-stage get_spirv calls
        // and to share the outer ParallelState's print mutex for interleave-free output.
        PipelineManagerParallelInfo const * current_parallel_info = nullptr;
        std::mutex * current_print_mtx = nullptr;
        std::atomic<u32> * current_completed_stages = nullptr;
        std::atomic<u32> * current_stage_cache_hits = nullptr;
        u32 current_total_stages = 0;

        template <typename PipeT, typename InfoT>
        struct PipelineState
        {
            std::shared_ptr<PipeT> pipeline_ptr = {};
            InfoT info = {};
            std::chrono::file_clock::time_point last_hotload_time = {};
            ShaderFileTimeSet observed_hotload_files = {};
        };

        using ComputePipelineState = PipelineState<ComputePipeline, ComputePipelineCompileInfo2>;
        using RasterPipelineState = PipelineState<RasterPipeline, RasterPipelineCompileInfo2>;
        using RayTracingPipelineState = PipelineState<RayTracingPipeline, RayTracingPipelineCompileInfo2>;

        std::vector<ComputePipelineState> compute_pipelines;
        std::vector<RasterPipelineState> raster_pipelines;
        std::vector<RayTracingPipelineState> ray_tracing_pipelines;

#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_GLSLANG
        struct GlslangBackend
        {
        };
        GlslangBackend glslang_backend = {};
#endif

#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SLANG
        struct SlangBackend
        {
            // NOTE(grundlett): literally global session. Not sure if this is reasonable or not.
            static inline Slang::ComPtr<slang::IGlobalSession> global_session;
            static inline std::mutex session_mtx = {};
        };
        SlangBackend slang_backend = {};
#endif

#if DAXA_BUILT_WITH_UTILS_PIPELINE_MANAGER_SPIRV_VALIDATION
        spvtools::SpirvTools spirv_tools = spvtools::SpirvTools{SPV_ENV_VULKAN_1_3};
#endif

        ImplPipelineManager(PipelineManagerInfo2 && a_info);
        ~ImplPipelineManager();

        auto create_ray_tracing_pipeline(RayTracingPipelineCompileInfo2 const & a_info) -> Result<RayTracingPipelineState>;
        auto create_compute_pipeline(ComputePipelineCompileInfo2 const & a_info) -> Result<ComputePipelineState>;
        auto create_raster_pipeline(RasterPipelineCompileInfo2 const & a_info) -> Result<RasterPipelineState>;
        void remove_ray_tracing_pipeline(std::shared_ptr<RayTracingPipeline> const & pipeline);
        void remove_compute_pipeline(std::shared_ptr<ComputePipeline> const & pipeline);
        void remove_raster_pipeline(std::shared_ptr<RasterPipeline> const & pipeline);
        void add_virtual_file(VirtualFileInfo const & virtual_info);
        auto reload_all() -> PipelineReloadResult;
        auto all_pipelines_valid() const -> bool;

        auto try_load_shader_cache(std::filesystem::path const & cache_folder, uint64_t shader_info_hash) -> Result<std::vector<u32>>;
        void save_shader_cache(std::filesystem::path const & out_folder, uint64_t shader_info_hash, std::vector<u32> const & spirv);
        auto full_path_to_file(std::filesystem::path const & path) -> Result<std::filesystem::path>;
        auto load_shader_source_from_file(std::filesystem::path const & path) -> Result<ShaderCode>;


        auto hash_shader_info(std::string const & source_string, ShaderCompileInfo2 const & compile_options, ImplPipelineManager::ShaderStage shader_stage) -> uint64_t;
        auto get_spirv(ShaderCompileInfo2 const & shader_info, std::string const & debug_name_opt, ShaderStage shader_stage) -> Result<std::vector<u32>>;
        auto get_spirv_glslang(ShaderCompileInfo2 const & shader_info, std::string const & debug_name_opt, ShaderStage shader_stage, ShaderCode const & code) -> Result<std::vector<u32>>;
        auto get_spirv_slang(ShaderCompileInfo2 const & shader_info, ShaderStage shader_stage, ShaderCode const & code) -> Result<std::vector<u32>>;

        static auto zero_ref_callback(ImplHandle const * handle);
    };
} // namespace daxa
