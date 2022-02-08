#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_shader_16bit_storage : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 10) in vec2 vtf_uv;
layout(location = 13) in vec3 vtf_world_space_normal;
layout(location = 14) in vec3 vtf_world_space_position;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec3 outNormal;

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
layout(std430, set = 0, binding = 4) buffer PrimitiveDatasBufferView{ PrimitiveInfo primitiveInfos[]; } primitiveDataBufferView[]; 
struct Light {
    vec3 position;
    float strength;
    vec4 color;
};
layout(std430, set = 0, binding = 4) buffer LightsBufferView{
    uint lightCount;
    Light lights[];
} lightsBufferView[];
layout(std140, set = 0, binding = 4) buffer OrthLightBufferView{
    vec3 direction;
    uint16_t shadowMap;
};
layout(set = 0, binding = 1) uniform sampler2D imageSampler2DViews[];

layout(std140, push_constant) uniform PushConstants {
    uint16_t albedoMap;
    uint16_t normalMap;
    uint16_t globals;
    uint16_t primitives;
    uint16_t lights;
    uint16_t vertexPosBufferId;
    uint16_t vertexUVBufferId;
    uint16_t vertexNormalBufferId;
    uint modelIndex;
} pushConstants;

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

void main() {
    vec3 interpVertexNormal = normalize(vtf_world_space_normal);
    vec4 ambient = vec4(0.01,0.01,0.02,1.0);

    vec3 normalMapValue = (texture(imageSampler2DViews[uint(pushConstants.normalMap)], vtf_uv).xyz * 2.0f) - vec3(1.0f);
    vec3 normal = genTBN(interpVertexNormal, vtf_world_space_position, vtf_uv) * normalMapValue;

    vec4 lightAcc = ambient;
    for (int i = 0; i < lightsBufferView[uint(pushConstants.lights)].lightCount; i++) {
        Light light = lightsBufferView[uint(pushConstants.lights)].lights[i];
        float dist = length(vtf_world_space_position - light.position);
        vec3 direction = vtf_world_space_position - light.position;

        float strength = 1.0f / ((dist + 1.0000001f) * (dist + 1.0000001f)) * light.strength;
        strength = strength * max(dot(normal, -direction), 0.0f);
        lightAcc += light.color * strength;
    }
    outFragColor = texture(imageSampler2DViews[uint(pushConstants.albedoMap)], vtf_uv) * lightAcc;

    outNormal = normal * 0.5f + vec3(0.5f);
}