#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#include "common.glsl"

layout(push_constant) uniform Push{
    uint globalBufferId;
} push;

layout(location = 0) out vec3 vertexPos;

// unit cube (faces have winding order for outside faces, dont cull backfaces)
vec3 positions[] = {
    vec3(-0.5f, -0.5f, -0.5f),
    vec3(-0.5f,  0.5f, -0.5f),
    vec3( 0.5f, -0.5f, -0.5f),

    vec3(-0.5f,  0.5f, -0.5f),
    vec3( 0.5f,  0.5f, -0.5f),
    vec3( 0.5f, -0.5f, -0.5f),

    vec3(-0.5f, -0.5f,  0.5f),
    vec3( 0.5f, -0.5f,  0.5f),
    vec3(-0.5f,  0.5f,  0.5f),

    vec3( 0.5f, -0.5f,  0.5f),
    vec3( 0.5f,  0.5f,  0.5f),
    vec3(-0.5f,  0.5f,  0.5f),

    vec3(-0.5f, -0.5f, -0.5f),
    vec3( 0.5f, -0.5f, -0.5f),
    vec3(-0.5f, -0.5f,  0.5f),

    vec3( 0.5f, -0.5f, -0.5f),
    vec3( 0.5f, -0.5f,  0.5f),
    vec3(-0.5f, -0.5f,  0.5f),

    vec3(-0.5f,  0.5f, -0.5f),
    vec3(-0.5f,  0.5f,  0.5f),
    vec3( 0.5f,  0.5f, -0.5f),

    vec3(-0.5f,  0.5f,  0.5f),
    vec3( 0.5f,  0.5f,  0.5f),
    vec3( 0.5f,  0.5f, -0.5f),

    vec3(-0.5f, -0.5f, -0.5f),
    vec3(-0.5f, -0.5f,  0.5f),
    vec3(-0.5f,  0.5f, -0.5f),

    vec3(-0.5f, -0.5f,  0.5f),
    vec3(-0.5f,  0.5f,  0.5f),
    vec3(-0.5f,  0.5f, -0.5f),

    vec3( 0.5f, -0.5f, -0.5f),
    vec3( 0.5f,  0.5f, -0.5f),
    vec3( 0.5f, -0.5f,  0.5f),

    vec3( 0.5f,  0.5f, -0.5f),
    vec3( 0.5f,  0.5f,  0.5f),
    vec3( 0.5f, -0.5f,  0.5f)
};

void main() {
    GlobalData globals = globalDataBufferView[push.globalBufferId].globalData;

    vec4 position = globals.view * vec4(positions[gl_VertexIndex],0);
    vertexPos = position.xyz;
    position.w = 1;
    gl_Position = position;
}