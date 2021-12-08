#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 10) in vec4 v_color;

layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform SomeBuffer {
	vec4 data;
} someBuffer;

void main()
{
	vec4 color = v_color;
	color *= someBuffer.data.b;
	outFragColor = color;
}