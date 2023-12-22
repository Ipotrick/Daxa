#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#include <daxa/daxa.inl>

#include "shared.inl"

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)

layout(location = 3) callableDataInEXT rayLight cLight;

// Hardcoded light position
vec3 lightPos = vec3(1.0, 2.0, 0.4);

// Hardcoded light intensity
float lightIntensity = 10.0;

void main()
{
    vec3 lDir               = lightPos - cLight.inHitPosition;
    cLight.outLightDistance = length(lDir);
    cLight.outIntensity     = lightIntensity / (cLight.outLightDistance * cLight.outLightDistance);
    cLight.outLightDir      = normalize(lDir);
}
