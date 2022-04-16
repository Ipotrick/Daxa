#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_shader_16bit_storage : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "common.glsl"
layout(set = 0, binding = 0) uniform sampler samplers[];
layout(set = 0, binding = 2) uniform textureCube cubeTextures[];

layout(location = 10) in vec2 vtf_uv;
layout(location = 13) in vec3 vtf_world_space_normal;
layout(location = 14) in vec3 vtf_world_space_position;
layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec3 outNormal;

mat3 genTBN(vec3 n, vec3 p, vec2 uv) {
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 dp2perp = cross(dp2, n);
    vec3 dp1perp = cross(n, dp1);
    vec3 t = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 b = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = inversesqrt(max(dot(t, t), dot(b, b)));
    return mat3(t * invmax, b * invmax, n);
}

layout(std140, push_constant) uniform PushConstants {
    uint globalBufferId;
    uint primitives;
    uint lights;
    uint modelIndex;
} push;

void main() {
    GlobalData globals = globalDataBufferView[push.globalBufferId].globalData;
    vec3 interpVertexNormal = normalize(vtf_world_space_normal);
    vec4 ambient = vec4(1.0,1.0,1.00,1.0);
    PrimitiveInfo prim = primitiveDataBufferView[push.primitives].primitiveInfos[push.modelIndex];

    vec3 normalMapValue = (texture(imageSampler2DViews[prim.normalMapId], vtf_uv).xyz * 2.0f) - vec3(1.0f);
    vec3 normal = genTBN(interpVertexNormal, vtf_world_space_position, vtf_uv) * normalMapValue;

    vec4 lightAcc = ambient;
    for (int i = 0; i < lightsBufferView[uint(push.lights)].lightCount; i++) {
        Light light = lightsBufferView[uint(push.lights)].lights[i];
        float dist = length(vtf_world_space_position - light.position);
        vec3 direction = vtf_world_space_position - light.position;

        float strength = 1.0f / ((dist + 1.0000001f) * (dist + 1.0000001f)) * light.strength;
        strength = strength * max(dot(normal, -direction), 0.0f);
        lightAcc += light.color * strength;
    }

    vec3 world_space_direction = normalize(vtf_world_space_position - globals.cameraPosition.xyz);
    vec3 reflected = reflect(world_space_direction, normal);
    vec4 skyBoxSample = texture(
        samplerCube(
            cubeTextures[globals.skyboxImageId],
            samplers[globals.generalSamplerId]
        ),
        reflected
    );

    outFragColor = 
        max(vec4(0), (skyBoxSample +
        texture(imageSampler2DViews[uint(prim.albedoMapId)], vtf_uv) * lightAcc) * 0.5);

    outNormal = normal * 0.5f + vec3(0.5f);
}