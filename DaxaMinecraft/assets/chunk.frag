#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 10) in vec3 v_pos;
layout(location = 11) in vec3 v_nrm;
layout(location = 12) in vec2 v_tex;

layout (location = 0) out vec4 o_col;
layout(set = 0, binding = 1) uniform sampler2D tex;

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
    // const int MAX_ITER = 1000;
    // vec2 c = v_pos.xz / 50;
    // vec2 z = c;
    // int i = 0;
    // for (; i < MAX_ITER; ++i) {
    //     float real = z.x * z.x - z.y * z.y;
    //     float imag = 2.0f * z.x * z.y;
    //     z = vec2(real, imag) + c;
    //     if (z.x * z.x + z.y * z.y > 2.0) break;
    // }
    // if (i == MAX_ITER) i = 0;
    
    vec4 tex_col = texture(tex, v_tex);
    if (tex_col.a < 0.1f) discard;
    vec3 albedo = tex_col.rgb;
    // vec3 albedo = vec3(vec2(i)/vec2(100), 1);
    vec3 diffuse = vec3(0.5);
    diffuse += vec3(0, 1, 20) * calc_light_point(vec3(3, 12, 1)) * 100;
    diffuse += vec3(1, 0.8, 0.6) * calc_light_directional(vec3(2, 3, 1)) * 0.1;
    o_col = vec4(diffuse * albedo, 1);
}
