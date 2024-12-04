#pragma once

#if defined(GLSLANG_WRAPPER_INTERNAL)
#define GLSLANG_WRAPPER_DLL_EXPORT __declspec(dllexport)
#else
#define GLSLANG_WRAPPER_DLL_EXPORT __declspec(dllimport)
typedef enum
{
    EShLangVertex,
    EShLangTessControl,
    EShLangTessEvaluation,
    EShLangGeometry,
    EShLangFragment,
    EShLangCompute,
    EShLangRayGen,
    EShLangIntersect,
    EShLangAnyHit,
    EShLangClosestHit,
    EShLangMiss,
    EShLangCallable,
    EShLangTask,
    EShLangMesh,
    EShLangCount,
} EShLanguage;
#endif

void GLSLANG_WRAPPER_DLL_EXPORT glslang_wrapper_init();
void GLSLANG_WRAPPER_DLL_EXPORT glslang_wrapper_deinit();

struct GlslangWrapperHeaderResult
{
    char const * header_name;
    size_t header_name_length;
    char const * header_code;
    size_t header_code_length;
};

using IncludeCallback = void(void * user_pointer, char const * header_name, char const * includer_name, GlslangWrapperHeaderResult & result);
using ReleaseStringCallback = void(char const * str);

struct GlslangWrapperCompileInfo
{
    EShLanguage stage;
    char const * preamble;
    char const * shader_glsl;
    char const * shader_name;
    char const * entry_point;
    char const * source_entry;
    bool use_debug_info;

    IncludeCallback * include_local_cb;
    IncludeCallback * include_system_cb;
    ReleaseStringCallback * release_string_cb;
    void * user_pointer;

    unsigned int ** out_spv_ptr;
    unsigned int * out_spv_size;
    char const ** out_error_str;
    unsigned int * out_error_str_size;
};

void GLSLANG_WRAPPER_DLL_EXPORT glslang_wrapper_compile(GlslangWrapperCompileInfo const & info);
void GLSLANG_WRAPPER_DLL_EXPORT glslang_wrapper_release_results(unsigned int * spv_ptr, char const * error_str);
