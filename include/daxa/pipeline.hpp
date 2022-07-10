#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    struct ShaderInfo
    {
        std::string inline_source = {};
        std::string path_to_source = {};
        std::string entry_point = {};
        std::vector<std::string> defines = {};
        std::string debug_name = {};
    };

    struct GraphicsPipelineInfo
    {
        std::optional<ShaderInfo> vertex_shader = {};
        std::optional<ShaderInfo> fragment_shader = {};
        std::vector<std::pair<Format, std::optional<BlendInfo>>> color_attachments = {};
        u32 push_constant_size = {};
        std::string debug_name = {};
    };

    struct GraphicsPipeline : Handle
    {
        auto info() const -> GraphicsPipelineInfo const &;
    };

    struct ComputePipelineInfo
    {
        ShaderInfo shader_info = {};
        u32 push_constant_size = {};
        std::string debug_name = {};
    };

    struct ComputePipeline : Handle
    {
        auto info() const -> ComputePipelineInfo const &;
    };

    struct PipelineCompilerInfo
    {
        std::vector<std::filesystem::path> root_paths = {};
        std::string debug_name = {};
    };

    struct PipelineCompiler : Handle
    {
        auto create_graphics_pipeline(GraphicsPipelineInfo const & info) -> Result<GraphicsPipeline>;
        auto create_compute_pipeline(ComputePipelineInfo const & info) -> Result<ComputePipeline>;
    };
} // namespace daxa