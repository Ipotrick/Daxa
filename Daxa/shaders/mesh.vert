#version 450

#extension GL_KHR_vulkan_glsl: enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;

layout (location = 0) out vec3 v_color;

layout(set = 1, binding = 0) uniform Data {
	mat4 model;
	mat4 view;
	mat4 proj;
} data;

layout(push_constant) uniform PushConstants
{
	mat4 render_matrix;
	vec4 data;
} constants;

void main() 
{	
	mat4 mvp = data.proj * data.view * data.model;
	gl_Position = mvp * vec4(vPosition, 1.0f);
	v_color = (transpose(inverse(data.model)) * vec4(vColor,0)).xyz;
}
