#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 10) in vec2 vtf_uv;
layout(location = 11) in vec3 vtf_normal;
layout(location = 12) in vec2 vtf_screen_space_normal;
layout(location = 13) in vec3 vtf_world_space_normal;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec2 outScreenSpaceNormal;

layout(set = 0, binding = 0) readonly uniform GlobalBuffer {
    mat4 vp;
    mat4 view;
} globalBuffer;
struct PrimitiveInfo {
    mat4 transform;
    mat4 inverseTransposeTransform;
};
layout(std140, set = 0, binding = 1) readonly buffer PrimitivesBuffer {
    PrimitiveInfo primitiveInfos[];
} primitivesBuffer;
struct Light {
    vec3 position;
    float strength;
    vec4 color;
};
layout(std140, set = 0, binding = 2) readonly buffer LightBuffer {
    uint lightCount;
    Light lights[];
} lightBuffer;

layout(set = 1, binding = 0) uniform sampler2D albedo;

void main() {
    vec4 ambient = vec4(0.01,0.01,0.01,1.0);

    vec3 lightDir = vec3(-0.22,-0.22,-0.22);
    vec3 iLightDir = -lightDir;
    float iStrength = max(dot(vtf_world_space_normal, iLightDir), 0.0f);

    vec4 color = texture(albedo, vtf_uv) * min(ambient + iStrength, 1.0f);
    outFragColor = color;

    outScreenSpaceNormal = vtf_screen_space_normal;
}