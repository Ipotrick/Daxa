#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(location = 10) out vec2 vtf_uv;

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


layout(push_constant) uniform PushConstants {
    uint modelIndex;
} pushConstants;

void main() {
    vtf_uv = uv;

    mat4 m = transformBuffer.transforms[pushConstants.modelIndex];
    mat4 mvp = globalBuffer.vp * m;
    gl_Position =  mvp * vec4(position, 1.0f);
}