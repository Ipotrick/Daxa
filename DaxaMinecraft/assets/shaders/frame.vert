#version 440 core
layout (location = 0) in vec2 a_pos;
out vec2 v_tex;
void main() {
    v_tex = a_pos;
    gl_Position = vec4(a_pos * 2.0 - 1.0, 0.0, 1.0); 
}
