#version 450
#extension GL_EXT_nonuniform_qualifier : enable

void main() {
    vec4 positions[3] = {
        vec4(-1,-3,0.5,1),
        vec4(-1, 1,0.5,1),
        vec4( 3, 1,0.5,1),
    };

    gl_Position = positions[gl_VertexIndex];
}