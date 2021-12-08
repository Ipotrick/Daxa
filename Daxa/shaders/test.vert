#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout(location = 10) out vec4 v_color;

void main()
{
	v_color = color;
	gl_Position = vec4(position, 1.0f);
}