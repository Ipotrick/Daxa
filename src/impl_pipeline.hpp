#pragma once

#include <daxa/pipeline.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    using ShaderFileTimeSet = std::map<std::filesystem::path, std::chrono::file_clock::time_point>;

    static inline constexpr usize PIPELINE_COMPILER_MAX_ATTACHMENTS = 16;

    struct ImplPipelineCompiler final : ManagedSharedState
    {
        ManagedWeakPtr impl_device = {};
        PipelineCompilerInfo info = {};

        std::vector<std::filesystem::path> current_seen_shader_files = {};
        ShaderFileTimeSet * current_observed_hotload_files = nullptr;

        struct ShadercBackend
        {
            shaderc::Compiler compiler = {};
            shaderc::CompileOptions options = {};
        };
        ShadercBackend shaderc_backend = {};

        ImplPipelineCompiler(ManagedWeakPtr impl_device, PipelineCompilerInfo const & info);
        ~ImplPipelineCompiler();

        auto get_spirv(ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage) -> Result<std::vector<u32>>;
        auto full_path_to_file(std::filesystem::path const & path) -> Result<std::filesystem::path>;
        auto load_shader_source_from_file(std::filesystem::path const & path) -> Result<ShaderCode>;
        auto gen_spirv_from_shaderc(ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage, ShaderCode const & code) -> Result<std::vector<u32>>;
    };

    struct ImplRasterPipeline final : ManagedSharedState
    {
        ManagedWeakPtr impl_device;
        RasterPipelineInfo info;
        VkPipeline vk_pipeline = {};
        VkPipelineLayout vk_pipeline_layout = {};
        ShaderFileTimeSet observed_hotload_files = {};
        std::chrono::file_clock::time_point last_hotload_time = {};

        ImplRasterPipeline(ManagedWeakPtr impl_device, RasterPipelineInfo const & info);
        virtual ~ImplRasterPipeline() override final;

        auto managed_cleanup() -> bool override final;
    };

    struct ImplComputePipeline final : ManagedSharedState
    {
        ManagedWeakPtr impl_device;
        ComputePipelineInfo info;
        VkPipeline vk_pipeline = {};
        VkPipelineLayout vk_pipeline_layout = {};
        ShaderFileTimeSet observed_hotload_files = {};
        std::chrono::file_clock::time_point last_hotload_time = {};

        ImplComputePipeline(ManagedWeakPtr impl_device, ComputePipelineInfo const & info);
        virtual ~ImplComputePipeline() override final;

        auto managed_cleanup() -> bool override final;
    };
} // namespace daxa
