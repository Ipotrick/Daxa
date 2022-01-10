
#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(location = 10) out vec2 vtf_uv;

layout(set = 0, binding = 0) uniform Globals {
    mat4 vp;
} globals;

layout(std140, set = 0, binding = 1) buffer ModelData {
    mat4 transforms[];
} modelData;

layout(push_constant) uniform PushConstants {
    uint modelIndex;
} pushConstants;

void main()
{
    vtf_uv = uv;
    gl_Position = globals.vp * modelData.transforms[pushConstants.modelIndex] * vec4(position, 1.0f);
}