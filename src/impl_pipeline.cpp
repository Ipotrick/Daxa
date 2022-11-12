#include "impl_pipeline.hpp"
#include "impl_swapchain.hpp"
#include "impl_device.hpp"

#if DAXA_BUILT_WITH_GLSLANG
static constexpr TBuiltInResource DAXA_DEFAULT_BUILTIN_RESOURCE = {
    .maxLights = 32,
    .maxClipPlanes = 6,
    .maxTextureUnits = 32,
    .maxTextureCoords = 32,
    .maxVertexAttribs = 64,
    .maxVertexUniformComponents = 4096,
    .maxVaryingFloats = 64,
    .maxVertexTextureImageUnits = 1 << 16,
    .maxCombinedTextureImageUnits = 1 << 16,
    .maxTextureImageUnits = 1 << 16,
    .maxFragmentUniformComponents = 4096,
    .maxDrawBuffers = 32,
    .maxVertexUniformVectors = 128,
    .maxVaryingVectors = 8,
    .maxFragmentUniformVectors = 16,
    .maxVertexOutputVectors = 16,
    .maxFragmentInputVectors = 15,
    .minProgramTexelOffset = -8,
    .maxProgramTexelOffset = 7,
    .maxClipDistances = 8,
    .maxComputeWorkGroupCountX = 65535,
    .maxComputeWorkGroupCountY = 65535,
    .maxComputeWorkGroupCountZ = 65535,
    .maxComputeWorkGroupSizeX = 1024,
    .maxComputeWorkGroupSizeY = 1024,
    .maxComputeWorkGroupSizeZ = 64,
    .maxComputeUniformComponents = 1024,
    .maxComputeTextureImageUnits = 1 << 16,
    .maxComputeImageUniforms = 1 << 16,
    .maxComputeAtomicCounters = 8,
    .maxComputeAtomicCounterBuffers = 1,
    .maxVaryingComponents = 60,
    .maxVertexOutputComponents = 64,
    .maxGeometryInputComponents = 64,
    .maxGeometryOutputComponents = 128,
    .maxFragmentInputComponents = 128,
    .maxImageUnits = 1 << 16,
    .maxCombinedImageUnitsAndFragmentOutputs = 8,
    .maxCombinedShaderOutputResources = 8,
    .maxImageSamples = 0,
    .maxVertexImageUniforms = 0,
    .maxTessControlImageUniforms = 0,
    .maxTessEvaluationImageUniforms = 0,
    .maxGeometryImageUniforms = 0,
    .maxFragmentImageUniforms = 8,
    .maxCombinedImageUniforms = 8,
    .maxGeometryTextureImageUnits = 16,
    .maxGeometryOutputVertices = 256,
    .maxGeometryTotalOutputComponents = 1024,
    .maxGeometryUniformComponents = 1024,
    .maxGeometryVaryingComponents = 64,
    .maxTessControlInputComponents = 128,
    .maxTessControlOutputComponents = 128,
    .maxTessControlTextureImageUnits = 16,
    .maxTessControlUniformComponents = 1024,
    .maxTessControlTotalOutputComponents = 4096,
    .maxTessEvaluationInputComponents = 128,
    .maxTessEvaluationOutputComponents = 128,
    .maxTessEvaluationTextureImageUnits = 16,
    .maxTessEvaluationUniformComponents = 1024,
    .maxTessPatchComponents = 120,
    .maxPatchVertices = 32,
    .maxTessGenLevel = 64,
    .maxViewports = 16,
    .maxVertexAtomicCounters = 0,
    .maxTessControlAtomicCounters = 0,
    .maxTessEvaluationAtomicCounters = 0,
    .maxGeometryAtomicCounters = 0,
    .maxFragmentAtomicCounters = 8,
    .maxCombinedAtomicCounters = 8,
    .maxAtomicCounterBindings = 1,
    .maxVertexAtomicCounterBuffers = 0,
    .maxTessControlAtomicCounterBuffers = 0,
    .maxTessEvaluationAtomicCounterBuffers = 0,
    .maxGeometryAtomicCounterBuffers = 0,
    .maxFragmentAtomicCounterBuffers = 1,
    .maxCombinedAtomicCounterBuffers = 1,
    .maxAtomicCounterBufferSize = 16384,
    .maxTransformFeedbackBuffers = 4,
    .maxTransformFeedbackInterleavedComponents = 64,
    .maxCullDistances = 8,
    .maxCombinedClipAndCullDistances = 8,
    .maxSamples = 4,
    .maxMeshOutputVerticesNV = 256,
    .maxMeshOutputPrimitivesNV = 512,
    .maxMeshWorkGroupSizeX_NV = 32,
    .maxMeshWorkGroupSizeY_NV = 1,
    .maxMeshWorkGroupSizeZ_NV = 1,
    .maxTaskWorkGroupSizeX_NV = 32,
    .maxTaskWorkGroupSizeY_NV = 1,
    .maxTaskWorkGroupSizeZ_NV = 1,
    .maxMeshViewCountNV = 4,
    .maxDualSourceDrawBuffersEXT = {},
    .limits{
        .nonInductiveForLoops = true,
        .whileLoops = true,
        .doWhileLoops = true,
        .generalUniformIndexing = true,
        .generalAttributeMatrixVectorIndexing = true,
        .generalVaryingIndexing = true,
        .generalSamplerIndexing = true,
        .generalVariableIndexing = true,
        .generalConstantMatrixVectorIndexing = true,
    },
};
#endif

#include <regex>
#include <thread>
#include <utility>

static const std::regex PRAGMA_ONCE_REGEX = std::regex(R"reg(#\s*pragma\s*once\s*)reg");
static const std::regex REPLACE_REGEX = std::regex(R"reg(\W)reg");
static void shader_preprocess(std::string & file_str, std::filesystem::path const & path)
{
    std::smatch matches = {};
    std::string line = {};
    std::stringstream file_ss{file_str};
    std::stringstream result_ss = {};
    bool has_pragma_once = false;
    auto abs_path_str = std::filesystem::absolute(path).string();
    while (std::getline(file_ss, line))
    {
        if (std::regex_match(line, matches, PRAGMA_ONCE_REGEX))
        {
            result_ss << "#if !defined(";
            std::regex_replace(std::ostreambuf_iterator<char>(result_ss), abs_path_str.begin(), abs_path_str.end(), REPLACE_REGEX, "");
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
        std::regex_replace(std::ostreambuf_iterator<char>(result_ss), abs_path_str.begin(), abs_path_str.end(), REPLACE_REGEX, "");
        result_ss << "\n#endif\n";
    }
    file_str = result_ss.str();
}

namespace daxa
{
#if DAXA_BUILT_WITH_GLSLANG
    class GlslangFileIncluder : public glslang::TShader::Includer
    {
      public:
        constexpr static inline size_t DELETE_SOURCE_NAME = 0x1;
        constexpr static inline size_t DELETE_CONTENT = 0x2;

        ImplPipelineCompiler * impl_pipeline_compiler = nullptr;

        auto includeLocal(
            char const * header_name, char const * includer_name, size_t inclusion_depth) -> IncludeResult * override
        {
            return includeSystem(header_name, includer_name, inclusion_depth);
        }

        auto includeSystem(
            char const * header_name, char const * /*includer_name*/, size_t /*inclusion_depth*/) -> IncludeResult * override
        {
            std::string headerName = {};
            char const * headerData = nullptr;
            size_t headerLength = 0;

            auto result = impl_pipeline_compiler->full_path_to_file(header_name);
            if (result.is_err())
            {
                return nullptr;
            }

            auto full_path = result.value();
            auto search_pred = [&](std::filesystem::path const & p)
            { return p == full_path; };

            if (std::find_if(
                    impl_pipeline_compiler->current_seen_shader_files.begin(),
                    impl_pipeline_compiler->current_seen_shader_files.end(),
                    search_pred) != impl_pipeline_compiler->current_seen_shader_files.end())
            {
                return nullptr;
            }

            impl_pipeline_compiler->current_observed_hotload_files->insert({full_path, std::chrono::file_clock::now()});
            auto shader_code_result = impl_pipeline_compiler->load_shader_source_from_file(full_path);

            if (shader_code_result.is_err())
            {
                return nullptr;
            }

            auto & shader_code_str = shader_code_result.value().string;
            headerLength = shader_code_str.size();
            char * res_content = new char[headerLength + 1];
            for (usize i = 0; i < headerLength; ++i)
            {
                res_content[i] = shader_code_str[i];
            }
            res_content[headerLength] = '\0';
            headerData = res_content;

            headerName = full_path.string();
            for (auto & c : headerName)
            {
                if (c == '\\')
                {
                    c = '/';
                }
            }

            return new IncludeResult{headerName, headerData, headerLength, nullptr};
        }

        void releaseInclude(IncludeResult * result) override
        {
            if (result != nullptr)
            {
                {
                    delete result->headerData;
                }
                delete result;
            }
        }
    };
#endif

#if DAXA_BUILT_WITH_DXC
    struct DxcCustomIncluder : public IDxcIncludeHandler
    {
        IDxcIncludeHandler * default_includer{};
        ImplPipelineCompiler * impl_pipeline_compiler{};

        virtual ~DxcCustomIncluder() = default;
        auto LoadSource(LPCWSTR filename, IDxcBlob ** include_source) -> HRESULT override
        {
            if (filename[0] == '.')
            {
                filename += 2;
            }

            auto result = impl_pipeline_compiler->full_path_to_file(filename);
            if (result.is_err())
            {
                *include_source = nullptr;
                return SCARD_E_FILE_NOT_FOUND;
            }
            auto full_path = result.value();
            auto search_pred = [&](std::filesystem::path const & p)
            { return p == full_path; };

            ComPtr<IDxcBlobEncoding> dxc_blob_encoding = {};
            if (std::find_if(impl_pipeline_compiler->current_seen_shader_files.begin(),
                             impl_pipeline_compiler->current_seen_shader_files.end(), search_pred) != impl_pipeline_compiler->current_seen_shader_files.end())
            {
                // Return empty string blob if this file has been included before
                static char const * const null_str = " ";
                impl_pipeline_compiler->dxc_backend.dxc_utils->CreateBlob(null_str, static_cast<u32>(strlen(null_str)), CP_UTF8, &dxc_blob_encoding);
                *include_source = dxc_blob_encoding.Detach();
                return S_OK;
            }
            else
            {
                impl_pipeline_compiler->current_observed_hotload_files->insert({full_path, std::chrono::file_clock::now()});
            }

            auto str_result = impl_pipeline_compiler->load_shader_source_from_file(full_path);
            if (str_result.is_err())
            {
                *include_source = nullptr;
                return SCARD_E_INVALID_PARAMETER;
            }
            std::string const str = str_result.value().string;

            impl_pipeline_compiler->dxc_backend.dxc_utils->CreateBlob(str.c_str(), static_cast<u32>(str.size()), CP_UTF8, &dxc_blob_encoding);
            *include_source = dxc_blob_encoding.Detach();
            return S_OK;
        }

        auto QueryInterface(REFIID riid, void ** object) -> HRESULT override
        {
            return default_includer->QueryInterface(riid, object);
        }

        unsigned long STDMETHODCALLTYPE AddRef() override { return 0; }
        unsigned long STDMETHODCALLTYPE Release() override { return 0; }
    };
#endif
} // namespace daxa

namespace daxa
{
    void ShaderCompileOptions::inherit(ShaderCompileOptions const & other)
    {
        if (!this->entry_point.has_value())
        {
            this->entry_point = other.entry_point;
        }
        if (!this->opt_level.has_value())
        {
            this->opt_level = other.opt_level;
        }
        if (!this->shader_model.has_value())
        {
            this->shader_model = other.shader_model;
        }
        if (!this->language.has_value())
        {
            this->language = other.language;
        }

        this->root_paths.insert(this->root_paths.begin(), other.root_paths.begin(), other.root_paths.end());
        this->defines.insert(this->defines.end(), other.defines.begin(), other.defines.end());
    }

    RasterPipeline::RasterPipeline(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto RasterPipeline::info() const -> RasterPipelineInfo const &
    {
        auto const & impl = *as<ImplRasterPipeline>();
        return impl.info;
    }

    ComputePipeline::ComputePipeline(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto ComputePipeline::info() const -> ComputePipelineInfo const &
    {
        auto const & impl = *as<ImplComputePipeline>();
        return impl.info;
    }

    PipelineCompiler::PipelineCompiler(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    auto PipelineCompiler::create_raster_pipeline(RasterPipelineInfo const & info) -> Result<RasterPipeline>
    {
        auto & impl = *as<ImplPipelineCompiler>();
        auto modified_info = info;
        modified_info.vertex_shader_info.compile_options.inherit(impl.info.shader_compile_options);
        modified_info.fragment_shader_info.compile_options.inherit(impl.info.shader_compile_options);

        if (modified_info.push_constant_size > MAX_PUSH_CONSTANT_BYTE_SIZE)
        {
            return Result<RasterPipeline>(std::string("push constant size of ") + std::to_string(modified_info.push_constant_size) + std::string(" exceeds the maximum size of ") + std::to_string(MAX_PUSH_CONSTANT_BYTE_SIZE));
        }
        if (modified_info.push_constant_size % 4 != 0)
        {
            return Result<RasterPipeline>(std::string("push constant size of ") + std::to_string(modified_info.push_constant_size) + std::string(" is not a multiple of 4(bytes)"));
        }

        auto * impl_pipeline = new ImplRasterPipeline(impl.impl_device, modified_info);
        impl.current_observed_hotload_files = &impl_pipeline->observed_hotload_files;
        impl_pipeline->last_hotload_time = std::chrono::file_clock::now();

        // TODO(grundlett): Maybe add ability to output the spirv binary for people!
        // {
        //     u32 i = 0;
        //     std::string vs_name = modified_info.vertex_shader_info.debug_name + ".vert.txt";
        //     std::ofstream vs_file{vs_name};
        //     for (auto s : v_spirv)
        //     {
        //         vs_file << "0x" << std::setfill('0') << std::setw(8) << std::hex << s << ", ";
        //         if (++i == 8)
        //         {
        //             i = 0;
        //             vs_file << "\n";
        //         }
        //     }
        // }
        // {
        //     u32 i = 0;
        //     std::string fs_name = modified_info.fragment_shader_info.debug_name + ".frag.txt";
        //     std::ofstream fs_file{fs_name};
        //     for (auto s : p_spirv)
        //     {
        //         fs_file << "0x" << std::setfill('0') << std::setw(8) << std::hex << s << ", ";
        //         if (++i == 8)
        //         {
        //             i = 0;
        //             fs_file << "\n";
        //         }
        //     }
        // }

        std::vector<VkShaderModule> vk_shader_modules{};
        std::vector<VkPipelineShaderStageCreateInfo> vk_pipeline_shader_stage_create_infos{};

        auto create_shader_module = [&](ShaderInfo const & shader_info, VkShaderStageFlagBits shader_stage) -> Result<bool>
        {
            if (!std::holds_alternative<std::monostate>(shader_info.source))
            {
                auto spirv_result = impl.get_spirv(shader_info, shader_stage);
                if (spirv_result.is_err())
                {
                    return Result<bool>(spirv_result.message());
                }

                std::vector<u32> spirv = spirv_result.value();

                VkShaderModule vk_shader_module = nullptr;

                VkShaderModuleCreateInfo const vk_shader_module_create_info{
                    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .codeSize = static_cast<u32>(spirv.size() * sizeof(u32)),
                    .pCode = spirv.data(),
                };

                vkCreateShaderModule(impl.impl_device.as<ImplDevice>()->vk_device, &vk_shader_module_create_info, nullptr, &vk_shader_module);
                vk_shader_modules.push_back(vk_shader_module);

                VkPipelineShaderStageCreateInfo const vk_pipeline_shader_stage_create_info{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .stage = shader_stage,
                    .module = vk_shader_module,
                    .pName = shader_info.compile_options.entry_point.value().c_str(),
                    .pSpecializationInfo = nullptr,
                };

                vk_pipeline_shader_stage_create_infos.push_back(vk_pipeline_shader_stage_create_info);
            }

            return Result<bool>(true);
        };

        {
            auto result = create_shader_module(modified_info.vertex_shader_info, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
            if (result.is_err())
            {
                return Result<RasterPipeline>(result.message());
            }
        }

        {
            auto result = create_shader_module(modified_info.fragment_shader_info, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
            if (result.is_err())
            {
                return Result<RasterPipeline>(result.message());
            }
        }

        impl_pipeline->vk_pipeline_layout = impl.impl_device.as<ImplDevice>()->gpu_shader_resource_table.pipeline_layouts.at((modified_info.push_constant_size + 3) / 4);

        constexpr VkPipelineVertexInputStateCreateInfo vk_vertex_input_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr,
        };
        constexpr VkPipelineInputAssemblyStateCreateInfo vk_input_assembly_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = {},
        };
        constexpr VkPipelineMultisampleStateCreateInfo vk_multisample_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = {},
            .alphaToCoverageEnable = {},
            .alphaToOneEnable = {},
        };

        VkPipelineRasterizationStateCreateInfo const vk_raster_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .depthClampEnable = {},
            .rasterizerDiscardEnable = {},
            .polygonMode = *reinterpret_cast<VkPolygonMode const *>(&info.raster.polygon_mode),
            .cullMode = *reinterpret_cast<VkCullModeFlags const *>(&info.raster.face_culling),
            .frontFace = VkFrontFace::VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = static_cast<VkBool32>(info.raster.depth_bias_enable),
            .depthBiasConstantFactor = info.raster.depth_bias_constant_factor,
            .depthBiasClamp = info.raster.depth_bias_clamp,
            .depthBiasSlopeFactor = info.raster.depth_bias_slope_factor,
            .lineWidth = info.raster.line_width,
        };
        VkPipelineDepthStencilStateCreateInfo const vk_depth_stencil_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .depthTestEnable = static_cast<VkBool32>(modified_info.depth_test.enable_depth_test),
            .depthWriteEnable = static_cast<VkBool32>(modified_info.depth_test.enable_depth_write),
            .depthCompareOp = static_cast<VkCompareOp>(modified_info.depth_test.depth_test_compare_op),
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = modified_info.depth_test.min_depth_bounds,
            .maxDepthBounds = modified_info.depth_test.max_depth_bounds,
        };

        DAXA_DBG_ASSERT_TRUE_M(modified_info.color_attachments.size() < PIPELINE_COMPILER_MAX_ATTACHMENTS, "too many color attachments, make pull request to bump max");

        std::array<VkPipelineColorBlendAttachmentState, PIPELINE_COMPILER_MAX_ATTACHMENTS> vk_pipeline_color_blend_attachment_blend_states = {};
        for (usize i = 0; i < modified_info.color_attachments.size(); ++i)
        {
            vk_pipeline_color_blend_attachment_blend_states.at(i) = *reinterpret_cast<VkPipelineColorBlendAttachmentState const *>(&modified_info.color_attachments.at(i).blend);
        }

        std::array<VkFormat, PIPELINE_COMPILER_MAX_ATTACHMENTS> vk_pipeline_color_attachment_formats = {};
        for (usize i = 0; i < modified_info.color_attachments.size(); ++i)
        {
            vk_pipeline_color_attachment_formats.at(i) = *reinterpret_cast<VkFormat const *>(&modified_info.color_attachments.at(i).format);
        }

        VkPipelineColorBlendStateCreateInfo const vk_color_blend_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .logicOpEnable = VK_FALSE,
            .logicOp = {},
            .attachmentCount = static_cast<u32>(modified_info.color_attachments.size()),
            .pAttachments = vk_pipeline_color_blend_attachment_blend_states.data(),
            .blendConstants = {1.0f, 1.0f, 1.0f, 1.0f},
        };

        constexpr VkViewport DEFAULT_VIEWPORT{.x = 0, .y = 0, .width = 1, .height = 1, .minDepth = 0, .maxDepth = 0};
        constexpr VkRect2D DEFAULT_SCISSOR{.offset = {0, 0}, .extent = {1, 1}};

        VkPipelineViewportStateCreateInfo const vk_viewport_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .viewportCount = 1,
            .pViewports = &DEFAULT_VIEWPORT,
            .scissorCount = 1,
            .pScissors = &DEFAULT_SCISSOR,
        };

        auto dynamic_state = std::array{
            VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
            VkDynamicState::VK_DYNAMIC_STATE_SCISSOR,
        };
        VkPipelineDynamicStateCreateInfo const vk_dynamic_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .dynamicStateCount = static_cast<u32>(dynamic_state.size()),
            .pDynamicStates = dynamic_state.data(),
        };

        VkPipelineRenderingCreateInfo vk_pipeline_rendering{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
            .pNext = nullptr,
            .viewMask = {},
            .colorAttachmentCount = static_cast<u32>(modified_info.color_attachments.size()),
            .pColorAttachmentFormats = vk_pipeline_color_attachment_formats.data(),
            .depthAttachmentFormat = static_cast<VkFormat>(modified_info.depth_test.depth_attachment_format),
            .stencilAttachmentFormat = {},
        };

        VkGraphicsPipelineCreateInfo const vk_graphics_pipeline_create_info{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &vk_pipeline_rendering,
            .flags = {},
            .stageCount = static_cast<u32>(vk_pipeline_shader_stage_create_infos.size()),
            .pStages = vk_pipeline_shader_stage_create_infos.data(),
            .pVertexInputState = &vk_vertex_input_state,
            .pInputAssemblyState = &vk_input_assembly_state,
            .pTessellationState = {},
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

        [[maybe_unused]] auto pipeline_result = vkCreateGraphicsPipelines(
            impl.impl_device.as<ImplDevice>()->vk_device,
            VK_NULL_HANDLE,
            1u,
            &vk_graphics_pipeline_create_info,
            nullptr,
            &impl_pipeline->vk_pipeline);

        DAXA_DBG_ASSERT_TRUE_M(pipeline_result == VK_SUCCESS, "failed to create graphics pipeline");

        for (auto & vk_shader_module : vk_shader_modules)
        {
            vkDestroyShaderModule(impl.impl_device.as<ImplDevice>()->vk_device, vk_shader_module, nullptr);
        }

        if (impl.impl_device.as<ImplDevice>()->impl_ctx.as<ImplContext>()->enable_debug_names && !modified_info.debug_name.empty())
        {
            auto raster_pipeline_name = modified_info.debug_name + std::string(" [Daxa RasterPipeline]");
            VkDebugUtilsObjectNameInfoEXT const name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_PIPELINE,
                .objectHandle = reinterpret_cast<uint64_t>(impl_pipeline->vk_pipeline),
                .pObjectName = raster_pipeline_name.c_str(),
            };
            impl.impl_device.as<ImplDevice>()->vkSetDebugUtilsObjectNameEXT(impl.impl_device.as<ImplDevice>()->vk_device, &name_info);
        }
        return Result<RasterPipeline>(RasterPipeline(ManagedPtr(impl_pipeline)));
    }

    auto PipelineCompiler::create_compute_pipeline(ComputePipelineInfo const & info) -> Result<ComputePipeline>
    {
        auto & impl = *as<ImplPipelineCompiler>();
        auto modified_info = info;
        modified_info.shader_info.compile_options.inherit(impl.info.shader_compile_options);

        if (modified_info.push_constant_size > MAX_PUSH_CONSTANT_BYTE_SIZE)
        {
            return Result<ComputePipeline>(std::string("push constant size of ") + std::to_string(modified_info.push_constant_size) + std::string(" exceeds the maximum size of ") + std::to_string(MAX_PUSH_CONSTANT_BYTE_SIZE));
        }
        if (modified_info.push_constant_size % 4 != 0)
        {
            return Result<ComputePipeline>(std::string("push constant size of ") + std::to_string(modified_info.push_constant_size) + std::string(" is not a multiple of 4(bytes)"));
        }

        auto * impl_pipeline = new ImplComputePipeline(impl.impl_device, modified_info);
        impl.current_observed_hotload_files = &impl_pipeline->observed_hotload_files;
        impl_pipeline->last_hotload_time = std::chrono::file_clock::now();

        auto spirv_result = impl.get_spirv(modified_info.shader_info, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT);
        if (spirv_result.is_err())
        {
            return Result<ComputePipeline>(spirv_result.message());
        }
        std::vector<u32> spirv = spirv_result.value();

        VkShaderModule vk_shader_module = {};

        VkShaderModuleCreateInfo const shader_module_ci{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .codeSize = static_cast<u32>(spirv.size() * sizeof(u32)),
            .pCode = spirv.data(),
        };
        vkCreateShaderModule(impl.impl_device.as<ImplDevice>()->vk_device, &shader_module_ci, nullptr, &vk_shader_module);

        impl_pipeline->vk_pipeline_layout = impl.impl_device.as<ImplDevice>()->gpu_shader_resource_table.pipeline_layouts.at((modified_info.push_constant_size + 3) / 4);

        VkComputePipelineCreateInfo const vk_compute_pipeline_create_info{
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .stage = VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
                .module = vk_shader_module,
                .pName = modified_info.shader_info.compile_options.entry_point.value().c_str(),
                .pSpecializationInfo = nullptr,
            },
            .layout = impl_pipeline->vk_pipeline_layout,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
        };

        [[maybe_unused]] auto pipeline_result = vkCreateComputePipelines(
            impl.impl_device.as<ImplDevice>()->vk_device,
            VK_NULL_HANDLE,
            1u,
            &vk_compute_pipeline_create_info,
            nullptr,
            &impl_pipeline->vk_pipeline);

        DAXA_DBG_ASSERT_TRUE_M(pipeline_result == VK_SUCCESS, "failed to create compute pipeline");

        vkDestroyShaderModule(impl.impl_device.as<ImplDevice>()->vk_device, vk_shader_module, nullptr);

        if (impl.impl_device.as<ImplDevice>()->impl_ctx.as<ImplContext>()->enable_debug_names && !info.debug_name.empty())
        {
            auto raster_pipeline_name = modified_info.debug_name + std::string(" [Daxa ComputePipeline]");
            VkDebugUtilsObjectNameInfoEXT const name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_PIPELINE,
                .objectHandle = reinterpret_cast<uint64_t>(impl_pipeline->vk_pipeline),
                .pObjectName = raster_pipeline_name.c_str(),
            };
            impl.impl_device.as<ImplDevice>()->vkSetDebugUtilsObjectNameEXT(impl.impl_device.as<ImplDevice>()->vk_device, &name_info);
        }

        return Result<ComputePipeline>(ComputePipeline(ManagedPtr(impl_pipeline)));
    }

    auto PipelineCompiler::recreate_raster_pipeline(RasterPipeline const & pipeline) -> Result<RasterPipeline>
    {
        auto const & impl_pipeline = *pipeline.as<ImplRasterPipeline>();
        auto result = create_raster_pipeline(impl_pipeline.info);
        return result;
    }

    auto PipelineCompiler::recreate_compute_pipeline(ComputePipeline const & pipeline) -> Result<ComputePipeline>
    {
        auto const & impl_pipeline = *pipeline.as<ImplComputePipeline>();
        auto result = create_compute_pipeline(impl_pipeline.info);
        return result;
    }

    using namespace std::chrono_literals;
    static constexpr auto HOTRELOAD_MIN_TIME = 250ms;

    auto PipelineCompiler::check_if_sources_changed(RasterPipeline & pipeline) -> bool
    {
        auto & pipeline_impl = *pipeline.as<ImplRasterPipeline>();
        auto now = std::chrono::file_clock::now();
        if (now - pipeline_impl.last_hotload_time < HOTRELOAD_MIN_TIME)
        {
            return false;
        }
        pipeline_impl.last_hotload_time = now;
        bool reload = false;
        for (auto & [path, recorded_write_time] : pipeline_impl.observed_hotload_files)
        {
            auto ifs = std::ifstream(path);
            if (ifs.good())
            {
                auto latest_write_time = std::filesystem::last_write_time(path);
                if (latest_write_time > recorded_write_time)
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
        auto & pipeline_impl = *pipeline.as<ImplComputePipeline>();
        auto now = std::chrono::file_clock::now();
        using namespace std::chrono_literals;
        if (now - pipeline_impl.last_hotload_time < HOTRELOAD_MIN_TIME)
        {
            return false;
        }
        pipeline_impl.last_hotload_time = now;
        bool reload = false;
        for (auto & [path, recorded_write_time] : pipeline_impl.observed_hotload_files)
        {
            auto ifs = std::ifstream(path);
            if (ifs.good())
            {
                auto latest_write_time = std::filesystem::last_write_time(path);
                if (latest_write_time > recorded_write_time)
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

    ImplPipelineCompiler::ImplPipelineCompiler(ManagedWeakPtr a_device_impl, PipelineCompilerInfo a_info)
        : impl_device{std::move(a_device_impl)}, info{std::move(a_info)}
    {
        if (!this->info.shader_compile_options.entry_point.has_value())
        {
            this->info.shader_compile_options.entry_point = std::optional<std::string>{"main"};
        }
        if (!this->info.shader_compile_options.opt_level.has_value())
        {
            this->info.shader_compile_options.opt_level = std::optional<u32>{0};
        }
        if (!this->info.shader_compile_options.shader_model.has_value())
        {
            this->info.shader_compile_options.shader_model = std::optional<ShaderModel>{ShaderModel{.major = 6, .minor = 6}};
        }
        if (!this->info.shader_compile_options.language.has_value())
        {
            this->info.shader_compile_options.language = std::optional<ShaderLanguage>{ShaderLanguage::HLSL};
        }

#if DAXA_BUILT_WITH_GLSLANG
        {
            glslang::InitializeProcess();
        }
#endif

#if DAXA_BUILT_WITH_DXC
        {
            [[maybe_unused]] HRESULT dxc_utils_result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&this->dxc_backend.dxc_utils));
            DAXA_DBG_ASSERT_TRUE_M(SUCCEEDED(dxc_utils_result), "Failed to create DXC utils");
            [[maybe_unused]] HRESULT dxc_compiler_result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&this->dxc_backend.dxc_compiler));
            DAXA_DBG_ASSERT_TRUE_M(SUCCEEDED(dxc_compiler_result), "Failed to create DXC compiler");
            ComPtr<DxcCustomIncluder> dxc_includer = new DxcCustomIncluder();
            dxc_includer->impl_pipeline_compiler = this;
            this->dxc_backend.dxc_utils->CreateDefaultIncludeHandler(&(dxc_includer->default_includer));
            this->dxc_backend.dxc_includer = dxc_includer.Detach();
        }
#endif
    }

    ImplPipelineCompiler::~ImplPipelineCompiler()
    {
#if DAXA_BUILT_WITH_GLSLANG
        {
            glslang::FinalizeProcess();
        }
#endif
    }

    auto ImplPipelineCompiler::get_spirv(ShaderInfo const & shader_info, [[maybe_unused]] VkShaderStageFlagBits shader_stage) -> Result<std::vector<u32>>
    {
        current_shader_info = &shader_info;
        std::vector<u32> spirv = {};
        if (std::holds_alternative<ShaderSPIRV>(shader_info.source))
        {
            auto const & input_spirv = std::get<ShaderSPIRV>(shader_info.source);
            spirv.resize(input_spirv.size);
            for (usize i = 0; i < input_spirv.size; ++i)
            {
                spirv[i] = input_spirv.data[i];
            }
        }
        else
        {
            ShaderCode code;
            if (auto const * shader_source = std::get_if<ShaderFile>(&shader_info.source))
            {
                auto ret = full_path_to_file(shader_source->path);
                if (ret.is_err())
                {
                    return Result<std::vector<u32>>(ret.message());
                }
                auto code_ret = load_shader_source_from_file(ret.value());
                if (code_ret.is_err())
                {
                    return Result<std::vector<u32>>(code_ret.message());
                }
                code = code_ret.value();
            }
            else
            {
                code = std::get<ShaderCode>(shader_info.source);
            }

            Result<std::vector<u32>> ret = Result<std::vector<u32>>("No shader was compiled");

            switch (shader_info.compile_options.language.value())
            {
#if DAXA_BUILT_WITH_GLSLANG
            case ShaderLanguage::GLSL:
                ret = gen_spirv_from_glslang(shader_info, shader_stage, code);
                break;
#endif
#if DAXA_BUILT_WITH_DXC
            case ShaderLanguage::HLSL:
                ret = gen_spirv_from_dxc(shader_info, shader_stage, code);
                break;
#endif
            default: break;
            }

            if (ret.is_err())
            {
                current_shader_info = nullptr;
                return Result<std::vector<u32>>(ret.message());
            }
            spirv = ret.value();
        }
        current_shader_info = nullptr;
        return Result<std::vector<u32>>(spirv);
    }

    auto ImplPipelineCompiler::full_path_to_file(std::filesystem::path const & path) -> Result<std::filesystem::path>
    {
        if (std::filesystem::exists(path))
        {
            return Result<std::filesystem::path>(path);
        }
        std::filesystem::path potential_path;
        if (this->current_shader_info)
        {
            for (auto & root : this->current_shader_info->compile_options.root_paths)
            {
                potential_path.clear();
                potential_path = root / path;
                if (std::filesystem::exists(potential_path))
                {
                    return Result<std::filesystem::path>(potential_path);
                }
            }
        }
        else
        {
            for (auto & root : this->info.shader_compile_options.root_paths)
            {
                potential_path.clear();
                potential_path = root / path;
                if (std::filesystem::exists(potential_path))
                {
                    return Result<std::filesystem::path>(potential_path);
                }
            }
        }
        std::string error_msg = {};
        error_msg += "could not find file :\"";
        error_msg += path.string();
        error_msg += "\"";
        return Result<std::filesystem::path>(std::string_view(error_msg));
    }

    auto ImplPipelineCompiler::load_shader_source_from_file(std::filesystem::path const & path) -> Result<ShaderCode>
    {
        auto result_path = full_path_to_file(path);
        if (result_path.is_err())
        {
            return Result<ShaderCode>(result_path.message());
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
            str.reserve(static_cast<usize>(ifs.tellg()));
            ifs.seekg(0, std::ios::beg);
            str.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
            if (str.empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            shader_preprocess(str, path);
            return Result(ShaderCode{.string = str});
        }
        std::string err = "timeout while trying to read file: \"";
        err += result_path.value().string() + "\"";
        return Result<ShaderCode>(err);
    }

    auto ImplPipelineCompiler::gen_spirv_from_glslang([[maybe_unused]] ShaderInfo const & shader_info, [[maybe_unused]] VkShaderStageFlagBits shader_stage, [[maybe_unused]] ShaderCode const & code) -> Result<std::vector<u32>>
    {
#if DAXA_BUILT_WITH_GLSLANG
        auto translate_shader_stage = [](VkShaderStageFlagBits stage) -> EShLanguage
        {
            switch (stage)
            {
            case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT: return EShLanguage::EShLangVertex;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return EShLanguage::EShLangTessControl;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return EShLanguage::EShLangTessEvaluation;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT: return EShLanguage::EShLangGeometry;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT: return EShLanguage::EShLangFragment;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT: return EShLanguage::EShLangCompute;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR: return EShLanguage::EShLangRayGen;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return EShLanguage::EShLangAnyHit;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return EShLanguage::EShLangClosestHit;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR: return EShLanguage::EShLangMiss;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_INTERSECTION_BIT_KHR: return EShLanguage::EShLangIntersect;
            case VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR: return EShLanguage::EShLangCallable;
            // case VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_NV: return EShLanguage::EShLangTask;
            // case VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_NV: return EShLanguage::EShLangMesh;
            default:
                DAXA_DBG_ASSERT_TRUE_M(false, "Tried creating shader with unknown shader stage");
                return EShLanguage::EShLangCount;
            }
        };

        DAXA_DBG_ASSERT_TRUE_M(shader_info.compile_options.opt_level < 3, "For glslang, The optimization level must be between 0 and 2 (inclusive)");

        std::string preamble;

        auto spirv_stage = translate_shader_stage(shader_stage);

        // preamble += "#version 450\n";
        preamble += "#define DAXA_SHADER 1\n";
        preamble += "#define DAXA_SHADERLANG 1\n";
        preamble += "#extension GL_GOOGLE_include_directive : enable\n";
        preamble += "#extension GL_EXT_nonuniform_qualifier : enable\n";
        preamble += "#extension GL_EXT_buffer_reference : enable\n";
        preamble += "#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable\n";

        // TODO(grundlett): Probably expose this as a compile option!
        preamble += "#extension GL_KHR_shader_subgroup_basic : enable\n";
        preamble += "#extension GL_KHR_shader_subgroup_vote : enable\n";
        preamble += "#extension GL_KHR_shader_subgroup_arithmetic : enable\n";
        preamble += "#extension GL_KHR_shader_subgroup_ballot : enable\n";
        preamble += "#extension GL_KHR_shader_subgroup_shuffle : enable\n";
        preamble += "#extension GL_KHR_shader_subgroup_shuffle_relative : enable\n";
        preamble += "#extension GL_KHR_shader_subgroup_clustered : enable\n";
        preamble += "#extension GL_KHR_shader_subgroup_quad : enable\n";
        preamble += "#extension GL_EXT_scalar_block_layout : require\n";
        for (auto const & shader_define : shader_info.compile_options.defines)
        {
            if (!shader_define.value.empty())
            {
                preamble += std::string("#define ") + shader_define.name + " " + shader_define.value + "\n";
            }
            else
            {
                preamble += std::string("#define ") + shader_define.name + "\n";
            }
        }
        std::string debug_name = "unnamed shader";
        if (ShaderFile const * shader_file = std::get_if<ShaderFile>(&shader_info.source))
        {
            debug_name = shader_file->path.string();
        }
        else if (!shader_info.debug_name.empty())
        {
            debug_name = shader_info.debug_name;
        }

        glslang::TShader shader{spirv_stage};

        shader.setPreamble(preamble.c_str());

        auto const & source_str = code.string;

        std::vector<char const *> sources;
        sources.push_back(source_str.c_str());

        shader.setStrings(sources.data(), static_cast<i32>(sources.size()));
        shader.setEntryPoint("main");
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);
        // shader.setOverrideVersion(450); // What should this be? I think it might be set by default

        GlslangFileIncluder includer;
        includer.impl_pipeline_compiler = this;
        auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
        TBuiltInResource const resource = DAXA_DEFAULT_BUILTIN_RESOURCE;

        if (!shader.parse(&resource, 450, false, messages, includer))
        {
            return Result<std::vector<u32>>(std::string("GLSLANG: ") + shader.getInfoLog() + shader.getInfoDebugLog());
        }

        glslang::TProgram program;
        program.addShader(&shader);

        if (!program.link(messages))
        {
            return Result<std::vector<u32>>(std::string("GLSLANG: ") + shader.getInfoLog() + shader.getInfoDebugLog());
        }

        auto * intermediary = program.getIntermediate(spirv_stage);
        if (intermediary == nullptr)
        {
            return Result<std::vector<u32>>(std::string("GLSLANG: Failed to get shader stage intermediary"));
        }

        spv::SpvBuildLogger logger;
        glslang::SpvOptions spv_options{};
        // spv_options.generateDebugInfo = true;
        std::vector<u32> spv;
        glslang::GlslangToSpv(*intermediary, spv, &logger, &spv_options);
        return Result<std::vector<u32>>(spv);
#else
        return Result<std::vector<u32>>("Asked for glslang compilation without enabling glslang");
#endif
    }

    auto ImplPipelineCompiler::gen_spirv_from_dxc([[maybe_unused]] ShaderInfo const & shader_info, [[maybe_unused]] VkShaderStageFlagBits shader_stage, [[maybe_unused]] ShaderCode const & code) -> Result<std::vector<u32>>
    {
#if DAXA_BUILT_WITH_DXC
        auto u8_ascii_to_wstring = [](char const * str) -> std::wstring
        {
            std::wstring ret = {};
            for (usize i = 0; (i < std::strlen(str) + 1) && str != nullptr; i++)
            {
                ret.push_back(static_cast<wchar_t>(str[i]));
            }
            return ret;
        };

        std::vector<wchar_t const *> args = {};

        std::vector<std::wstring> wstring_buffer = {};

        wstring_buffer.reserve(shader_info.compile_options.defines.size() + 1 + shader_info.compile_options.root_paths.size());

        for (auto const & define : shader_info.compile_options.defines)
        {
            auto define_str = define.name;
            if (define.value.length() > 0)
            {
                define_str = define_str + "=" + define.value;
            }
            wstring_buffer.push_back(u8_ascii_to_wstring(define_str.c_str()));
            args.push_back(L"-D");
            args.push_back(wstring_buffer.back().c_str());
        }
        args.push_back(L"-DDAXA_SHADER");
        args.push_back(L"-DDAXA_SHADERLANG=2");

        if (std::holds_alternative<ShaderFile>(shader_info.source))
        {
            wstring_buffer.push_back(std::get<ShaderFile>(shader_info.source).path.wstring());
            args.push_back(wstring_buffer.back().c_str());
        }

        for (auto const & root : shader_info.compile_options.root_paths)
        {
            args.push_back(L"-I");
            wstring_buffer.push_back(root.wstring());
            args.push_back(wstring_buffer.back().c_str());
        }

        // set matrix packing to column major
        args.push_back(L"-Zpc");
        // set warnings as errors
        args.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
        // setting target
        args.push_back(L"-spirv");
        args.push_back(L"-fspv-target-env=vulkan1.1");
        // set optimization setting
        switch (shader_info.compile_options.opt_level.value())
        {
        case 0: args.push_back(L"-O0"); break;
        case 1: args.push_back(L"-O1"); break;
        case 2: args.push_back(L"-O2"); break;
        case 3: args.push_back(L"-O3"); break;
        default: DAXA_DBG_ASSERT_TRUE_M(false, "Bad optimization level set in Pipeline Compiler"); break;
        }
        // setting entry point
        args.push_back(L"-E");
        auto entry_point_wstr = u8_ascii_to_wstring(shader_info.compile_options.entry_point.value().c_str());
        args.push_back(entry_point_wstr.c_str());
        args.push_back(L"-fvk-use-scalar-layout");

        // set shader model
        args.push_back(L"-T");
        std::wstring profile = L"vs_x_x";
        profile[3] = L'0' + static_cast<wchar_t>(shader_info.compile_options.shader_model.value().major);
        profile[5] = L'0' + static_cast<wchar_t>(shader_info.compile_options.shader_model.value().minor);
        switch (shader_stage)
        {
        case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT: profile[0] = L'v'; break;
        case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT: profile[0] = L'p'; break;
        case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT: profile[0] = L'c'; break;
        default: break;
        }
        args.push_back(profile.c_str());
        // set hlsl version to 2021
        args.push_back(L"-HV");
        args.push_back(L"2021");
        DxcBuffer const source_buffer{
            .Ptr = code.string.c_str(),
            .Size = static_cast<u32>(code.string.size()),
            .Encoding = static_cast<u32>(0),
        };

        IDxcResult * result = nullptr;
        this->dxc_backend.dxc_compiler->Compile(
            &source_buffer, args.data(), static_cast<u32>(args.size()),
            this->dxc_backend.dxc_includer, IID_PPV_ARGS(&result));
        IDxcBlobUtf8 * error_message = nullptr;
        result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&error_message), nullptr);

        if ((error_message != nullptr) && error_message->GetStringLength() > 0)
        {
            auto str = std::string();
            str.resize(error_message->GetBufferSize());
            for (usize i = 0; i < str.size(); i++)
            {
                str[i] = static_cast<char const *>(error_message->GetBufferPointer())[i];
            }
            str = std::string("DXC: ") + str;
            return Result<std::vector<u32>>(str);
        }

        IDxcBlob * shader_obj = nullptr;
        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader_obj), nullptr);

        std::vector<u32> spv;
        spv.resize(shader_obj->GetBufferSize() / sizeof(u32));
        for (usize i = 0; i < spv.size(); i++)
        {
            spv[i] = static_cast<u32 *>(shader_obj->GetBufferPointer())[i];
        }
        return Result<std::vector<u32>>(spv);
#else
        return Result<std::vector<u32>>("Asked for Dxc compilation without enabling Dxc");
#endif
    }

    ImplRasterPipeline::ImplRasterPipeline(ManagedWeakPtr a_impl_device, RasterPipelineInfo a_info)
        : ImplPipeline(std::move(a_impl_device)), info{std::move(a_info)}
    {
    }

    ImplComputePipeline::ImplComputePipeline(ManagedWeakPtr a_impl_device, ComputePipelineInfo a_info)
        : ImplPipeline(std::move(a_impl_device)), info{std::move(a_info)}
    {
    }

    ImplPipeline::ImplPipeline(ManagedWeakPtr a_impl_device)
        : impl_device{std::move(a_impl_device)}
    {
    }

    ImplPipeline::~ImplPipeline() // NOLINT(bugprone-exception-escape)
    {
        auto * device = this->impl_device.as<ImplDevice>();
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{device->main_queue_zombies_mtx});
        u64 const main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(device->main_queue_cpu_timeline);
        device->main_queue_pipeline_zombies.push_front({
            main_queue_cpu_timeline_value,
            PipelineZombie{
                .vk_pipeline = vk_pipeline,
            },
        });
    }
} // namespace daxa
