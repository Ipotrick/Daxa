#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#include <daxa/daxa.inl>
#include "shared.inl"

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)

layout(location = 1) rayPayloadInEXT bool isShadowed;

void main()
{
  isShadowed = false;
}
