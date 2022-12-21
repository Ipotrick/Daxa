#pragma once

#include <daxa/utils/pipeline_manager.hpp>

#if DAXA_BUILT_WITH_DXC
#if defined(_WIN32)
#include "Windows.h"
#include "wrl/client.h"
using namespace Microsoft::WRL;
#include <dxcapi.h>
#else
#define SCARD_E_FILE_NOT_FOUND static_cast<HRESULT>(0x80100024)
#define SCARD_E_INVALID_PARAMETER static_cast<HRESULT>(0x80100004)
#include <dxc/dxcapi.h>
template <typename T>
using ComPtr = CComPtr<T>;
#endif
#endif

#if DAXA_BUILT_WITH_GLSLANG
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Include/ResourceLimits.h>
#endif

namespace daxa
{
    struct ImplDevice;

    using ShaderFileTimeSet = std::map<std::filesystem::path, std::chrono::file_clock::time_point>;

    struct ImplPipelineManager final : ManagedSharedState
    {
        enum class ShaderStage
        {
            COMP,
            VERT,
            FRAG,
        };

        PipelineManagerInfo info = {};

        std::vector<std::filesystem::path> current_seen_shader_files = {};
        ShaderFileTimeSet * current_observed_hotload_files = nullptr;

        template <typename PipeT, typename InfoT>
        struct PipelineState
        {
            std::shared_ptr<PipeT> pipeline_ptr;
            InfoT info;
            std::chrono::file_clock::time_point last_hotload_time = {};
            ShaderFileTimeSet observed_hotload_files = {};
        };

        using ComputePipelineState = PipelineState<ComputePipeline, ComputePipelineCompileInfo>;
        using RasterPipelineState = PipelineState<RasterPipeline, RasterPipelineCompileInfo>;

        std::vector<ComputePipelineState> compute_pipelines;
        std::vector<RasterPipelineState> raster_pipelines;

        // TODO: Make the pipeline compiler thread-safe!
        // This is accessed by the includer, which makes that not thread-safe
        ShaderCompileInfo const * current_shader_info = nullptr;

#if DAXA_BUILT_WITH_GLSLANG
        struct GlslangBackend
        {
        };
        GlslangBackend glslang_backend = {};
#endif

#if DAXA_BUILT_WITH_DXC
        struct DxcBackend
        {
            IDxcUtils * dxc_utils = nullptr;
            IDxcCompiler3 * dxc_compiler = nullptr;
            std::shared_ptr<IDxcIncludeHandler> dxc_includer = nullptr;
        };
        DxcBackend dxc_backend = {};
#endif

        ImplPipelineManager(PipelineManagerInfo && a_info);
        ~ImplPipelineManager();

        auto create_compute_pipeline(ComputePipelineCompileInfo const & a_info) -> Result<ComputePipelineState>;
        auto create_raster_pipeline(RasterPipelineCompileInfo const & a_info) -> Result<RasterPipelineState>;
        auto add_compute_pipeline(ComputePipelineCompileInfo const & a_info) -> Result<std::shared_ptr<ComputePipeline>>;
        auto add_raster_pipeline(RasterPipelineCompileInfo const & a_info) -> Result<std::shared_ptr<RasterPipeline>>;
        void remove_compute_pipeline(std::shared_ptr<ComputePipeline> const & pipeline);
        void remove_raster_pipeline(std::shared_ptr<RasterPipeline> const & pipeline);
        auto reload_all() -> Result<bool>;

        auto full_path_to_file(std::filesystem::path const & path) -> Result<std::filesystem::path>;
        auto load_shader_source_from_file(std::filesystem::path const & path) -> Result<ShaderCode>;

        auto get_spirv(ShaderCompileInfo const & shader_info, ShaderStage shader_stage) -> Result<std::vector<u32>>;
        auto get_spirv_glslang(ShaderCompileInfo const & shader_info, ShaderStage shader_stage, ShaderCode const & code) -> Result<std::vector<u32>>;
        auto get_spirv_dxc(ShaderCompileInfo const & shader_info, ShaderStage shader_stage, ShaderCode const & code) -> Result<std::vector<u32>>;
    };
} // namespace daxa
