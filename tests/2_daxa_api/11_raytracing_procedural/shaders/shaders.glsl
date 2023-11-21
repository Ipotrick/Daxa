#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable
#include <daxa/daxa.inl>

#include "shared.inl"

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)

layout(local_size_x = 8, local_size_y = 8) in;
void main()
{
    const ivec2 index = ivec2(gl_GlobalInvocationID.xy);
    if (index.x >= p.size.x || index.y >= p.size.y)
    {
        return;
    }

    uint cull_mask = 0xff;
    vec3 origin = vec3(
        (float(index.x) + 0.5f) / float(p.size.x),
        (float(index.y) + 0.5f) / float(p.size.y),
        -3.0f
    );
    float t_min = 0.0f;
    vec3 direction = vec3(0,0,1);
    float t_max = 1000.0f;
    rayQueryEXT ray_query;

    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(ray_query, daxa_accelerationStructureEXT(p.tlas),
                        gl_RayFlagsOpaqueEXT,
                        cull_mask, origin, t_min, direction, t_max);

	rayQueryProceedEXT(rayQuery);

    vec3 out_colour = vec3(0.0, 0.0, 0.0);

	if (rayQueryGetIntersectionTypeEXT(rayQuery, false) ==
             gl_RayQueryCandidateIntersectionAABBEXT)
    {
		out_colour = vec3(1.0, 1.0, 1.0);
	}

    imageStore(daxa_image2D(p.swapchain), index, vec4(out_colour,1));
}