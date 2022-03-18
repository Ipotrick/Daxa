#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_shader_16bit_storage : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 10) out vec2 vtf_uv;
layout(location = 13) out vec3 vtf_world_space_normal;
layout(location = 14) out vec3 vtf_world_space_position;

struct GlobalData {
    mat4 vp;
    mat4 view;
    mat4 itvp;
    mat4 itview;
};
layout(std430, set = 0, binding = 4) buffer GlobalDataBufferView{ GlobalData globalData; } globalDataBufferView[];
struct PrimitiveInfo {
    mat4 transform;
    mat4 inverseTransposeTransform;
};
layout(std430, set = 0, binding = 4) buffer PrimitiveInfosBufferView{ PrimitiveInfo primitiveInfos[]; } primitiveInfoBufferView[]; 
struct PackedVec3 {
    float x;
    float y;
    float z;
};
layout(std430, set = 0, binding = 4) buffer PackedVec3BufferView{ PackedVec3 packedVec3s[]; } packedVec3BufferView[]; 
layout(std430, set = 0, binding = 4) buffer Vec2BufferView{ vec2 vecs[]; } vec2BufferView[]; 

layout(std140, push_constant) uniform PushConstants {
    uint albedoMap;
    uint normalMap;
    uint globals;
    uint primitives;
    uint lights;
    uint vertexPosBufferId;
    uint vertexUVBufferId;
    uint vertexNormalBufferId;
    uint modelIndex;
} pushConstants;

void main() {
    mat4 m = primitiveInfoBufferView[uint(pushConstants.primitives)].primitiveInfos[pushConstants.modelIndex].transform;
    mat4 itm = primitiveInfoBufferView[uint(pushConstants.primitives)].primitiveInfos[pushConstants.modelIndex].inverseTransposeTransform;

    PackedVec3 packedPos = packedVec3BufferView[uint(pushConstants.vertexPosBufferId)].packedVec3s[gl_VertexIndex];
    vec3 pulledPosition = vec3(packedPos.x,packedPos.y,packedPos.z);

    vec2 pulledUV = vec2BufferView[uint(pushConstants.vertexUVBufferId)].vecs[gl_VertexIndex];

    PackedVec3 packedNormal = packedVec3BufferView[uint(pushConstants.vertexNormalBufferId)].packedVec3s[gl_VertexIndex];
    vec3 pulledNormal = vec3(packedNormal.x,packedNormal.y,packedNormal.z);

    vtf_world_space_normal = (itm * vec4(pulledNormal,0.0f)).xyz;      // mul inverse transpose model matrix with the normal vector
    vtf_world_space_normal = normalize(vtf_world_space_normal);         // as the scaling can change the vector length, we re-normalize the vector's length

    vtf_world_space_position = (m * vec4(pulledPosition, 1.0f)).xyz;

    mat4 mvp = globalDataBufferView[uint(pushConstants.globals)].globalData.vp * m;
    vtf_uv = pulledUV;
    gl_Position = mvp * vec4(pulledPosition, 1.0f);

    gl_Position = vec4(1.0,1.0,1.0,1.0);
}