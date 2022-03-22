#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_texture_array : enable

#include "utils/shading.glsl"

layout(location = 10) in vec3 v_pos;
layout(location = 11) in vec3 v_nrm;
layout(location = 12) in vec3 v_tex;

layout(location = 0) out vec4 o_col;
layout(set = 0, binding = 1) uniform sampler2DArray tex;

void main() {
    vec4 tex_col = texture(tex, v_tex);
    if (tex_col.a < 0.4f)
        discard;
    vec3 albedo = tex_col.rgb;
    vec3 diffuse = vec3(0.0);
    diffuse += vec3(0.3, 0.4, 1.0) * (calc_light_directional(v_nrm, vec3(0, 1, 0)) + 2.0) * 0.5;
    diffuse += vec3(1, 0.8, 0.6) * calc_light_directional(v_nrm, vec3(-2, 3, 1)) * 1.0;
    o_col = vec4(diffuse * albedo, 1);
}
