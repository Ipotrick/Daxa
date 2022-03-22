#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_texture_array : enable

layout(location = 10) in vec3 v_pos;
layout(location = 11) in vec3 v_nrm;
layout(location = 12) in vec3 v_tex;

layout(location = 0) out vec4 o_col;
layout(set = 0, binding = 1) uniform sampler2DArray tex;

float calc_light_point(vec3 light_pos) {
    vec3 light_del = light_pos - v_pos;
    float distsq = dot(light_del, light_del);
    vec3 light_del_dir = normalize(light_del);
    vec3 nrm = normalize(v_nrm);
    float dotnrm = dot(nrm, light_del_dir);
    return max(0, dotnrm / distsq);
}
float calc_light_directional(vec3 light_dir) {
    vec3 nrm = normalize(v_nrm);
    float dotnrm = dot(nrm, light_dir);
    return max(0, dotnrm);
}

void main() {
    vec4 tex_col = texture(tex, v_tex);
    if (tex_col.a < 0.4f)
        discard;
    vec3 albedo = tex_col.rgb;
    vec3 diffuse = vec3(0.0);
    diffuse += vec3(0.3, 0.4, 1.0) * (calc_light_directional(vec3(0, 1, 0)) + 2.0) * 0.5;
    diffuse += vec3(1, 0.8, 0.6) * calc_light_directional(vec3(-2, 3, 1)) * 1.0;
    o_col = vec4(diffuse * albedo, 1);
}
