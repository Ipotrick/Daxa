#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Include/ResourceLimits.h>

#define GLSLANG_WRAPPER_INTERNAL
#include "wrapper.hpp"

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
    // TODO: Verify these values are reasonable:
    .maxMeshOutputVerticesEXT = 512,
    .maxMeshOutputPrimitivesEXT = 512,
    .maxMeshWorkGroupSizeX_EXT = 512,
    .maxMeshWorkGroupSizeY_EXT = 512,
    .maxMeshWorkGroupSizeZ_EXT = 512,
    .maxTaskWorkGroupSizeX_EXT = 512,
    .maxTaskWorkGroupSizeY_EXT = 512,
    .maxTaskWorkGroupSizeZ_EXT = 512,
    .maxMeshViewCountEXT = 2,
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

class GlslangFileIncluder : public glslang::TShader::Includer
{
  public:
    constexpr static inline size_t DELETE_SOURCE_NAME = 0x1;
    constexpr static inline size_t DELETE_CONTENT = 0x2;
    constexpr static inline size_t MAX_INCLUSION_DEPTH = 100;

    GlslangWrapperCompileInfo const * info;

    auto includeLocal(
        char const * header_name, char const * includer_name, size_t inclusion_depth) -> IncludeResult * override
    {
        if (inclusion_depth > MAX_INCLUSION_DEPTH)
            return nullptr;
        GlslangWrapperHeaderResult result{.header_name = nullptr};
        info->include_local_cb(info->user_pointer, header_name, includer_name, result);
        if (header_name == nullptr)
        {
            info->release_string_cb(result.header_name);
            return nullptr;
        }
        auto inc_result = new IncludeResult(std::string(result.header_name, result.header_name_length), result.header_code, result.header_code_length, nullptr);
        info->release_string_cb(result.header_name);
        return inc_result;
    }

    auto includeSystem(
        char const * header_name, char const * includer_name, size_t inclusion_depth) -> IncludeResult * override
    {
        if (inclusion_depth > MAX_INCLUSION_DEPTH)
            return nullptr;
        GlslangWrapperHeaderResult result{.header_name = nullptr};
        info->include_system_cb(info->user_pointer, header_name, includer_name, result);
        if (header_name == nullptr)
        {
            info->release_string_cb(result.header_name);
            return nullptr;
        }
        auto inc_result = new IncludeResult(std::string(result.header_name, result.header_name_length), result.header_code, result.header_code_length, nullptr);
        info->release_string_cb(result.header_name);
        return inc_result;
    }

    void releaseInclude(IncludeResult * result) override
    {
        if (result != nullptr)
        {
            info->release_string_cb(result->headerData);
            delete result;
        }
    }
};

void GLSLANG_WRAPPER_DLL_EXPORT glslang_wrapper_init()
{
    glslang::InitializeProcess();
}
void GLSLANG_WRAPPER_DLL_EXPORT glslang_wrapper_deinit()
{
    glslang::FinalizeProcess();
}

void GLSLANG_WRAPPER_DLL_EXPORT glslang_wrapper_compile(GlslangWrapperCompileInfo const & info)
{
    glslang::TShader shader{info.stage};
    shader.setPreamble(info.preamble);

    shader.setStringsWithLengthsAndNames(&info.shader_glsl, nullptr, &info.shader_name, 1);
    shader.setEntryPoint(info.entry_point);
    shader.setSourceEntryPoint(info.source_entry);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);

    glslang::SpvOptions spv_options{};
    spv_options.generateDebugInfo = info.use_debug_info; // -g
    // spv_options.emitNonSemanticShaderDebugInfo = use_debug_info; // -gV
    // if (spv_options.emitNonSemanticShaderDebugInfo)
    //     spv_options.emitNonSemanticShaderDebugSource = use_debug_info; // -gVS
    spv_options.stripDebugInfo = !info.use_debug_info;

    if (spv_options.emitNonSemanticShaderDebugInfo)
        shader.setDebugInfo(info.use_debug_info);

    spv_options.disableOptimizer = info.use_debug_info;

    GlslangFileIncluder includer;
    includer.info = &info;
    auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
    TBuiltInResource const resource = DAXA_DEFAULT_BUILTIN_RESOURCE;

    static constexpr int SHADER_VERSION = 460;

    auto error_message_prefix = std::string("GLSLANG [") + info.shader_name + "] ";

    if (!shader.parse(&resource, SHADER_VERSION, false, messages, includer))
    {
        auto error_str = error_message_prefix + shader.getInfoLog() + shader.getInfoDebugLog();
        auto cstr = new char[error_str.size() + 1];
        memcpy(cstr, error_str.data(), error_str.size());
        cstr[error_str.size()] = '\0';
        *info.out_error_str = cstr;
        *info.out_error_str_size = error_str.size();
        return;
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(messages))
    {
        auto error_str = error_message_prefix + program.getInfoLog() + program.getInfoDebugLog();
        auto cstr = new char[error_str.size() + 1];
        memcpy(cstr, error_str.data(), error_str.size());
        cstr[error_str.size()] = '\0';
        *info.out_error_str = cstr;
        *info.out_error_str_size = error_str.size();
        return;
    }

    auto * intermediary = program.getIntermediate(info.stage);
    if (intermediary == nullptr)
    {
        auto error_str = error_message_prefix + std::string("Failed to get shader stage intermediary");
        auto cstr = new char[error_str.size() + 1];
        memcpy(cstr, error_str.data(), error_str.size());
        cstr[error_str.size()] = '\0';
        *info.out_error_str = cstr;
        *info.out_error_str_size = error_str.size();
        return;
    }

    spv::SpvBuildLogger logger;

    std::vector<uint32_t> spv;
    glslang::GlslangToSpv(*intermediary, spv, &logger, &spv_options);

    auto cspv = new uint32_t[spv.size()];
    memcpy(cspv, spv.data(), spv.size() * sizeof(uint32_t));
    *info.out_spv_ptr = cspv;
    *info.out_spv_size = spv.size();
}

void GLSLANG_WRAPPER_DLL_EXPORT glslang_wrapper_release_results(unsigned int * spv_ptr, char const * error_str)
{
    if (spv_ptr != nullptr)
    {
        delete[] spv_ptr;
    }
    if (error_str != nullptr)
    {
        delete[] error_str;
    }
}
