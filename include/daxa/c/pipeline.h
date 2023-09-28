#ifndef __DAXA_PIPELINE_H__
#define __DAXA_PIPELINE_H__

#include "types.h"

typedef struct
{
    uint32_t const * data;
    size_t size;
} daxa_ShaderByteCode;

typedef struct
{
    daxa_ShaderByteCode byte_code;
    char const * entry_point;
} daxa_ShaderInfo;

typedef struct
{
    daxa_ShaderInfo shader_info;
    uint32_t push_constant_size;
    char const * name;
} daxa_ComputePipelineInfo;

typedef struct daxa_ImplComputePipeline * daxa_ComputePipeline;

daxa_ComputePipelineInfo const *
daxa_compute_pipeline_info(daxa_ComputePipeline compute_pipeline);

typedef struct
{
    VkFormat depth_attachment_format;
    VkBool32 enable_depth_test;
    VkBool32 enable_depth_write;
    VkCompareOp depth_test_compare_op;
    float min_depth_bounds;
    float max_depth_bounds;
} daxa_DepthTestInfo;

static const daxa_DepthTestInfo DAXA_DEFAULT_DEPTH_TEST_INFO = {
    .depth_attachment_format = VK_FORMAT_UNDEFINED,
    .enable_depth_test = 0,
    .enable_depth_write = 0,
    .depth_test_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
    .min_depth_bounds = 0.0f,
    .max_depth_bounds = 1.0f,
};

typedef struct
{
    VkConservativeRasterizationMode mode;
    float size;
} daxa_ConservativeRasterInfo;

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
    daxa_Bool8 enable_conservative_rasterization;
    daxa_ConservativeRasterInfo conservative_raster_info;
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
    .enable_conservative_rasterization = 0,
    .conservative_raster_info,
};

// should be moved in c++ from types to pipeline.hpp.
typedef struct
{
    daxa_Bool8 blend_enable;
    VkBlendFactor src_color_blend_factor;
    VkBlendFactor dst_color_blend_factor;
    VkBlendOp color_blend_op;
    VkBlendFactor src_alpha_blend_factor;
    VkBlendFactor dst_alpha_blend_factor;
    VkBlendOp alpha_blend_op;
    VkColorComponentFlags color_write_mask
} daxa_BlendInfo;

static const daxa_BlendInfo DAXA_DEFAULT_BLEND_INFO = {
    .blend_enable = 0;
    .src_color_blend_factor = VK_BLEND_FACTOR_ONE;
    .dst_color_blend_factor = VK_BLEND_FACTOR_ZERO;
    .color_blend_op = VK_BLEND_OP_ADD;
    .src_alpha_blend_factor = VK_BLEND_FACTOR_ONE;
    .dst_alpha_blend_factor = VK_BLEND_FACTOR_ZERO;
    .alpha_blend_op = VK_BLEND_OP_ADD;
    .color_write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT  | VK_COLOR_COMPONENT_A_BIT;
};

typedef struct
{
    VkFormat format;
    daxa_BlendInfo blend;
} daxa_RenderAttachment;

typedef struct 
{
    uint32_t control_points;
    VkTesselationDomainOrigin origin;
} daxa_TesselationInfo;

// NODE(pahrens): This is an arbitrary number, bump it if you need more.
static const uint32_t DAXA_MAXIMUM_COLOR_ATTACHMENT_COUNT = 8;

struct RasterPipelineInfo
{
    VkShaderStageFlags enabled_stages;
    daxa_ShaderInfo mesh_shader_info;
    daxa_ShaderInfo vertex_shader_info;
    daxa_ShaderInfo tesselation_control_shader_info;
    daxa_ShaderInfo tesselation_evaluation_shader_info;
    daxa_ShaderInfo fragment_shader_info;
    daxa_ShaderInfo task_shader_info;
    daxa_RenderAttachment color_attachments[DAXA_MAXIMUM_COLOR_ATTACHMENT_COUNT];
    uint32_t color_attachment_count;
    daxa_DepthTestInfo depth_test = {};
    daxa_RasterizerInfo raster = {};
    daxa_TesselationInfo tesselation = {};
    uint32_t push_constant_size = {};
    char const * name = {};
};

struct daxa_ImplRasterPipeline * daxa_RasterPipeline;

daxa_RasterPipelineInfo const *
daxa_raster_pipeline_info(daxa_RasterPipeline raster_pipeline);

#endif // #ifndef __DAXA_PIPELINE_H__