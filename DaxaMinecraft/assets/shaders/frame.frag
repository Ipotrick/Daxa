#version 440 core
in vec2 v_tex;
uniform sampler2D g_pos_tex;
uniform sampler2D g_nrm_tex;
uniform sampler2D g_col_tex;
uniform vec2 frame_size;
out vec4 o_col;
void main() {
    vec3 pos = texture(g_pos_tex, v_tex).rgb;
    vec3 nrm = texture(g_nrm_tex, v_tex).rgb;
    vec3 col = texture(g_col_tex, v_tex).rgb;
    vec2 px_offset = vec2(1.0) / vec2(1024, 720);

    vec3 light_dir = normalize(vec3(1.0, -2.0, 0.5));
    float ambient = 0.2;
    float diffuse = max(dot(nrm, -light_dir) * 0.5 + 0.5, 0.0) + ambient;

    // col = 1.0 - col;
    // o_col = vec4(clamp(pos, 0, 1) * diffuse, 1);
    
    o_col = vec4(col * diffuse, 1);
}
