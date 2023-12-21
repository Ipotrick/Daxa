#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#include <daxa/daxa.inl>
#include "shared.inl"

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main()
{
  vec3 color = vec3(1.0, 0.5, 0.0);
  prd.hitValue = color;
}
