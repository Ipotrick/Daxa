#ifndef __DAXA_PIPELINE_H__
#define __DAXA_PIPELINE_H__

#include <daxa/c/types.h>

typedef struct
{
    uint32_t const * byte_code;
    size_t byte_code_size;
    daxa_StringView entry_point;
} daxa_ShaderInfo;
_DAXA_DECL_OPTIONAL(daxa_ShaderInfo)

typedef struct
{
    daxa_ShaderInfo shader_info;
    uint32_t push_constant_size;
    daxa_StringView name;
} daxa_ComputePipelineInfo;

typedef struct daxa_ImplComputePipeline * daxa_ComputePipeline;

DAXA_EXPORT daxa_ComputePipelineInfo const *
daxa_compute_pipeline_info(daxa_ComputePipeline compute_pipeline);

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
    daxa_StringView name;
} daxa_RasterPipelineInfo;

typedef struct daxa_ImplRasterPipeline * daxa_RasterPipeline;

DAXA_EXPORT daxa_RasterPipelineInfo const *
daxa_raster_pipeline_info(daxa_RasterPipeline raster_pipeline);

#endif // #ifndef __DAXA_PIPELINE_H__
