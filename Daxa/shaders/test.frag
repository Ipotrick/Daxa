#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 10) in vec4 v_color;

layout (location = 0) out vec4 outFragColor;

layout (push_constant) uniform constant{
	float triAlpha;
} pushConstants;

void main()
{
	vec4 color = v_color;
	color.a = pushConstants.triAlpha;
	outFragColor = color;
}