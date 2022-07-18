#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    struct ShaderFile
    {
        std::filesystem::path path;
    };
    struct ShaderCode
    {
        std::string string;
    };
    struct ShaderSPIRV
    {
        u32 * data;
        usize size;
    };

    using ShaderSource = std::variant<ShaderFile, ShaderCode, ShaderSPIRV>;

    struct ShaderInfo
    {
        ShaderSource source;
        std::string entry_point = { "main" };
        std::vector<std::string> defines = {};
        std::string debug_name = {};
    };

    struct ComputePipelineInfo
    {
        ShaderInfo shader_info = {};
        u32 push_constant_size = {};
        std::string debug_name = {};
    };

    struct ComputePipeline : Handle
    {
        ~ComputePipeline();

        auto info() const -> ComputePipelineInfo const &;

      private:
        friend struct PipelineCompiler;
        friend struct CommandList;
        ComputePipeline(std::shared_ptr<void> impl);
    };

    struct PipelineCompilerInfo
    {
        std::vector<std::filesystem::path> root_paths = {};
        std::string debug_name = {};
    };

    struct PipelineCompiler : Handle
    {
        auto create_compute_pipeline(ComputePipelineInfo const & info) -> Result<ComputePipeline>;
        auto recreate_compute_pipeline(ComputePipeline const &pipeline) -> Result<ComputePipeline>;
        auto check_if_sources_changed(ComputePipeline const &pipeline) -> bool;

      private:
        friend struct Device;
        PipelineCompiler(std::shared_ptr<void> impl);
    };
} // namespace daxa
