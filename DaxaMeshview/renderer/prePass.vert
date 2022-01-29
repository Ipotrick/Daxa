#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_shader_16bit_storage : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 position;

layout(set = 0, binding = 0) readonly uniform GlobalBuffer {
    mat4 vp;
    mat4 view;
    mat4 itvp;
    mat4 itview;
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

layout(std140, push_constant) uniform PushConstants {
    uint primitiveIndex;
} pushConstants;

void main() {
    mat4 m = primitivesBuffer.primitiveInfos[pushConstants.primitiveIndex].transform;
    mat4 mvp = globalBuffer.vp * m;
    gl_Position =  mvp * vec4(position, 1.0f);
}