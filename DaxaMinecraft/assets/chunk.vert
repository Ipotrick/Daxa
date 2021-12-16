#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location=0) in vec3 a_pos;
layout(location=1) in vec3 a_nrm;
layout(location=2) in vec2 a_tex;

layout(location = 10) out vec3 v_pos;
layout(location = 11) out vec3 v_nrm;
layout(location = 12) out vec2 v_tex;

layout(set = 0, binding = 0) uniform SomeBuffer {
    mat4 viewproj_mat;
} u;
layout(push_constant) uniform Push {
    mat4 modl_mat;
} p;

void main() {
    v_nrm = a_nrm;
    v_tex = a_tex;
    vec4 pos = p.modl_mat * vec4(a_pos, 1);
    gl_Position = u.viewproj_mat * pos;
    gl_Position.y *= -1;
    v_pos = pos.xyz;
}
