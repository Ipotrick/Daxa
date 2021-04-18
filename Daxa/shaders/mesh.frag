#version 450

#extension GL_KHR_vulkan_glsl: enable

layout (location = 0) in vec3 v_color;

//output write
layout (location = 0) out vec4 outFragColor;

const vec3 color = vec3(1,0,1);
const vec3 lightDir = vec3(-0.66,-0.66,0);
void main() 
{
	outFragColor = vec4(color * dot(-lightDir,v_color) * 0.9f + 0.1f * vec3(0.3,0.3,0),1);
}
