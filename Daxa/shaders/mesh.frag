#version 450

layout (location = 0) in vec3 v_color;

//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform texture2D textures[4096];
layout(set = 0, binding = 1) uniform sampler s;

const vec3 color = vec3(1,0,1);
const vec3 lightDir = vec3(-0.66,-0.66,0);
void main() 
{
	vec4 sampledColor = texture(sampler2D(textures[0], s), vec2(0,0));
	outFragColor = vec4(sampledColor.rgb * dot(-lightDir,v_color) * 0.9f + 0.1f * vec3(0.3,0.3,0),1);
}
