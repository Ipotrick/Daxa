
#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 10) in vec2 vtf_uv;
layout(location = 11) in vec3 vtf_world_space_normal;
layout(location = 12) in vec2 vtl_screen_space_normal;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec2 out_screen_space_normals;

layout(set = 0, binding = 0) uniform Globals {
    mat4 vp;
    mat4 view;
} globals;

layout(set = 1, binding = 0) uniform sampler2D albedo;

void main()
{
    out_screen_space_normals = vtl_screen_space_normal;
    // example of how to reconstruct the world space normals from the screen space normals
    mat4 iview = inverse(globals.view);
    float screen_space_normal_z_component = sqrt(1 - vtl_screen_space_normal.x * vtl_screen_space_normal.x - vtl_screen_space_normal.y * vtl_screen_space_normal.y);
    vec3 screen_space_normal = vec3(vtl_screen_space_normal.xy, screen_space_normal_z_component);
    vec3 reconstructed_world_space_normal = (iview * vec4(screen_space_normal, 0.f)).xyz;
    
    vec4 color = texture(albedo, vtf_uv);
    //vec4 color = vec4(((vtf_world_space_normal + vec3(1,1,1)) * 0.5f),1);
    //vec4 color = vec4((vtl_screen_space_normal + vec2(1,1))*0.5f, 1, 1);
    outFragColor = color;
}