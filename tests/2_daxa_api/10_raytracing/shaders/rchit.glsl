#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_tracing_position_fetch : require
#include <daxa/daxa.inl>
#include "shared.inl"

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)

hitAttributeEXT vec2 attribs;

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main()
{

  // Barycentric coordinates from GL_EXT_ray_tracing extension
  const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  // Vertex positions from GL_EXT_ray_tracing_position_fetch extension
  vec3 v0 = gl_HitTriangleVertexPositionsEXT[0];
  vec3 v1 = gl_HitTriangleVertexPositionsEXT[1];
  vec3 v2 = gl_HitTriangleVertexPositionsEXT[2];

  // Position calculation
  const vec3 pos = v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
  const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.0));  // Transforming the position to world space

  // Normal calculation
  const vec3 normal = normalize(cross(v1 - v0, v2 - v0));
  const vec3 worldNormal = normalize(vec3(gl_ObjectToWorldEXT * vec4(normal, 0.0)));  // Transforming the normal to world space


  prd.hitValue = vec3(worldPos);
}
