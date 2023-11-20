#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query : require
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

    rayQueryEXT ray_query;
    uint cull_mask = 0xff;
    vec3 origin = vec3(
        (float(index.x) + 0.5f) / float(p.size.x),
        (float(index.y) + 0.5f) / float(p.size.y),
        0
    );
    float t_min = 0.0;
    vec3 direction = vec3(0,0,1);
    float t_max = 10000000.0;
    rayQueryInitializeEXT(ray_query, daxa_accelerationStructureEXT(p.tlas),
                        gl_RayFlagsTerminateOnFirstHitEXT,
                        cull_mask, origin, t_min, direction, t_max);

    while(rayQueryProceedEXT(ray_query)) {
        if (rayQueryGetIntersectionTypeEXT(ray_query, false) ==
            gl_RayQueryCandidateIntersectionTriangleEXT)
        {
            rayQueryConfirmIntersectionEXT(ray_query);
        }
    }
    if (rayQueryGetIntersectionTypeEXT(ray_query, true) ==
        gl_RayQueryCommittedIntersectionNoneEXT)
    {
        imageStore(daxa_image2D(p.swapchain), index, vec4(0,0,0,0));
    } else {
        imageStore(daxa_image2D(p.swapchain), index, vec4(0,1,1,0));
    }
}