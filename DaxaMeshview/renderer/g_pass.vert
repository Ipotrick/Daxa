
#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(location = 10) out vec2 vtf_uv;
layout(location = 11) out vec3 vtf_world_space_normal;
layout(location = 12) out vec2 vtl_screen_space_normal;

layout(set = 0, binding = 0) uniform Globals {
    mat4 vp;
    mat4 view;
} globals;

layout(std140, set = 0, binding = 1) buffer ModelData {
    mat4 transforms[];
} modelData;

layout(push_constant) uniform PushConstants {
    uint modelIndex;
} pushConstants;

void main()
{
    vtf_uv = uv;
    mat4 m = modelData.transforms[pushConstants.modelIndex];

    mat4 itm = inverse(transpose(m));               // get inverse transposed model matrix
    vtf_world_space_normal = (itm * vec4(normal,0.0f)).xyz;     // mul inverse transpose model matrix with the normal vector
    vtf_world_space_normal = normalize(vtf_world_space_normal);               // as the scaling can change the vector length, we re-normalize the vector's length

    vtl_screen_space_normal = (globals.view * vec4(vtf_world_space_normal,0.0f)).xy;

    mat4 mvp = globals.vp * m;
    gl_Position =  mvp * vec4(position, 1.0f);
}