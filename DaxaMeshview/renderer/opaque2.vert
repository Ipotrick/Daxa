#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_shader_16bit_storage : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 10) out vec2 vtf_uv;
layout(location = 13) out vec3 vtf_world_space_normal;
layout(location = 14) out vec3 vtf_world_space_position;

#include "common.glsl"

struct PackedVec3 {
    float x;
    float y;
    float z;
};
layout(std430, set = 0, binding = 4) buffer PackedVec3BufferView{ PackedVec3 packedVec3s[]; } packedVec3BufferView[]; 
layout(std430, set = 0, binding = 4) buffer Vec2BufferView{ vec2 vecs[]; } vec2BufferView[]; 

layout(std140, push_constant) uniform PushConstants {
    uint globals;
    uint primitives;
    uint lights;
    uint modelIndex;
} push;

void main() {
    PrimitiveInfo prim = primitiveDataBufferView[push.primitives].primitiveInfos[push.modelIndex];

    PackedVec3 packedPos = packedVec3BufferView[uint(prim.vertexPositionsId)].packedVec3s[gl_VertexIndex];
    vec3 pulledPosition = vec3(packedPos.x,packedPos.y,packedPos.z);

    vec2 pulledUV = vec2BufferView[uint(prim.vertexUVsId)].vecs[gl_VertexIndex];

    PackedVec3 packedNormal = packedVec3BufferView[uint(prim.vertexNormalsId)].packedVec3s[gl_VertexIndex];
    vec3 pulledNormal = vec3(packedNormal.x,packedNormal.y,packedNormal.z);

    vtf_world_space_normal = (prim.ttTransform * vec4(pulledNormal,0.0f)).xyz;      // mul inverse transpose model matrix with the normal vector
    vtf_world_space_normal = normalize(vtf_world_space_normal);         // as the scaling can change the vector length, we re-normalize the vector's length

    vtf_world_space_position = (prim.transform * vec4(pulledPosition, 1.0f)).xyz;

    mat4 mvp = globalDataBufferView[uint(push.globals)].globalData.vp * prim.transform;
    vtf_uv = pulledUV;
    gl_Position = mvp * vec4(pulledPosition, 1.0f);
}