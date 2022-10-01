#pragma once

#include <daxa/pipeline.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    using ShaderFileTimeSet = std::map<std::filesystem::path, std::chrono::file_clock::time_point>;

    static inline constexpr usize PIPELINE_COMPILER_MAX_ATTACHMENTS = 16;

    struct PipelineZombie
    {
        VkPipeline vk_pipeline = {};
    };

    struct ImplPipeline : ManagedSharedState
    {
        explicit ImplPipeline(ManagedWeakPtr impl_device);

        ManagedWeakPtr impl_device;
        VkPipeline vk_pipeline = {};
        VkPipelineLayout vk_pipeline_layout = {};
        ShaderFileTimeSet observed_hotload_files = {};
        std::chrono::file_clock::time_point last_hotload_time = {};

        virtual ~ImplPipeline() override;
    };

    struct ImplPipelineCompiler final : ManagedSharedState
    {
        ManagedWeakPtr impl_device = {};
        PipelineCompilerInfo info = {};

        std::vector<std::filesystem::path> current_seen_shader_files = {};
        ShaderFileTimeSet * current_observed_hotload_files = nullptr;

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
            IDxcIncludeHandler * dxc_includer = nullptr;
        };
        DxcBackend dxc_backend = {};
#endif

        ImplPipelineCompiler(ManagedWeakPtr device_impl, PipelineCompilerInfo info);
        ~ImplPipelineCompiler();

        auto get_spirv(ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage) -> Result<std::vector<u32>>;
        auto full_path_to_file(std::filesystem::path const & path) -> Result<std::filesystem::path>;
        auto load_shader_source_from_file(std::filesystem::path const & path) -> Result<ShaderCode>;

        auto gen_spirv_from_glslang(ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage, ShaderCode const & code) -> Result<std::vector<u32>>;
        auto gen_spirv_from_dxc(ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage, ShaderCode const & code) -> Result<std::vector<u32>>;
    };

    struct ImplRasterPipeline final : ImplPipeline
    {
        RasterPipelineInfo info;

        ImplRasterPipeline(ManagedWeakPtr impl_device, RasterPipelineInfo info);
    };

    struct ImplComputePipeline final : ImplPipeline
    {
        ComputePipelineInfo info;

        ImplComputePipeline(ManagedWeakPtr impl_device, ComputePipelineInfo info);
    };
} // namespace daxa
