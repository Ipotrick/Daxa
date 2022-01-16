#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 10) in vec2 vtf_uv;

layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform GlobalBuffer {
    mat4 vp;
    mat4 view;
} globalBuffer;
layout(std140, set = 0, binding = 1) buffer TransformBuffer {
    mat4 transforms[];
} transformBuffer;
struct Light {
    vec3 position;
    float strength;
    vec4 color;
};
layout(std140, set = 0, binding = 2) buffer LightBuffer {
    uint lightCount;
    vec3 _pad0;
    Light lights[];
} lightBuffer;

layout(set = 1, binding = 0) uniform sampler2D albedo;

void main()
{
    vec4 color = texture(albedo, vtf_uv);
    outFragColor = color;
}