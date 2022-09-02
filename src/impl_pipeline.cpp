#include "impl_pipeline.hpp"
#include "impl_swapchain.hpp"
#include "impl_device.hpp"

#include <regex>
#include <thread>

static const std::regex PRAGMA_ONCE_REGEX = std::regex(R"reg(#\s*pragma\s*once\s*)reg");
static const std::regex REPLACE_REGEX = std::regex(R"reg(\W)reg");
static void shader_preprocess(std::string & file_str, std::filesystem::path const & path)
{
    std::smatch matches = {};
    std::string line = {};
    std::stringstream file_ss{file_str};
    std::stringstream result_ss = {};
    bool has_pragma_once = false;
    auto abspath_str = std::filesystem::absolute(path).string();
    for (daxa::usize line_num = 0; std::getline(file_ss, line); ++line_num)
    {
        if (std::regex_match(line, matches, PRAGMA_ONCE_REGEX))
        {
            result_ss << "#if !defined(";
            std::regex_replace(std::ostreambuf_iterator<char>(result_ss), abspath_str.begin(), abspath_str.end(), REPLACE_REGEX, "");
            result_ss << ")\n";
            has_pragma_once = true;
        }
        else
        {
            result_ss << line << "\n";
        }
    }
    if (has_pragma_once)
    {
        result_ss << "\n#define ";
        std::regex_replace(std::ostreambuf_iterator<char>(result_ss), abspath_str.begin(), abspath_str.end(), REPLACE_REGEX, "");
        result_ss << "\n#endif\n";
    }
    file_str = result_ss.str();
}

namespace daxa
{
    class ShadercFileIncluder : public shaderc::CompileOptions::IncluderInterface
    {
      public:
        constexpr static inline size_t DELETE_SOURCE_NAME = 0x1;
        constexpr static inline size_t DELETE_CONTENT = 0x2;

        ImplPipelineCompiler * impl_pipeline_compiler;

        virtual shaderc_include_result * GetInclude(
            const char * requested_source,
            shaderc_include_type type,
            const char * requesting_source,
            size_t include_depth) override
        {
            shaderc_include_result * res = new shaderc_include_result{};
            if (include_depth <= 10)
            {
                res->source_name = requested_source;
                res->source_name_length = strlen(requested_source);
                res->user_data = 0;

                auto result = impl_pipeline_compiler->full_path_to_file(requested_source);
                if (result.is_err())
                {
                    res->content = "#error could not find file";
                    res->content_length = strlen(res->content);

                    // res->source_name = nullptr;
                    // res->source_name_length = 0;
                    return res;
                }

                auto full_path = result.value();
                auto search_pred = [&](std::filesystem::path const & p)
                { return p == full_path; };

                if (std::find_if(
                        impl_pipeline_compiler->current_seen_shader_files.begin(),
                        impl_pipeline_compiler->current_seen_shader_files.end(),
                        search_pred) != impl_pipeline_compiler->current_seen_shader_files.end())
                {
                    // Return empty string blob if this file has been included before
                    static const char null_str[] = " ";
                    res->content = null_str;
                    res->content_length = 0;
                    return res;
                }

                impl_pipeline_compiler->current_observed_hotload_files->insert({full_path, std::chrono::file_clock::now()});
                auto shadercode_result = impl_pipeline_compiler->load_shader_source_from_file(full_path);

                if (shadercode_result.is_err())
                {
                    res->content = "#error could not load shader source";
                    res->content_length = strlen(res->content);

                    res->source_name = nullptr;
                    res->source_name_length = 0;
                    return res;
                }

                auto & shadercode_str = shadercode_result.value().string;
                res->content_length = shadercode_str.size();
                char * res_content = new char[res->content_length + 1];
                for (usize i = 0; i < res->content_length; ++i)
                {
                    res_content[i] = shadercode_str[i];
                }
                res_content[res->content_length] = '\0';
                res->content = res_content;

                auto full_path_str = full_path.string();
                res->source_name_length = full_path_str.size();
                char * res_source_name = new char[res->source_name_length + 1];
                for (usize i = 0; i < res->source_name_length; ++i)
                {
                    auto c = full_path_str[i];
                    if (c == '\\')
                        c = '/';
                    res_source_name[i] = c;
                }
                res_source_name[res->source_name_length] = '\0';
                res->source_name = res_source_name;

                res->user_data = reinterpret_cast<void *>(reinterpret_cast<size_t>(res->user_data) | DELETE_CONTENT);
                res->user_data = reinterpret_cast<void *>(reinterpret_cast<size_t>(res->user_data) | DELETE_SOURCE_NAME);
            }
            else
            {
                // max include depth exceeded
                res->content = "current include depth of 10 was exceeded";
                res->content_length = std::strlen(res->content);

                res->source_name = nullptr;
                res->source_name_length = 0;
            }
            return res;
        }

        virtual void ReleaseInclude(shaderc_include_result * data) override
        {
            if (data)
            {
                if (reinterpret_cast<size_t>(data->user_data) & DELETE_CONTENT)
                {
                    delete data->content;
                }
                if (reinterpret_cast<size_t>(data->user_data) & DELETE_SOURCE_NAME)
                {
                    delete data->source_name;
                }
                delete data;
            }
        };
    };

    RasterPipeline::RasterPipeline(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    ComputePipeline::ComputePipeline(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    PipelineCompiler::PipelineCompiler(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto PipelineCompiler::create_raster_pipeline(RasterPipelineInfo const & info) -> Result<RasterPipeline>
    {
        auto & impl = *as<ImplPipelineCompiler>();

        if (info.push_constant_size > MAX_PUSH_CONSTANT_BYTE_SIZE)
        {
            return ResultErr{std::string("push constant size of ") + std::to_string(info.push_constant_size) + std::string(" exceeds the maximum size of ") + std::to_string(MAX_PUSH_CONSTANT_BYTE_SIZE)};
        }
        if (info.push_constant_size % 4 != 0)
        {
            return ResultErr{std::string("push constant size of ") + std::to_string(info.push_constant_size) + std::string(" is not a multiple of 4(bytes)")};
        }

        auto impl_pipeline = new ImplRasterPipeline(impl.impl_device, info);
        impl.current_observed_hotload_files = &impl_pipeline->observed_hotload_files;

        auto v_spirv_result = impl.get_spirv(info.vertex_shader_info, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
        if (v_spirv_result.is_err())
        {
            return ResultErr{.message = v_spirv_result.message()};
        }
        std::vector<u32> v_spirv = v_spirv_result.value();

        auto p_spirv_result = impl.get_spirv(info.fragment_shader_info, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
        if (p_spirv_result.is_err())
        {
            return ResultErr{.message = p_spirv_result.message()};
        }
        std::vector<u32> p_spirv = p_spirv_result.value();

        VkShaderModule v_vk_shader_module = {};
        VkShaderModule p_vk_shader_module = {};

        VkShaderModuleCreateInfo shader_module_vertex{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .codeSize = static_cast<u32>(v_spirv.size() * sizeof(u32)),
            .pCode = v_spirv.data(),
        };
        vkCreateShaderModule(impl.impl_device.as<ImplDevice>()->vk_device, &shader_module_vertex, nullptr, &v_vk_shader_module);

        VkShaderModuleCreateInfo shader_module_pixel{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .codeSize = static_cast<u32>(p_spirv.size() * sizeof(u32)),
            .pCode = p_spirv.data(),
        };
        vkCreateShaderModule(impl.impl_device.as<ImplDevice>()->vk_device, &shader_module_pixel, nullptr, &p_vk_shader_module);

        impl_pipeline->vk_pipeline_layout = impl.impl_device.as<ImplDevice>()->gpu_table.pipeline_layouts[(info.push_constant_size + 3) / 4];

        VkPipelineShaderStageCreateInfo vk_pipeline_shader_stage_create_infos[2] = {
            VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
                .module = v_vk_shader_module,
                .pName = info.vertex_shader_info.entry_point.c_str(),
                .pSpecializationInfo = nullptr,
            },
            VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = p_vk_shader_module,
                .pName = info.fragment_shader_info.entry_point.c_str(),
                .pSpecializationInfo = nullptr,
            },
        };

        constexpr VkPipelineVertexInputStateCreateInfo vk_vertex_input_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr,
        };
        constexpr VkPipelineInputAssemblyStateCreateInfo vk_input_assembly_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        };
        constexpr VkPipelineMultisampleStateCreateInfo vk_multisample_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
            .minSampleShading = 1.0f,
        };

        VkPipelineRasterizationStateCreateInfo vk_raster_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .polygonMode = *reinterpret_cast<VkPolygonMode const *>(&info.raster.polygon_mode),
            .cullMode = *reinterpret_cast<VkCullModeFlags const *>(&info.raster.face_culling),
            .frontFace = VkFrontFace::VK_FRONT_FACE_CLOCKWISE,
            .lineWidth = 1.0f,
        };
        VkPipelineDepthStencilStateCreateInfo vk_depth_stencil_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .depthTestEnable = info.depth_test.enable_depth_test,
            .depthWriteEnable = info.depth_test.enable_depth_write,
            .depthCompareOp = static_cast<VkCompareOp>(info.depth_test.depth_test_compare_op),
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = info.depth_test.min_depth_bounds,
            .maxDepthBounds = info.depth_test.max_depth_bounds,
        };

        DAXA_DBG_ASSERT_TRUE_M(info.color_attachments.size() < PIPELINE_COMPILER_MAX_ATTACHMENTS, "too many color attachments, make pull request to bump max");

        std::array<VkPipelineColorBlendAttachmentState, PIPELINE_COMPILER_MAX_ATTACHMENTS> vk_pipeline_color_blend_attachment_blend_states = {};
        for (usize i = 0; i < info.color_attachments.size(); ++i)
        {
            vk_pipeline_color_blend_attachment_blend_states[i] = *reinterpret_cast<VkPipelineColorBlendAttachmentState const *>(&info.color_attachments[i].blend);
        }

        std::array<VkFormat, PIPELINE_COMPILER_MAX_ATTACHMENTS> vk_pipeline_color_attachment_formats = {};
        for (usize i = 0; i < info.color_attachments.size(); ++i)
        {
            vk_pipeline_color_attachment_formats[i] = *reinterpret_cast<VkFormat const *>(&info.color_attachments[i].format);
        }

        VkPipelineColorBlendStateCreateInfo vk_color_blend_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .logicOpEnable = VK_FALSE,
            .logicOp = {},
            .attachmentCount = static_cast<u32>(info.color_attachments.size()),
            .pAttachments = vk_pipeline_color_blend_attachment_blend_states.data(),
            .blendConstants = {1.0f, 1.0f, 1.0f, 1.0f},
        };

        constexpr VkViewport DEFAULT_VIEWPORT{.width = 1, .height = 1};
        constexpr VkRect2D DEFAULT_SCISSOR{.extent = {1, 1}};

        VkPipelineViewportStateCreateInfo vk_viewport_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .viewportCount = 1,
            .pViewports = &DEFAULT_VIEWPORT,
            .scissorCount = 1,
            .pScissors = &DEFAULT_SCISSOR,
        };

        auto dynamic_state = std::array{
            VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
            VkDynamicState::VK_DYNAMIC_STATE_SCISSOR,
        };
        VkPipelineDynamicStateCreateInfo vk_dynamic_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .dynamicStateCount = static_cast<u32>(dynamic_state.size()),
            .pDynamicStates = dynamic_state.data(),
        };

        VkPipelineRenderingCreateInfo vk_pipeline_rendering{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
            .pNext = nullptr,
            .colorAttachmentCount = static_cast<u32>(info.color_attachments.size()),
            .pColorAttachmentFormats = vk_pipeline_color_attachment_formats.data(),
            .depthAttachmentFormat = static_cast<VkFormat>(info.depth_test.depth_attachment_format),
        };

        VkGraphicsPipelineCreateInfo vk_graphics_pipeline_create_info{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &vk_pipeline_rendering,
            .flags = {},
            .stageCount = 2,
            .pStages = vk_pipeline_shader_stage_create_infos,
            .pVertexInputState = &vk_vertex_input_state,
            .pInputAssemblyState = &vk_input_assembly_state,
            .pViewportState = &vk_viewport_state,
            .pRasterizationState = &vk_raster_state,
            .pMultisampleState = &vk_multisample_state,
            .pDepthStencilState = &vk_depth_stencil_state,
            .pColorBlendState = &vk_color_blend_state,
            .pDynamicState = &vk_dynamic_state,
            .layout = impl_pipeline->vk_pipeline_layout,
            .renderPass = nullptr,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
        };

        auto pipeline_result = vkCreateGraphicsPipelines(
            impl.impl_device.as<ImplDevice>()->vk_device,
            VK_NULL_HANDLE,
            1u,
            &vk_graphics_pipeline_create_info,
            nullptr,
            &impl_pipeline->vk_pipeline);

        DAXA_DBG_ASSERT_TRUE_M(pipeline_result == VK_SUCCESS, "failed to create graphics pipeline");

        vkDestroyShaderModule(impl.impl_device.as<ImplDevice>()->vk_device, v_vk_shader_module, nullptr);
        vkDestroyShaderModule(impl.impl_device.as<ImplDevice>()->vk_device, p_vk_shader_module, nullptr);

        if (impl.impl_device.as<ImplDevice>()->impl_ctx.as<ImplContext>()->enable_debug_names && info.debug_name.size() > 0)
        {
            VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_PIPELINE,
                .objectHandle = reinterpret_cast<uint64_t>(impl_pipeline->vk_pipeline),
                .pObjectName = info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(impl.impl_device.as<ImplDevice>()->vk_device, &name_info);
        }
        return RasterPipeline{ManagedPtr{impl_pipeline}};
    }

    auto PipelineCompiler::create_compute_pipeline(ComputePipelineInfo const & info) -> Result<ComputePipeline>
    {
        auto & impl = *as<ImplPipelineCompiler>();

        if (info.push_constant_size > MAX_PUSH_CONSTANT_BYTE_SIZE)
        {
            return ResultErr{std::string("push constant size of ") + std::to_string(info.push_constant_size) + std::string(" exceeds the maximum size of ") + std::to_string(MAX_PUSH_CONSTANT_BYTE_SIZE)};
        }
        if (info.push_constant_size % 4 != 0)
        {
            return ResultErr{std::string("push constant size of ") + std::to_string(info.push_constant_size) + std::string(" is not a multiple of 4(bytes)")};
        }

        auto impl_pipeline = new ImplComputePipeline(impl.impl_device, info);
        impl.current_observed_hotload_files = &impl_pipeline->observed_hotload_files;

        auto spirv_result = impl.get_spirv(info.shader_info, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT);
        if (spirv_result.is_err())
        {
            return ResultErr{.message = spirv_result.message()};
        }
        std::vector<u32> spirv = spirv_result.value();

        VkShaderModule vk_shader_module = {};

        VkShaderModuleCreateInfo shader_module_ci{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .codeSize = static_cast<u32>(spirv.size() * sizeof(u32)),
            .pCode = spirv.data(),
        };
        vkCreateShaderModule(impl.impl_device.as<ImplDevice>()->vk_device, &shader_module_ci, nullptr, &vk_shader_module);

        impl_pipeline->vk_pipeline_layout = impl.impl_device.as<ImplDevice>()->gpu_table.pipeline_layouts[(info.push_constant_size + 3) / 4];

        VkComputePipelineCreateInfo vk_compute_pipeline_create_info{
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .stage = VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
                .module = vk_shader_module,
                .pName = info.shader_info.entry_point.c_str(),
                .pSpecializationInfo = nullptr,
            },
            .layout = impl_pipeline->vk_pipeline_layout,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
        };

        auto pipeline_result = vkCreateComputePipelines(
            impl.impl_device.as<ImplDevice>()->vk_device,
            VK_NULL_HANDLE,
            1u,
            &vk_compute_pipeline_create_info,
            nullptr,
            &impl_pipeline->vk_pipeline);

        DAXA_DBG_ASSERT_TRUE_M(pipeline_result == VK_SUCCESS, "failed to create compute pipeline");

        vkDestroyShaderModule(impl.impl_device.as<ImplDevice>()->vk_device, vk_shader_module, nullptr);

        if (impl.impl_device.as<ImplDevice>()->impl_ctx.as<ImplContext>()->enable_debug_names && info.debug_name.size() > 0)
        {
            VkDebugUtilsObjectNameInfoEXT name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_PIPELINE,
                .objectHandle = reinterpret_cast<uint64_t>(impl_pipeline->vk_pipeline),
                .pObjectName = info.debug_name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(impl.impl_device.as<ImplDevice>()->vk_device, &name_info);
        }

        return ComputePipeline{ManagedPtr{impl_pipeline}};
    }

    auto PipelineCompiler::recreate_raster_pipeline(RasterPipeline const & pipeline) -> Result<RasterPipeline>
    {
        auto & impl_pipeline = *pipeline.as<ImplRasterPipeline>();
        auto result = create_raster_pipeline(impl_pipeline.info);
        if (result.is_ok())
        {
            return {std::move(result.value())};
        }
        return ResultErr{.message = result.message()};
    }

    auto PipelineCompiler::recreate_compute_pipeline(ComputePipeline const & pipeline) -> Result<ComputePipeline>
    {
        auto & impl_pipeline = *pipeline.as<ImplComputePipeline>();
        auto result = create_compute_pipeline(impl_pipeline.info);
        if (result.is_ok())
        {
            return {std::move(result.value())};
        }
        return ResultErr{.message = result.message()};
    }

    auto PipelineCompiler::check_if_sources_changed(RasterPipeline & pipeline) -> bool
    {
        auto & impl = *as<ImplPipelineCompiler>();
        auto & pipeline_impl = *pipeline.as<ImplRasterPipeline>();
        auto now = std::chrono::file_clock::now();
        using namespace std::chrono_literals;
        if (now - pipeline_impl.last_hotload_time < 250ms)
        {
            return false;
        }
        pipeline_impl.last_hotload_time = now;
        bool reload = false;
        for (auto & [path, recordedWriteTime] : pipeline_impl.observed_hotload_files)
        {
            auto ifs = std::ifstream(path);
            if (ifs.good())
            {
                auto latestWriteTime = std::filesystem::last_write_time(path);
                if (latestWriteTime > recordedWriteTime)
                {
                    reload = true;
                }
            }
        }
        if (reload)
        {
            for (auto & pair : pipeline_impl.observed_hotload_files)
            {
                auto ifs = std::ifstream(pair.first);
                if (ifs.good())
                {
                    pair.second = std::filesystem::last_write_time(pair.first);
                }
            }
        }
        return reload;
    }

    auto PipelineCompiler::check_if_sources_changed(ComputePipeline & pipeline) -> bool
    {
        auto & impl = *as<ImplPipelineCompiler>();
        auto & pipeline_impl = *pipeline.as<ImplComputePipeline>();
        auto now = std::chrono::file_clock::now();
        using namespace std::chrono_literals;
        if (now - pipeline_impl.last_hotload_time < 250ms)
        {
            return false;
        }
        pipeline_impl.last_hotload_time = now;
        bool reload = false;
        for (auto & [path, recordedWriteTime] : pipeline_impl.observed_hotload_files)
        {
            auto ifs = std::ifstream(path);
            if (ifs.good())
            {
                auto latestWriteTime = std::filesystem::last_write_time(path);
                if (latestWriteTime > recordedWriteTime)
                {
                    reload = true;
                }
            }
        }
        if (reload)
        {
            for (auto & pair : pipeline_impl.observed_hotload_files)
            {
                auto ifs = std::ifstream(pair.first);
                if (ifs.good())
                {
                    pair.second = std::filesystem::last_write_time(pair.first);
                }
            }
        }
        return reload;
    }

    ImplPipelineCompiler::ImplPipelineCompiler(ManagedWeakPtr a_impl_device, PipelineCompilerInfo const & info)
        : impl_device{std::move(a_impl_device)}, info{info}
    {
        auto includer = ShadercFileIncluder{};
        includer.impl_pipeline_compiler = this;
        shaderc_backend.options.SetIncluder(std::make_unique<ShadercFileIncluder>(includer));

        DAXA_DBG_ASSERT_TRUE_M(info.opt_level < 3, "The optimization level must be between 0 and 2 (inclusive)");
    }

    ImplPipelineCompiler::~ImplPipelineCompiler() {}

    auto ImplPipelineCompiler::get_spirv(ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage) -> Result<std::vector<u32>>
    {
        std::vector<u32> spirv = {};
        if (shader_info.source.index() == 2)
        {
            ShaderSPIRV const & input_spirv = std::get<ShaderSPIRV>(shader_info.source);
            spirv.resize(input_spirv.size);
            for (usize i = 0; i < input_spirv.size; ++i)
            {
                spirv[i] = input_spirv.data[i];
            }
        }
        else
        {
            ShaderCode code = {};
            if (auto shader_source = std::get_if<ShaderFile>(&shader_info.source))
            {
                auto ret = full_path_to_file(shader_source->path);
                if (ret.is_err())
                {
                    return ResultErr{ret.message()};
                }
                auto code_ret = load_shader_source_from_file(ret.value());
                if (code_ret.is_err())
                {
                    return ResultErr{code_ret.message()};
                }
                code = code_ret.value();
            }
            else
            {
                code = std::get<ShaderCode>(shader_info.source);
            }

            auto ret = gen_spirv_from_shaderc(shader_info, shader_stage, code);
            if (ret.is_err())
            {
                return ResultErr{ret.message()};
            }
            spirv = ret.value();
        }
        return spirv;
    }

    auto ImplPipelineCompiler::full_path_to_file(std::filesystem::path const & file) -> Result<std::filesystem::path>
    {
        if (std::filesystem::exists(file))
        {
            return {file};
        }
        std::filesystem::path potential_path;
        for (auto & root : this->info.root_paths)
        {
            potential_path.clear();
            potential_path = root / file;
            if (std::filesystem::exists(potential_path))
            {
                return {potential_path};
            }
        }
        std::string error_msg = {};
        error_msg += "could not find file :\"";
        error_msg += file.string();
        error_msg += "\"";
        return ResultErr{.message = std::move(error_msg)};
    }

    auto ImplPipelineCompiler::load_shader_source_from_file(std::filesystem::path const & path) -> Result<ShaderCode>
    {
        auto result_path = full_path_to_file(path);
        if (result_path.is_err())
        {
            return ResultErr{.message = result_path.message()};
        }
        auto start_time = std::chrono::steady_clock::now();
        while ((std::chrono::steady_clock::now() - start_time).count() < 100'000'000)
        {
            std::ifstream ifs{path};
            DAXA_DBG_ASSERT_TRUE_M(ifs.good(), "Could not open shader file");
            current_observed_hotload_files->insert({
                result_path.value(),
                std::filesystem::last_write_time(result_path.value()),
            });
            std::string str = {};
            ifs.seekg(0, std::ios::end);
            str.reserve(ifs.tellg());
            ifs.seekg(0, std::ios::beg);
            str.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
            if (str.size() < 1)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            shader_preprocess(str, path);
            return ShaderCode{.string = str};
        }
        std::string err = "timeout while trying to read file: \"";
        err += result_path.value().string() + "\"";
        return ResultErr{.message = err};
    }

    auto ImplPipelineCompiler::gen_spirv_from_shaderc(ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage, ShaderCode const & code) -> Result<std::vector<u32>>
    {
        auto translate_shader_stage = [](VkShaderStageFlagBits stage) -> shaderc_shader_kind
        {
            switch (stage)
            {
            case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT: return shaderc_shader_kind::shaderc_vertex_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return shaderc_shader_kind::shaderc_tess_control_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return shaderc_shader_kind::shaderc_tess_evaluation_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT: return shaderc_shader_kind::shaderc_geometry_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT: return shaderc_shader_kind::shaderc_fragment_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT: return shaderc_shader_kind::shaderc_compute_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR: return shaderc_shader_kind::shaderc_raygen_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return shaderc_shader_kind::shaderc_anyhit_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return shaderc_shader_kind::shaderc_closesthit_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR: return shaderc_shader_kind::shaderc_miss_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_INTERSECTION_BIT_KHR: return shaderc_shader_kind::shaderc_intersection_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR: return shaderc_shader_kind::shaderc_callable_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_NV: return shaderc_shader_kind::shaderc_task_shader;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_NV: return shaderc_shader_kind::shaderc_mesh_shader;
            default:
                std::cerr << "error: unknown shader stage!\n";
                std::abort();
            }
        };

        auto spirv_stage = translate_shader_stage(shader_stage);
        shaderc_backend.options.SetSourceLanguage(shaderc_source_language_glsl);
        shaderc_backend.options.SetTargetEnvironment(shaderc_target_env_vulkan, VK_API_VERSION_1_2);
        shaderc_backend.options.SetTargetSpirv(shaderc_spirv_version_1_3);
        shaderc_backend.options.SetOptimizationLevel(static_cast<shaderc_optimization_level>(this->info.opt_level));

        for (auto const & shader_define : shader_info.defines)
        {
            if (shader_define.value.size() > 0)
            {
                shaderc_backend.options.AddMacroDefinition(shader_define.name, shader_define.value);
            }
            else
            {
                shaderc_backend.options.AddMacroDefinition(shader_define.name);
            }
        }

        std::string debug_name = "unnamed shader";
        if (ShaderFile const * shader_file = std::get_if<ShaderFile>(&shader_info.source))
            debug_name = shader_file->path.string();
        else if (shader_info.debug_name.size() > 0)
            debug_name = shader_info.debug_name;

        shaderc::SpvCompilationResult spv_module = shaderc_backend.compiler.CompileGlslToSpv(
            code.string.c_str(), spirv_stage, debug_name.c_str(), shaderc_backend.options);

        if (spv_module.GetCompilationStatus() != shaderc_compilation_status_success)
            return daxa::ResultErr{.message = std::string("SHADERC: ") + spv_module.GetErrorMessage()};

        return {std::vector<u32>{spv_module.begin(), spv_module.end()}};
    }

    ImplRasterPipeline::ImplRasterPipeline(ManagedWeakPtr a_impl_device, RasterPipelineInfo const & info)
        : impl_device{std::move(a_impl_device)}, info{info}
    {
    }

    ImplRasterPipeline::~ImplRasterPipeline()
    {
        vkDestroyPipeline(this->impl_device.as<ImplDevice>()->vk_device, this->vk_pipeline, nullptr);
    }

    auto ImplRasterPipeline::managed_cleanup() -> bool
    {
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock lock{this->impl_device.as<ImplDevice>()->main_queue_zombies_mtx});
        u64 main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(this->impl_device.as<ImplDevice>()->main_queue_cpu_timeline);
        this->impl_device.as<ImplDevice>()->main_queue_raster_pipeline_zombies.push_front({main_queue_cpu_timeline_value, std::unique_ptr<ImplRasterPipeline>{this}});
        return false;
    }

    ImplComputePipeline::ImplComputePipeline(ManagedWeakPtr a_impl_device, ComputePipelineInfo const & info)
        : impl_device{std::move(a_impl_device)}, info{info}
    {
    }

    ImplComputePipeline::~ImplComputePipeline()
    {
        vkDestroyPipeline(this->impl_device.as<ImplDevice>()->vk_device, this->vk_pipeline, nullptr);
    }

    auto ImplComputePipeline::managed_cleanup() -> bool
    {
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock lock{this->impl_device.as<ImplDevice>()->main_queue_zombies_mtx});
        u64 main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(this->impl_device.as<ImplDevice>()->main_queue_cpu_timeline);
        this->impl_device.as<ImplDevice>()->main_queue_compute_pipeline_zombies.push_front({main_queue_cpu_timeline_value, std::unique_ptr<ImplComputePipeline>{this}});
        return false;
    }
} // namespace daxa
