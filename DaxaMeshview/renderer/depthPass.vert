#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_shader_16bit_storage : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(std140, set = 0, binding = 0) uniform CameraInfoBufferView{ mat4 vp; } cameraInfoBufferView;
struct PrimitiveInfo {
    mat4 transform;
    mat4 inverseTransposeTransform;
};
layout(std140, set = 0, binding = 1) readonly buffer PrimitivesBufferView { PrimitiveInfo primitiveInfos[]; } primitivesBufferView[];
struct PackedVec3{
    float x;
    float y;
    float z;
};
layout(std140, set = 1, binding = 4) readonly buffer PackedVec3BufferView{ PackedVec3 packedVec3s[]; } packedVec3BufferView[]; 

layout(std140, push_constant) uniform PushConstants {
    uint vertexPositionBufferId;
    uint primitiveIndex;
} pushConstants;

void main() {
    PackedVec3 packedPosition = packedVec3BufferView[uint(pushConstants.vertexPositionBufferId)].packedVec3s[gl_VertexIndex];
    vec3 position = vec3(packedPosition.x, packedPosition.y, packedPosition.z);
    mat4 m = primitivesBufferView[uint(pushConstants.vertexPositionBufferId)].primitiveInfos[pushConstants.primitiveIndex].transform;

    mat4 mvp = cameraInfoBufferView.vp * m;
    gl_Position =  mvp * vec4(position, 1.0f);
}