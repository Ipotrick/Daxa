#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#include <daxa/daxa.inl>

#include "shared.inl"

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)


// Ray-AABB intersection
float hitAabb(const Aabb aabb, const Ray r)
{
  vec3  invDir = 1.0 / r.direction;
  vec3  tbot   = invDir * (aabb.minimum - r.origin);
  vec3  ttop   = invDir * (aabb.maximum - r.origin);
  vec3  tmin   = min(ttop, tbot);
  vec3  tmax   = max(ttop, tbot);
  float t0     = max(tmin.x, max(tmin.y, tmin.z));
  float t1     = min(tmax.x, min(tmax.y, tmax.z));
  return t1 > max(t0, 0.0) ? t0 : -1.0;
}

void main()
{
    Ray ray;
    ray.origin = gl_ObjectRayOriginEXT;
    ray.direction = gl_ObjectRayDirectionEXT;

    mat4 model = mat4(
      gl_WorldToObjectEXT[0][0], gl_WorldToObjectEXT[0][1], gl_WorldToObjectEXT[0][2], 0,
      gl_WorldToObjectEXT[1][0], gl_WorldToObjectEXT[1][1], gl_WorldToObjectEXT[1][2], 0,
      gl_WorldToObjectEXT[2][0], gl_WorldToObjectEXT[2][1], gl_WorldToObjectEXT[2][2], 0,
      gl_WorldToObjectEXT[3][0], gl_WorldToObjectEXT[3][1], gl_WorldToObjectEXT[3][2], 1.0);

    ray.origin = (model * vec4(ray.origin, 1)).xyz;
    ray.direction = (model * vec4(ray.direction, 0)).xyz;


    float tHit = -1;

    uint i = gl_PrimitiveID + gl_GeometryIndexEXT;

    Aabb aabb = deref(p.aabb_buffer).aabbs[i];
    aabb.minimum = (model * vec4(aabb.minimum, 1)).xyz;
    aabb.maximum = (model * vec4(aabb.maximum, 1)).xyz;


    tHit = hitAabb(aabb, ray);

    // Report hit point
    if (tHit > 0)
        reportIntersectionEXT(tHit, 0); // 0 is the hit kind (hit group index)
}
