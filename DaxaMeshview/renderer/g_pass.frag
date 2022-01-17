#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 10) in vec2 vtf_uv;
layout(location = 11) in vec3 vtf_world_space_normal;
layout(location = 12) in vec2 vtl_screen_space_normal;

layout (location = 0) out vec2 out_screen_space_normals;

layout(set = 0, binding = 0) readonly uniform GlobalBuffer {
    mat4 vp;
    mat4 view;
} globalBuffer;
layout(std140, set = 0, binding = 1) readonly buffer TransformBuffer {
    mat4 transforms[];
} transformBuffer;
struct Light {
    vec3 position;
    float strength;
    vec4 color;
};
layout(std140, set = 0, binding = 2) readonly buffer LightBuffer {
    uint lightCount;
    Light lights[];
} lightBuffer;

void main()
{
    out_screen_space_normals = vtl_screen_space_normal;
}