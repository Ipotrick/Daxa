#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(push_constant) uniform Push{
    uint hdrImgId;
    uint width;
    uint height;
} push;

layout(set = 0, binding = 1) uniform sampler2D images[];

layout(location = 0) out vec4 o_color;

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;
vec3 uncharted2Tonemap(vec3 x)
{
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main() {

    vec2 coord = vec2(
        gl_FragCoord.x / (push.width-1),
        gl_FragCoord.y / (push.height-1)
    );
    vec3 color = texture(images[push.hdrImgId],coord).rgb;
    vec3 mappedColor = uncharted2Tonemap(color);
    o_color = vec4(mappedColor,1); 
}