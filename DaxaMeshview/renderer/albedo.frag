
#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 10) in vec2 vtf_uv;
layout(location = 11) in vec3 vtf_normal;

layout (location = 0) out vec4 outFragColor;

layout(set = 1, binding = 0) uniform sampler2D albedo;

void main()
{
    vec4 color = vec4(vtf_normal, 1.0f);//texture(albedo, vtf_uv);
    outFragColor = color;
}