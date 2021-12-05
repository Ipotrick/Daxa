#version 440 core
layout(location=0) in vec3 a_pos;
layout(location=1) in vec3 a_nrm;
layout(location=2) in vec2 a_tex;
out vec3 v_pos;
out vec3 v_nrm;
out vec2 v_tex;
uniform mat4 mvp_mat;
void main() {
    v_nrm = a_nrm;
    v_tex = a_tex;
    gl_Position = mvp_mat * vec4(a_pos, 1);
    v_pos = gl_Position.xyz;
}
