#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#include "common.glsl"

layout(location = 0) in vec3 vertexPos;

layout(set = 0, binding = 0) uniform sampler samplers[];
layout(set = 0, binding = 2) uniform textureCube cubeTextures[];

layout(push_constant) uniform Push{
    uint globalBufferId;
} push;

layout(location = 0) out vec4 o_color;

void main() {
    GlobalData globals = globalDataBufferView[push.globalBufferId].globalData;
    vec4 tex = texture(
        samplerCube(
            cubeTextures[globals.skyboxImageId], 
            samplers[globals.generalSamplerId]
        ), 
        normalize(vertexPos)
    );
    o_color = max(vec4(0),tex - vec4(vec3(0),0));
}