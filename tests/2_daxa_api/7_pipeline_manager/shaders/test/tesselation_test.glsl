#define DAXA_ENABLE_SHADER_NO_NAMESPACE 1
#include "daxa/daxa.inl"

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_VERTEX
void main()
{
    gl_Position = f32vec4(1.0, 1.0, 1.0, 1.0);
}
#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_TESSELATION_CONTROL
layout (vertices = 3) out;

void main(void)
{
    if (gl_InvocationID == 0)
    {
        gl_TessLevelInner[0] = 1.0;
        gl_TessLevelOuter[0] = 1.0;
        gl_TessLevelOuter[1] = 1.0;
        gl_TessLevelOuter[2] = 1.0;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_TESSELATION_EVALUATION
layout (triangles, fractional_odd_spacing, cw) in;

void main()
{
    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +
                  (gl_TessCoord.y * gl_in[1].gl_Position) +
                  (gl_TessCoord.z * gl_in[2].gl_Position);
}
#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_FRAGMENT
layout (location = 0) out f32vec4 outFragColor;
void main()
{
    outFragColor = f32vec4(1.0, 0.0, 0.0, 1.0);
}
#endif
