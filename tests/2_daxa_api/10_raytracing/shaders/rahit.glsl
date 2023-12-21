#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#include <daxa/daxa.inl>

#include "shared.inl"
#include "random.glsl"

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)


layout(location = 0) rayPayloadInEXT hitPayload prd;

// hardcoded dissolve
const float dissolve = 0.3;

void main()
{

  uint seed = prd.seed;  // We don't want to modify the PRD
  if(rnd(seed) > dissolve)
    ignoreIntersectionEXT;
}
