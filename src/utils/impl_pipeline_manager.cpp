#if DAXA_BUILT_WITH_UTILS

#include "../impl_core.hpp"
#include "impl_pipeline_manager.hpp"

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

        ImplPipelineManager * impl_pipeline_manager = nullptr;

        static auto process_include(daxa::Result<daxa::ShaderCode> const & shader_code_result, std::filesystem::path const & full_path) -> IncludeResult *
        {
            std::string headerName = {};
            char const * headerData = nullptr;
            size_t headerLength = 0;

            if (shader_code_result.is_err())
            {
                return nullptr;
            }

            auto const & shader_code_str = shader_code_result.value().string;
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

        auto includeLocal(
            char const * header_name, char const * includer_name, size_t inclusion_depth) -> IncludeResult * override
        {
            constexpr usize MAX_INCLUSION_DEPTH = 100;
            if (inclusion_depth > MAX_INCLUSION_DEPTH)
            {
                return nullptr;
            }
            auto result = impl_pipeline_manager->full_path_to_file(includer_name);
            if (result.is_err())
            {
                return nullptr;
            }
            auto full_path = result.value().parent_path() / header_name;
            impl_pipeline_manager->current_observed_hotload_files->insert({full_path, std::chrono::file_clock::now()});
            auto shader_code_result = impl_pipeline_manager->load_shader_source_from_file(full_path);
            return process_include(shader_code_result, full_path);
        }

        auto includeSystem(
            char const * header_name, char const * /*includer_name*/, size_t /*inclusion_depth*/) -> IncludeResult * override
        {
            auto result = impl_pipeline_manager->full_path_to_file(header_name);
            if (result.is_err())
            {
                return nullptr;
            }
            auto full_path = result.value();
            auto search_pred = [&](std::filesystem::path const & p)
            { return p == full_path; };
            if (std::find_if(
                    impl_pipeline_manager->current_seen_shader_files.begin(),
                    impl_pipeline_manager->current_seen_shader_files.end(),
                    search_pred) != impl_pipeline_manager->current_seen_shader_files.end())
            {
                return nullptr;
            }
            impl_pipeline_manager->current_observed_hotload_files->insert({full_path, std::chrono::file_clock::now()});
            auto shader_code_result = impl_pipeline_manager->load_shader_source_from_file(full_path);
            return process_include(shader_code_result, full_path);
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
        ImplPipelineManager * impl_pipeline_manager{};

        virtual ~DxcCustomIncluder() = default;
        auto LoadSource(LPCWSTR filename, IDxcBlob ** include_source) -> HRESULT override
        {
            if (filename[0] == '.')
            {
                filename += 2;
            }
            auto result = impl_pipeline_manager->full_path_to_file(filename);
            if (result.is_err())
            {
                *include_source = nullptr;
                return SCARD_E_FILE_NOT_FOUND;
            }
            auto full_path = result.value();
            auto search_pred = [&](std::filesystem::path const & p)
            { return p == full_path; };
            ComPtr<IDxcBlobEncoding> dxc_blob_encoding = {};
            if (std::find_if(impl_pipeline_manager->current_seen_shader_files.begin(),
                             impl_pipeline_manager->current_seen_shader_files.end(), search_pred) != impl_pipeline_manager->current_seen_shader_files.end())
            {
                // Return empty string blob if this file has been included before
                static char const * const null_str = " ";
                impl_pipeline_manager->dxc_backend.dxc_utils->CreateBlob(null_str, static_cast<u32>(strlen(null_str)), CP_UTF8, &dxc_blob_encoding);
                *include_source = dxc_blob_encoding.Detach();
                return S_OK;
            }
            else
            {
                impl_pipeline_manager->current_observed_hotload_files->insert({full_path, std::chrono::file_clock::now()});
            }
            auto str_result = impl_pipeline_manager->load_shader_source_from_file(full_path);
            if (str_result.is_err())
            {
                *include_source = nullptr;
                return SCARD_E_INVALID_PARAMETER;
            }
            std::string const str = str_result.value().string;
            impl_pipeline_manager->dxc_backend.dxc_utils->CreateBlob(str.c_str(), static_cast<u32>(str.size()), CP_UTF8, &dxc_blob_encoding);
            *include_source = dxc_blob_encoding.Detach();
            return S_OK;
        }

        auto QueryInterface(REFIID riid, void ** object) -> HRESULT override
        {
            return default_includer->QueryInterface(riid, object);
        }

        auto STDMETHODCALLTYPE AddRef() -> unsigned long override { return 0; }
        auto STDMETHODCALLTYPE Release() -> unsigned long override { return 0; }
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
        if (!this->write_out_preprocessed_code.has_value())
        {
            this->write_out_preprocessed_code = other.write_out_preprocessed_code;
        }
        if (!this->write_out_shader_binary.has_value())
        {
            this->write_out_shader_binary = other.write_out_shader_binary;
        }
        if (!this->language.has_value())
        {
            this->language = other.language;
        }
        if (!this->enable_debug_info.has_value())
        {
            this->enable_debug_info = other.enable_debug_info;
        }

        this->root_paths.insert(this->root_paths.begin(), other.root_paths.begin(), other.root_paths.end());
        this->defines.insert(this->defines.end(), other.defines.begin(), other.defines.end());
    }

    PipelineManager::PipelineManager(PipelineManagerInfo info) : ManagedPtr(new ImplPipelineManager{std::move(info)}) {}

    auto PipelineManager::add_compute_pipeline(ComputePipelineCompileInfo const & info) -> Result<std::shared_ptr<ComputePipeline>>
    {
        auto & impl = *reinterpret_cast<ImplPipelineManager *>(this->object);
        return impl.add_compute_pipeline(info);
    }

    auto PipelineManager::add_raster_pipeline(RasterPipelineCompileInfo const & info) -> Result<std::shared_ptr<RasterPipeline>>
    {
        auto & impl = *reinterpret_cast<ImplPipelineManager *>(this->object);
        return impl.add_raster_pipeline(info);
    }

    void PipelineManager::remove_compute_pipeline(std::shared_ptr<ComputePipeline> const & pipeline)
    {
        auto & impl = *reinterpret_cast<ImplPipelineManager *>(this->object);
        return impl.remove_compute_pipeline(pipeline);
    }

    void PipelineManager::remove_raster_pipeline(std::shared_ptr<RasterPipeline> const & pipeline)
    {
        auto & impl = *reinterpret_cast<ImplPipelineManager *>(this->object);
        return impl.remove_raster_pipeline(pipeline);
    }

    auto PipelineManager::reload_all() -> Result<bool>
    {
        auto & impl = *reinterpret_cast<ImplPipelineManager *>(this->object);
        return impl.reload_all();
    }

    ImplPipelineManager::ImplPipelineManager(PipelineManagerInfo && a_info)
        : info{std::move(a_info)}
    {
        if (!this->info.shader_compile_options.entry_point.has_value())
        {
            this->info.shader_compile_options.entry_point = std::optional<std::string>{"main"};
        }
        if (!this->info.shader_compile_options.language.has_value())
        {
            this->info.shader_compile_options.language = std::optional<ShaderLanguage>{ShaderLanguage::HLSL};
        }
        if (!this->info.shader_compile_options.enable_debug_info.has_value())
        {
            this->info.shader_compile_options.enable_debug_info = {false};
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

            this->dxc_backend.dxc_includer = std::make_shared<DxcCustomIncluder>();
            dynamic_cast<DxcCustomIncluder &>(*this->dxc_backend.dxc_includer).impl_pipeline_manager = this;
            this->dxc_backend.dxc_utils->CreateDefaultIncludeHandler(&dynamic_cast<DxcCustomIncluder &>(*this->dxc_backend.dxc_includer).default_includer);
        }
#endif
    }

    ImplPipelineManager::~ImplPipelineManager()
    {
#if DAXA_BUILT_WITH_GLSLANG
        {
            glslang::FinalizeProcess();
        }
#endif
    }

    auto ImplPipelineManager::create_compute_pipeline(ComputePipelineCompileInfo const & a_info) -> Result<ComputePipelineState>
    {
        auto modified_info = a_info;
        modified_info.shader_info.compile_options.inherit(this->info.shader_compile_options);
        if (modified_info.push_constant_size > MAX_PUSH_CONSTANT_BYTE_SIZE)
        {
            return Result<ComputePipelineState>(std::string("push constant size of ") + std::to_string(modified_info.push_constant_size) + std::string(" exceeds the maximum size of ") + std::to_string(MAX_PUSH_CONSTANT_BYTE_SIZE));
        }
        if (modified_info.push_constant_size % 4 != 0)
        {
            return Result<ComputePipelineState>(std::string("push constant size of ") + std::to_string(modified_info.push_constant_size) + std::string(" is not a multiple of 4(bytes)"));
        }
        auto pipe_result = ComputePipelineState{
            .pipeline_ptr = std::make_shared<ComputePipeline>(),
            .info = modified_info,
            .last_hotload_time = std::chrono::file_clock::now(),
            .observed_hotload_files = {},
        };
        this->current_observed_hotload_files = &pipe_result.observed_hotload_files;
        auto spirv_result = get_spirv(pipe_result.info.shader_info, ShaderStage::COMP);
        if (spirv_result.is_err())
        {
            return Result<ComputePipelineState>(spirv_result.message());
        }
        (*pipe_result.pipeline_ptr) = this->info.device.create_compute_pipeline({
            .shader_info = {
                .binary = std::move(spirv_result.value()),
                .entry_point = a_info.shader_info.compile_options.entry_point,
            },
            .push_constant_size = modified_info.push_constant_size,
            .debug_name = modified_info.debug_name,
        });
        return Result<ComputePipelineState>(std::move(pipe_result));
    }

    auto ImplPipelineManager::create_raster_pipeline(RasterPipelineCompileInfo const & a_info) -> Result<RasterPipelineState>
    {
        auto modified_info = a_info;
        modified_info.vertex_shader_info.compile_options.inherit(this->info.shader_compile_options);
        modified_info.fragment_shader_info.compile_options.inherit(this->info.shader_compile_options);
        if (modified_info.push_constant_size > MAX_PUSH_CONSTANT_BYTE_SIZE)
        {
            return Result<RasterPipelineState>(std::string("push constant size of ") + std::to_string(modified_info.push_constant_size) + std::string(" exceeds the maximum size of ") + std::to_string(MAX_PUSH_CONSTANT_BYTE_SIZE));
        }
        if (modified_info.push_constant_size % 4 != 0)
        {
            return Result<RasterPipelineState>(std::string("push constant size of ") + std::to_string(modified_info.push_constant_size) + std::string(" is not a multiple of 4(bytes)"));
        }
        auto pipe_result = RasterPipelineState{
            .pipeline_ptr = std::make_shared<RasterPipeline>(),
            .info = modified_info,
            .last_hotload_time = std::chrono::file_clock::now(),
            .observed_hotload_files = {},
        };
        this->current_observed_hotload_files = &pipe_result.observed_hotload_files;
        auto vert_spirv_result = get_spirv(pipe_result.info.vertex_shader_info, ShaderStage::VERT);
        if (vert_spirv_result.is_err())
        {
            return Result<RasterPipelineState>(vert_spirv_result.message());
        }
        auto frag_spirv_result = get_spirv(pipe_result.info.fragment_shader_info, ShaderStage::FRAG);
        if (frag_spirv_result.is_err())
        {
            return Result<RasterPipelineState>(frag_spirv_result.message());
        }
        (*pipe_result.pipeline_ptr) = this->info.device.create_raster_pipeline({
            .vertex_shader_info = {
                .binary = std::move(vert_spirv_result.value()),
                .entry_point = a_info.vertex_shader_info.compile_options.entry_point,
            },
            .fragment_shader_info = {
                .binary = std::move(frag_spirv_result.value()),
                .entry_point = a_info.fragment_shader_info.compile_options.entry_point,
            },
            .color_attachments = modified_info.color_attachments,
            .depth_test = modified_info.depth_test,
            .raster = modified_info.raster,
            .push_constant_size = modified_info.push_constant_size,
            .debug_name = modified_info.debug_name,
        });
        return Result<RasterPipelineState>(std::move(pipe_result));
    }

    auto ImplPipelineManager::add_compute_pipeline(ComputePipelineCompileInfo const & a_info) -> Result<std::shared_ptr<ComputePipeline>>
    {
        auto pipe_result = create_compute_pipeline(a_info);
        if (pipe_result.is_err())
        {
            return Result<std::shared_ptr<ComputePipeline>>(pipe_result.m);
        }
        this->compute_pipelines.push_back(pipe_result.value());
        return Result<std::shared_ptr<ComputePipeline>>(std::move(pipe_result.value().pipeline_ptr));
    }

    auto ImplPipelineManager::add_raster_pipeline(RasterPipelineCompileInfo const & a_info) -> Result<std::shared_ptr<RasterPipeline>>
    {
        auto pipe_result = create_raster_pipeline(a_info);
        if (pipe_result.is_err())
        {
            return Result<std::shared_ptr<RasterPipeline>>(pipe_result.m);
        }
        this->raster_pipelines.push_back(pipe_result.value());
        return Result<std::shared_ptr<RasterPipeline>>(std::move(pipe_result.value().pipeline_ptr));
    }

    void ImplPipelineManager::remove_compute_pipeline(std::shared_ptr<ComputePipeline> const & pipeline)
    {
        auto pipeline_iter = std::find_if(
            this->compute_pipelines.begin(),
            this->compute_pipelines.end(),
            [&pipeline](std::shared_ptr<ComputePipeline> const & other)
            {
                return pipeline.get() == other.get();
            });
        if (pipeline_iter == this->compute_pipelines.end())
        {
            return;
        }
        this->compute_pipelines.erase(pipeline_iter);
    }

    void ImplPipelineManager::remove_raster_pipeline(std::shared_ptr<RasterPipeline> const & pipeline)
    {
        auto pipeline_iter = std::find_if(
            this->raster_pipelines.begin(),
            this->raster_pipelines.end(),
            [&pipeline](std::shared_ptr<RasterPipeline> const & other)
            {
                return pipeline.get() == other.get();
            });
        if (pipeline_iter == this->raster_pipelines.end())
        {
            return;
        }
        this->raster_pipelines.erase(pipeline_iter);
    }

    auto ImplPipelineManager::reload_all() -> Result<bool>
    {
        auto check_if_sources_changed = [&](std::chrono::file_clock::time_point & last_hotload_time, ShaderFileTimeSet & observed_hotload_files) -> bool
        {
            using namespace std::chrono_literals;
            static constexpr auto HOTRELOAD_MIN_TIME = 250ms;

            auto now = std::chrono::file_clock::now();
            using namespace std::chrono_literals;
            if (now - last_hotload_time < HOTRELOAD_MIN_TIME)
            {
                return false;
            }
            last_hotload_time = now;
            bool reload = false;
            for (auto & [path, recorded_write_time] : observed_hotload_files)
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
                for (auto & pair : observed_hotload_files)
                {
                    auto ifs = std::ifstream(pair.first);
                    if (ifs.good())
                    {
                        pair.second = std::filesystem::last_write_time(pair.first);
                    }
                }
            }
            return reload;
        };

        bool reloaded = false;

        for (auto & [pipeline, compile_info, last_hotload_time, observed_hotload_files] : this->compute_pipelines)
        {
            if (check_if_sources_changed(last_hotload_time, observed_hotload_files))
            {
                auto new_pipeline = create_compute_pipeline(compile_info);
                if (new_pipeline.is_ok())
                {
                    *pipeline = std::move(*new_pipeline.value().pipeline_ptr);
                    reloaded = true;
                }
                else
                {
                    return Result<bool>(new_pipeline.m);
                }
            }
        }

        for (auto & [pipeline, compile_info, last_hotload_time, observed_hotload_files] : this->raster_pipelines)
        {
            if (check_if_sources_changed(last_hotload_time, observed_hotload_files))
            {
                auto new_pipeline = create_raster_pipeline(compile_info);
                if (new_pipeline.is_ok())
                {
                    *pipeline = std::move(*new_pipeline.value().pipeline_ptr);
                    reloaded = true;
                }
                else
                {
                    return Result<bool>(new_pipeline.m);
                }
            }
        }

        return Result<bool>(reloaded);
    }

    auto ImplPipelineManager::get_spirv(ShaderCompileInfo const & shader_info, ShaderStage shader_stage) -> Result<std::vector<u32>>
    {
        current_shader_info = &shader_info;
        std::vector<u32> spirv = {};
        if (std::holds_alternative<ShaderBinary>(shader_info.source))
        {
            spirv = std::get<ShaderBinary>(shader_info.source);
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
                // This is a hack. Instead of providing the file as source code, we provide the full path.
                code = {.string = std::string("#include \"") + ret.value().string() + "\"\n"};
                // auto code_ret = load_shader_source_from_file(ret.value());
                // if (code_ret.is_err())
                // {
                //     return Result<std::vector<u32>>(code_ret.message());
                // }
                // code = code_ret.value();
            }
            else
            {
                code = std::get<ShaderCode>(shader_info.source);
            }

            Result<std::vector<u32>> ret = Result<std::vector<u32>>("No shader was compiled");

            assert(shader_info.compile_options.language.has_value() && "How did this happen? You mustn't provide a nullopt for the language");

            DAXA_DBG_ASSERT_TRUE_M(shader_info.compile_options.language.has_value(), "You must have a shader language set when compiling GLSL");
            switch (shader_info.compile_options.language.value())
            {
#if DAXA_BUILT_WITH_GLSLANG
            case ShaderLanguage::GLSL:
                ret = get_spirv_glslang(shader_info, shader_stage, code);
                break;
#endif
#if DAXA_BUILT_WITH_DXC
            case ShaderLanguage::HLSL:
                ret = get_spirv_dxc(shader_info, shader_stage, code);
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

        std::string debug_name = "unnamed-shader";
        if (ShaderFile const * shader_file = std::get_if<ShaderFile>(&shader_info.source))
        {
            debug_name = shader_file->path.string();
        }
        else if (!shader_info.debug_name.empty())
        {
            debug_name = shader_info.debug_name;
        }

        if (shader_info.compile_options.write_out_shader_binary.has_value())
        {
            std::replace(debug_name.begin(), debug_name.end(), '/', '_');
            std::replace(debug_name.begin(), debug_name.end(), '\\', '_');
            std::replace(debug_name.begin(), debug_name.end(), ':', '_');
            debug_name = debug_name + ".spv";
            std::ofstream ofs(shader_info.compile_options.write_out_shader_binary.value() / debug_name, std::ios_base::trunc | std::ios_base::binary);
            ofs.write(reinterpret_cast<char const *>(spirv.data()), static_cast<std::streamsize>(spirv.size() * 4));
            ofs.close();
        }
        return Result<std::vector<u32>>(spirv);
    }

    auto ImplPipelineManager::full_path_to_file(std::filesystem::path const & path) -> Result<std::filesystem::path>
    {
        if (std::filesystem::exists(path))
        {
            return Result<std::filesystem::path>(path);
        }
        std::filesystem::path potential_path;
        if (this->current_shader_info != nullptr)
        {
            for (auto const & root : this->current_shader_info->compile_options.root_paths)
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

    auto ImplPipelineManager::load_shader_source_from_file(std::filesystem::path const & path) -> Result<ShaderCode>
    {
        auto result_path = full_path_to_file(path);
        if (result_path.is_err())
        {
            return Result<ShaderCode>(result_path.message());
        }
        auto start_time = std::chrono::steady_clock::now();
        using namespace std::chrono_literals;
        while (std::chrono::duration<f32>(std::chrono::steady_clock::now() - start_time) < 0.1s)
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

    auto ImplPipelineManager::get_spirv_glslang(ShaderCompileInfo const & shader_info, ShaderStage shader_stage, ShaderCode const & code) -> Result<std::vector<u32>>
    {
#if DAXA_BUILT_WITH_GLSLANG
        auto translate_shader_stage = [](ShaderStage stage) -> EShLanguage
        {
            switch (stage)
            {
            case ShaderStage::COMP: return EShLanguage::EShLangCompute;
            case ShaderStage::VERT: return EShLanguage::EShLangVertex;
            case ShaderStage::FRAG: return EShLanguage::EShLangFragment;
            default:
                DAXA_DBG_ASSERT_TRUE_M(false, "Tried creating shader with unknown shader stage");
                return EShLanguage::EShLangCount;
            }
        };

        std::string preamble;

        auto spirv_stage = translate_shader_stage(shader_stage);

        // NOTE: You can't set #version in the preamble. However,
        // you can set the version as a `shader.` parameter. This
        // is unnecessary, though, since it appears to default to
        // the latest version.
        // preamble += "#version 450\n";
        preamble += "#define DAXA_SHADER 1\n";
        preamble += "#define DAXA_SHADERLANG 1\n";
        preamble += "#extension GL_GOOGLE_include_directive : enable\n";
        preamble += "#extension GL_EXT_nonuniform_qualifier : enable\n";
        preamble += "#extension GL_EXT_buffer_reference : enable\n";
        preamble += "#extension GL_EXT_buffer_reference2 : enable\n";
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
        preamble += "#extension GL_EXT_shader_image_load_formatted : require\n";
        preamble += "#extension GL_EXT_control_flow_attributes : require\n";
        preamble += "#extension GL_EXT_shader_image_int64 : require\n";
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
        std::string debug_name = "unnamed-shader";
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
        auto const * source_cstr = source_str.c_str();
        auto const * name_cstr = debug_name.c_str();

        bool use_debug_info = shader_info.compile_options.enable_debug_info.value_or(false);

        shader.setStringsWithLengthsAndNames(&source_cstr, nullptr, &name_cstr, 1);
        shader.setEntryPoint("main");
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);

        // NOTE: For some reason, this causes a crash in GLSLANG
        // shader.setDebugInfo(use_debug_info);

        GlslangFileIncluder includer;
        includer.impl_pipeline_manager = this;
        auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
        TBuiltInResource const resource = DAXA_DEFAULT_BUILTIN_RESOURCE;

        static constexpr int SHADER_VERSION = 450;

        if (!shader.parse(&resource, SHADER_VERSION, false, messages, includer))
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
        spv_options.generateDebugInfo = use_debug_info;

        // NOTE: For some reason, this also causes a crash in GLSLANG
        // spv_options.emitNonSemanticShaderDebugInfo = use_debug_info;

        spv_options.stripDebugInfo = !use_debug_info;
        std::vector<u32> spv;
        glslang::GlslangToSpv(*intermediary, spv, &logger, &spv_options);

        if (shader_info.compile_options.write_out_preprocessed_code.has_value())
        {
            std::replace(debug_name.begin(), debug_name.end(), '/', '_');
            std::replace(debug_name.begin(), debug_name.end(), '\\', '_');
            std::replace(debug_name.begin(), debug_name.end(), ':', '_');
            std::string const name = std::string("preprocessed_") + debug_name + ".glsl";
            auto filepath = shader_info.compile_options.write_out_preprocessed_code.value() / name;
            std::string preprocessed_result = {};
            shader.preprocess(&DAXA_DEFAULT_BUILTIN_RESOURCE, SHADER_VERSION, EProfile::ENoProfile, false, false, messages, &preprocessed_result, includer);
            auto ofs = std::ofstream{filepath, std::ios_base::trunc};
            ofs.write(preprocessed_result.data(), static_cast<std::streamsize>(preprocessed_result.size()));
            ofs.close();
        }
        return Result<std::vector<u32>>(spv);
#else
        return Result<std::vector<u32>>("Asked for glslang compilation without enabling glslang");
#endif
    }

    auto ImplPipelineManager::get_spirv_dxc(ShaderCompileInfo const & shader_info, ShaderStage shader_stage, ShaderCode const & code) -> Result<std::vector<u32>>
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
                define_str += "=";
                define_str += define.value;
            }
            wstring_buffer.push_back(u8_ascii_to_wstring(define_str.c_str()));
            args.push_back(L"-D");
            args.push_back(wstring_buffer.back().c_str());
        }
        args.push_back(L"-DDAXA_SHADER");
        args.push_back(L"-DDAXA_SHADERLANG=2");

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
        // setting entry point
        args.push_back(L"-E");
        auto entry_point_wstr = u8_ascii_to_wstring(shader_info.compile_options.entry_point.value_or("main").c_str());
        args.push_back(entry_point_wstr.c_str());
        args.push_back(L"-fvk-use-scalar-layout");
        if (shader_info.compile_options.enable_debug_info.value_or(false))
        {
            // insert debug info
            args.push_back(L"-Zi");
            args.push_back(L"-fspv-debug=line");
        }

        // set shader model explicitly to 6.6
        args.push_back(L"-T");
        std::wstring profile = L"vs_6_6";
        switch (shader_stage)
        {
        case ShaderStage::COMP: profile[0] = L'c'; break;
        case ShaderStage::VERT: profile[0] = L'v'; break;
        case ShaderStage::FRAG: profile[0] = L'p'; break;
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
        dynamic_cast<DxcCustomIncluder &>(*this->dxc_backend.dxc_includer).impl_pipeline_manager = this;
        this->dxc_backend.dxc_compiler->Compile(
            &source_buffer, args.data(), static_cast<u32>(args.size()),
            this->dxc_backend.dxc_includer.get(), IID_PPV_ARGS(&result));
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
} // namespace daxa

#endif
