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

  // Vertex positions in object space from GL_EXT_ray_tracing_position_fetch extension
  vec3 v0 = gl_HitTriangleVertexPositionsEXT[0];
  vec3 v1 = gl_HitTriangleVertexPositionsEXT[1];
  vec3 v2 = gl_HitTriangleVertexPositionsEXT[2];

  // Color vertices (adjust these colors as desired)
  vec3 color0 = vec3(1.0, 0.0, 0.0); // Red
  vec3 color1 = vec3(0.0, 1.0, 0.0); // Green
  vec3 color2 = vec3(0.0, 0.0, 1.0); // Blue

  // Interpolate colors using barycentric coordinates
  vec3 interpolatedColor = barycentrics.x * color0 + barycentrics.y * color1 + barycentrics.z * color2;

  // Output the interpolated color
  prd.hitValue = interpolatedColor;
}
