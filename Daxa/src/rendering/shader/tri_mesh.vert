#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;

layout (location = 0) out vec3 v_color;

layout(set = 0, binding = 0) uniform Data {
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
	gl_Position = data.proj * data.view * data.model * vec4(vPosition, 1.0f);
	v_color = vNormal;
}
