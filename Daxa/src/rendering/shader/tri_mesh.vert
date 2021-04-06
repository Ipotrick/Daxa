#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;

layout (location = 0) out vec3 v_color;

layout(push_constant) uniform constants
{
	mat4 render_matrix;
	vec4 data;
} PushConstants;

void main() 
{	
	gl_Position = PushConstants.render_matrix * vec4(vPosition, 1.0f);
	v_color = vColor;
}
