#version 450

#include <shared.inl>

DAXA_PUSH_CONSTANT(DrawPushConstant)

layout(location = 0) out vec4 color;

void main()
{
    color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}