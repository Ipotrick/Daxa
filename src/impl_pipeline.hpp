#pragma once

#include <daxa/pipeline.hpp>
#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    struct ImplPipelineCompiler
    {
        std::weak_ptr<ImplDevice> impl_device;
        PipelineCompilerInfo info;

        IDxcUtils * dxc_utils = nullptr;
        IDxcCompiler3 * dxc_compiler = nullptr;
        IDxcIncludeHandler * dxc_include_handler = nullptr;

        ImplPipelineCompiler(std::weak_ptr<ImplDevice> impl_device, PipelineCompilerInfo const & info);
        ~ImplPipelineCompiler();

        auto full_path_to_file(std::filesystem::path const& path) -> Result<std::filesystem::path>;
        auto load_shader_source_from_file(std::filesystem::path const & path) -> ShaderCode;
        auto gen_spirv_from_dxc(ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage, ShaderCode const & code) -> std::vector<u32>;
    };

    struct ImplComputePipeline
    {
        std::weak_ptr<ImplDevice> impl_device;
        ComputePipelineInfo info;
        VkPipeline vk_pipeline_handle = {};
        VkPipelineLayout vk_pipeline_layout_handle = {};

        ImplComputePipeline(std::weak_ptr<ImplDevice> impl_device, ComputePipelineInfo const & info, VkShaderModule vk_shader_module_handle);
        ~ImplComputePipeline();
    };
} // namespace daxa
