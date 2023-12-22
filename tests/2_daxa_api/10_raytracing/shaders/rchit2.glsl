#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#include <daxa/daxa.inl>
#include "shared.inl"

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;
layout(location = 3) callableDataEXT rayLight cLight;

// Change this to 0 to see the difference between callables
#define DAXA_CALLABLE_INDEX 0
// #define DEBUG_NORMALS

void main()
{
  vec3 world_pos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

  cLight.inHitPosition = world_pos;


  if(DAXA_CALLABLE_INDEX < 2)
    executeCallableEXT(DAXA_CALLABLE_INDEX, 3);
  else {
    vec3 lDir               = vec3(1.0, 2.0, 0.4) - cLight.inHitPosition;
    cLight.outLightDistance = length(lDir);
    cLight.outIntensity     = 100.0 / (cLight.outLightDistance * cLight.outLightDistance);
    cLight.outLightDir      = normalize(lDir);
  }

  mat4 model = mat4(
      gl_ObjectToWorldEXT[0][0], gl_ObjectToWorldEXT[0][1], gl_ObjectToWorldEXT[0][2], 0,
      gl_ObjectToWorldEXT[0][1], gl_ObjectToWorldEXT[1][1], gl_ObjectToWorldEXT[1][2], 0,
      gl_ObjectToWorldEXT[2][0], gl_ObjectToWorldEXT[2][1], gl_ObjectToWorldEXT[2][2], 0,
      gl_ObjectToWorldEXT[3][0], gl_ObjectToWorldEXT[3][1], gl_ObjectToWorldEXT[3][2], 1.0);

  uint prim_index = gl_PrimitiveID + gl_GeometryIndexEXT + gl_InstanceCustomIndexEXT;

  Aabb aabb = deref(p.aabb_buffer).aabbs[prim_index];
  vec3 center = (aabb.minimum + aabb.maximum) * 0.5;
  center = (model * vec4(center, 1)).xyz;

  // Computing the normal at hit position
  vec3 world_nrm = normalize(world_pos - center);

  {
    vec3  absN = abs(world_nrm);
    float maxC = max(max(absN.x, absN.y), absN.z);
    world_nrm   = (maxC == absN.x) ? vec3(sign(world_nrm.x), 0, 0) :
                 (maxC == absN.y) ? vec3(0, sign(world_nrm.y), 0) :
                                    vec3(0, 0, sign(world_nrm.z));
  }

  const vec3 diffuse = world_nrm * 0.5 + 0.5;

#ifdef DEBUG_NORMALS
  prd.hitValue = diffuse;
  // prd.hitValue = center;
#else 
  float attenuation = 0.3;

  // Tracing shadow ray only if the light is visible from the surface
  if(dot(world_nrm, cLight.outLightDir) > 0)
  {
    float tMin   = 0.001;
    float tMax   = cLight.outLightDistance;
    vec3  origin = world_pos;
    vec3  rayDir = cLight.outLightDir;
    uint  flags  = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
    isShadowed   = true;
    traceRayEXT(daxa_accelerationStructureEXT(p.tlas),  // acceleration structure
                flags,       // rayFlags
                0xFF,        // cullMask
                0,           // sbtRecordOffset
                0,           // sbtRecordStride
                1,           // missIndex
                origin,      // ray origin
                tMin,        // ray min range
                rayDir,      // ray direction
                tMax,        // ray max range
                1            // payload (location = 1)
    );

    if(isShadowed)
    {
      attenuation = 0.3;
    } else {
      attenuation = 1.0;
    }
  }

  // if (isShadowed) {
  //     prd.hitValue = vec3(1.0, 0.0, 0.0); // Visualize shadows in red
  // } else {
      prd.hitValue = vec3(cLight.outIntensity * attenuation * (diffuse));
  // }
#endif // DEBUG_NORMALS

}
