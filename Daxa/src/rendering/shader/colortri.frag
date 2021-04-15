#version 450

#extension GL_KHR_vulkan_glsl: enable

layout(set = 0, binding = 1) uniform SceneData {
	vec4 color;
} sceneData;

layout (location = 0) in vec3 v_color;

//output write
layout (location = 0) out vec4 outFragColor;

void main() 
{
	//return red
	outFragColor = vec4(v_color,1.0f);
}
