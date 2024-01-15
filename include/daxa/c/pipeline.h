#ifndef __DAXA_PIPELINE_H__
#define __DAXA_PIPELINE_H__

#include <daxa/c/types.h>


typedef struct
{
    uint32_t const * byte_code;
    uint32_t byte_code_size;
    daxa_SmallString entry_point;
} daxa_ShaderInfo;
_DAXA_DECL_OPTIONAL(daxa_ShaderInfo)

// RAY TRACING PIPELINE 
typedef struct
{
    daxa_ShaderInfo info;
} daxa_RayTracingShaderInfo;
_DAXA_DECL_FIXED_LIST(daxa_RayTracingShaderInfo, 10)

typedef struct {
    // TODO: daxa types?
    VkRayTracingShaderGroupTypeKHR type;
    uint32_t general_shader_index;
    uint32_t closest_hit_shader_index;
    uint32_t any_hit_shader_index;
    uint32_t intersection_shader_index;
} daxa_RayTracingShaderGroupInfo;
_DAXA_DECL_FIXED_LIST(daxa_RayTracingShaderGroupInfo, 50)

#define DAXA_RAY_TRACING_SHADER_GROUP_INFO_DEFAULT { \
    .group_type = 0, \
    .general_shader_index = VK_SHADER_UNUSED_KHR, \
    .closest_hit_shader_index = VK_SHADER_UNUSED_KHR, \
    .any_hit_shader_index = VK_SHADER_UNUSED_KHR, \
    .intersection_shader_index = VK_SHADER_UNUSED_KHR \
}

typedef struct
{
    // TODO: dynamic size?
    daxa_FixedList(daxa_RayTracingShaderInfo, 10) ray_gen_stages;
    daxa_FixedList(daxa_RayTracingShaderInfo, 10) miss_stages;
    daxa_FixedList(daxa_RayTracingShaderInfo, 10) callable_stages;
    daxa_FixedList(daxa_RayTracingShaderInfo, 10) intersection_stages;
    daxa_FixedList(daxa_RayTracingShaderInfo, 10) closest_hit_stages;
    daxa_FixedList(daxa_RayTracingShaderInfo, 10) any_hit_stages;
    daxa_FixedList(daxa_RayTracingShaderGroupInfo, 50) shader_groups;
    uint32_t max_ray_recursion_depth;
    uint32_t push_constant_size;
    daxa_SmallString name;
} daxa_RayTracingPipelineInfo;

DAXA_EXPORT daxa_RayTracingPipelineInfo const *
daxa_ray_tracing_pipeline_info(daxa_RayTracingPipeline ray_tracing_pipeline);

DAXA_EXPORT uint64_t
daxa_ray_tracing_pipeline_inc_refcnt(daxa_RayTracingPipeline pipeline);
DAXA_EXPORT uint64_t
daxa_ray_tracing_pipeline_dec_refcnt(daxa_RayTracingPipeline pipeline);


// COMPUTE PIPELINE

typedef struct
{
    daxa_ShaderInfo shader_info;
    uint32_t push_constant_size;
    daxa_SmallString name;
} daxa_ComputePipelineInfo;

DAXA_EXPORT daxa_ComputePipelineInfo const *
daxa_compute_pipeline_info(daxa_ComputePipeline compute_pipeline);

DAXA_EXPORT uint64_t
daxa_compute_pipeline_inc_refcnt(daxa_ComputePipeline pipeline);
DAXA_EXPORT uint64_t
daxa_compute_pipeline_dec_refcnt(daxa_ComputePipeline pipeline);


// RASTER PIPELINE

typedef struct
{
    VkFormat depth_attachment_format;
    daxa_Bool8 enable_depth_write;
    VkCompareOp depth_test_compare_op;
    float min_depth_bounds;
    float max_depth_bounds;
} daxa_DepthTestInfo;
_DAXA_DECL_OPTIONAL(daxa_DepthTestInfo)

static const daxa_DepthTestInfo DAXA_DEFAULT_DEPTH_TEST_INFO = {
    .depth_attachment_format = VK_FORMAT_UNDEFINED,
    .enable_depth_write = 0,
    .depth_test_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
    .min_depth_bounds = 0.0f,
    .max_depth_bounds = 1.0f,
};

typedef struct
{
    VkConservativeRasterizationModeEXT mode;
    float size;
} daxa_ConservativeRasterInfo;
_DAXA_DECL_OPTIONAL(daxa_ConservativeRasterInfo)

typedef struct
{
    VkPrimitiveTopology primitive_topology;
    daxa_Bool8 primitive_restart_enable;
    VkPolygonMode polygon_mode;
    VkCullModeFlags face_culling;
    VkFrontFace front_face_winding;
    daxa_Bool8 depth_clamp_enable;
    daxa_Bool8 rasterizer_discard_enable;
    daxa_Bool8 depth_bias_enable;
    float depth_bias_constant_factor;
    float depth_bias_clamp;
    float depth_bias_slope_factor;
    float line_width;
    daxa_u32 samples;
    daxa_Optional(daxa_ConservativeRasterInfo) conservative_raster_info;
} daxa_RasterizerInfo;

static const daxa_RasterizerInfo DAXA_DEFAULT_RASTERIZATION_INFO = {
    .primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitive_restart_enable = 0,
    .polygon_mode = VK_POLYGON_MODE_FILL,
    .face_culling = VK_CULL_MODE_NONE,
    .front_face_winding = VK_FRONT_FACE_CLOCKWISE,
    .depth_clamp_enable = 0,
    .rasterizer_discard_enable = 0,
    .depth_bias_enable = 0,
    .depth_bias_constant_factor = 0.0f,
    .depth_bias_clamp = 0.0f,
    .depth_bias_slope_factor = 0.0f,
    .line_width = 1.0f,
    .samples = 1,
    .conservative_raster_info = {.has_value = 0},
};

// should be moved in c++ from types to pipeline.hpp.
typedef struct
{
    VkBlendFactor src_color_blend_factor;
    VkBlendFactor dst_color_blend_factor;
    VkBlendOp color_blend_op;
    VkBlendFactor src_alpha_blend_factor;
    VkBlendFactor dst_alpha_blend_factor;
    VkBlendOp alpha_blend_op;
    VkColorComponentFlags color_write_mask;
} daxa_BlendInfo;
_DAXA_DECL_OPTIONAL(daxa_BlendInfo)

static const daxa_BlendInfo DAXA_DEFAULT_BLEND_INFO = {
    .src_color_blend_factor = VK_BLEND_FACTOR_ONE,
    .dst_color_blend_factor = VK_BLEND_FACTOR_ZERO,
    .color_blend_op = VK_BLEND_OP_ADD,
    .src_alpha_blend_factor = VK_BLEND_FACTOR_ONE,
    .dst_alpha_blend_factor = VK_BLEND_FACTOR_ZERO,
    .alpha_blend_op = VK_BLEND_OP_ADD,
    .color_write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
};

typedef struct
{
    VkFormat format;
    daxa_Optional(daxa_BlendInfo) blend;
} daxa_RenderAttachment;
_DAXA_DECL_FIXED_LIST(daxa_RenderAttachment, 8)

typedef struct
{
    uint32_t control_points;
    VkTessellationDomainOrigin origin;
} daxa_TesselationInfo;
_DAXA_DECL_OPTIONAL(daxa_TesselationInfo)

typedef struct
{
    daxa_Optional(daxa_ShaderInfo) mesh_shader_info;
    daxa_Optional(daxa_ShaderInfo) vertex_shader_info;
    daxa_Optional(daxa_ShaderInfo) tesselation_control_shader_info;
    daxa_Optional(daxa_ShaderInfo) tesselation_evaluation_shader_info;
    daxa_Optional(daxa_ShaderInfo) fragment_shader_info;
    daxa_Optional(daxa_ShaderInfo) task_shader_info;
    daxa_FixedList(daxa_RenderAttachment, 8) color_attachments;
    daxa_Optional(daxa_DepthTestInfo) depth_test;
    daxa_Optional(daxa_TesselationInfo) tesselation;
    daxa_RasterizerInfo raster;
    uint32_t push_constant_size;
    daxa_SmallString name;
} daxa_RasterPipelineInfo;

DAXA_EXPORT daxa_RasterPipelineInfo const *
daxa_raster_pipeline_info(daxa_RasterPipeline raster_pipeline);

DAXA_EXPORT uint64_t
daxa_raster_pipeline_inc_refcnt(daxa_RasterPipeline pipeline);
DAXA_EXPORT uint64_t
daxa_raster_pipeline_dec_refcnt(daxa_RasterPipeline pipeline);

#endif // #ifndef __DAXA_PIPELINE_H__
