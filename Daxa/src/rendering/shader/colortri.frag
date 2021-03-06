#version 450

#extension GL_KHR_vulkan_glsl: enable

layout (location = 0) in vec3 v_color;

//output write
layout (location = 0) out vec4 outFragColor;

const vec3 color = vec3(1,1,1);
const vec3 lightDir = vec3(0,-1,0);

void main() 
{
	//return red
	outFragColor = vec4(color * min(0.0f, dot(-lightDir,v_color)),1.0f);
}
