#pragma once

#include <daxa/pipeline.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    using ShaderFileTimeSet = std::map<std::filesystem::path, std::chrono::file_clock::time_point>;

    static inline constexpr usize PIPELINE_COMPILER_MAX_ATTACHMENTS = 16;

    struct ImplPipelineCompiler
    {
        std::weak_ptr<ImplDevice> impl_device = {};
        PipelineCompilerInfo info = {};

        std::vector<std::filesystem::path> current_seen_shader_files = {};
        ShaderFileTimeSet * current_observed_hotload_files = nullptr;

        IDxcUtils * dxc_utils = nullptr;
        IDxcCompiler3 * dxc_compiler = nullptr;
        IDxcIncludeHandler * dxc_includer = nullptr;

        ImplPipelineCompiler(std::weak_ptr<ImplDevice> impl_device, PipelineCompilerInfo const & info);
        ~ImplPipelineCompiler();

        auto get_spirv(ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage) -> Result<std::vector<u32>>;
        auto full_path_to_file(std::filesystem::path const & path) -> Result<std::filesystem::path>;
        auto load_shader_source_from_file(std::filesystem::path const & path) -> Result<ShaderCode>;
        auto gen_spirv_from_dxc(ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage, ShaderCode const & code) -> Result<std::vector<u32>>;
    };

    struct ImplRasterPipeline
    {
        std::weak_ptr<ImplDevice> impl_device;
        RasterPipelineInfo info;
        VkPipeline vk_pipeline = {};
        VkPipelineLayout vk_pipeline_layout = {};
        ShaderFileTimeSet observed_hotload_files = {};
        std::chrono::file_clock::time_point last_hotload_time = {};

        ImplRasterPipeline(std::weak_ptr<ImplDevice> impl_device, RasterPipelineInfo const & info);
        ~ImplRasterPipeline();
    };

    struct ImplComputePipeline
    {
        std::weak_ptr<ImplDevice> impl_device;
        ComputePipelineInfo info;
        VkPipeline vk_pipeline = {};
        VkPipelineLayout vk_pipeline_layout = {};
        ShaderFileTimeSet observed_hotload_files = {};
        std::chrono::file_clock::time_point last_hotload_time = {};

        ImplComputePipeline(std::weak_ptr<ImplDevice> impl_device, ComputePipelineInfo const & info);
        ~ImplComputePipeline();
    };
} // namespace daxa
