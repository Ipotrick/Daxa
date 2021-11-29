#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 10) in vec4 v_color;

//output write
layout (location = 0) out vec4 outFragColor;

void main()
{
	//return red
	outFragColor = v_color;
}