#version 440 core
in vec3 v_pos;
in vec3 v_nrm;
in vec2 v_tex;
uniform sampler2D t0;
layout (location = 0) out vec4 g_pos;
layout (location = 1) out vec4 g_nrm;
layout (location = 2) out vec4 g_col;
void main() {
    vec4 tex_col = texture(t0, v_tex);
    if (tex_col.a < 0.1) discard;
    vec3 albedo = tex_col.rgb;

    g_pos = vec4(v_pos, 0);
    g_nrm = vec4(normalize(v_nrm), 0);
    g_col = vec4(albedo, 1.0);
}
