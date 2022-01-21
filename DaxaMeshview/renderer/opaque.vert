#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(location = 10) out vec2 vtf_uv;
layout(location = 13) out vec3 vtf_world_space_normal;
layout(location = 14) out vec3 vtf_world_space_position;

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
    uint modelIndex;
} pushConstants;

void main() {
    vtf_uv = uv;
    mat4 m = primitivesBuffer.primitiveInfos[pushConstants.modelIndex].transform;
    mat4 itm = primitivesBuffer.primitiveInfos[pushConstants.modelIndex].inverseTransposeTransform;

    vtf_world_space_normal = (itm * vec4(normal,0.0f)).xyz;      // mul inverse transpose model matrix with the normal vector
    vtf_world_space_normal = normalize(vtf_world_space_normal);         // as the scaling can change the vector length, we re-normalize the vector's length

    vtf_world_space_position = (m * vec4(position, 1.0f)).xyz;

    mat4 mvp = globalBuffer.vp * m;
    gl_Position = mvp * vec4(position, 1.0f);
}