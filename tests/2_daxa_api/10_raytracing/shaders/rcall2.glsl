#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#include <daxa/daxa.inl>

#include "shared.inl"

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)

layout(location = 3) callableDataInEXT rayLight cLight;

// Harcoded light direction
vec3 lightDirection = vec3(0.0, 1.0, 0.0);

void main()
{
    cLight.outLightDistance = 1000000;
    cLight.outIntensity     = 3.0;
    cLight.outLightDir      = normalize(lightDirection);
}
