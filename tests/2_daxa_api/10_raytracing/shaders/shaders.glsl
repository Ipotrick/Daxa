#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable
#include <daxa/daxa.inl>

#include "shared.inl"

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)

// Credit: https://gamedev.stackexchange.com/questions/92015/optimized-linear-to-srgb-glsl
vec4 fromLinear(vec4 linearRGB)
{
    bvec4 cutoff = lessThan(linearRGB, vec4(0.0031308));
    vec4 higher = vec4(1.055) * pow(linearRGB, vec4(1.0 / 2.4)) - vec4(0.055);
    vec4 lower = linearRGB * vec4(12.92);

    return mix(higher, lower, cutoff);
}

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
        0);
    float t_min = 0.0f;
    vec3 direction = vec3(0, 0, 1);
    float t_max = 100.0f;
    rayQueryEXT ray_query;
    rayQueryInitializeEXT(
        ray_query, daxa_accelerationStructureEXT(p.tlas),
        gl_RayFlagsOpaqueEXT,
        cull_mask, origin, t_min, direction, t_max);

    while (rayQueryProceedEXT(ray_query))
    {
        uint type = rayQueryGetIntersectionTypeEXT(ray_query, false);
        if (type == gl_RayQueryCandidateIntersectionTriangleEXT)
        {
            rayQueryConfirmIntersectionEXT(ray_query);
        }
        else if (type == gl_RayQueryCandidateIntersectionAABBEXT)
        {
            rayQueryGenerateIntersectionEXT(ray_query, t_max);
        }
    }

    vec3 out_colour = vec3(0.0, 0.0, 0.0);
    uint type = rayQueryGetIntersectionTypeEXT(ray_query, true);

    if (type == gl_RayQueryCommittedIntersectionTriangleEXT)
    {
        vec2 barycentrics = rayQueryGetIntersectionBarycentricsEXT(ray_query, true);
        out_colour = vec3(barycentrics.x, barycentrics.y, 1.0 - barycentrics.x - barycentrics.y);
    }
    else if (type == gl_RayQueryCommittedIntersectionGeneratedEXT)
    {
        // interpolate from gl_GlobalInvocationID.xy and clamp to [0,1]
        out_colour = vec3(
            (float(index.x) + 0.5f) / float(p.size.x),
            (float(index.y) + 0.5f) / float(p.size.y),
            abs(sin(float(index.x) * float(index.y))));
    }

    imageStore(daxa_image2D(p.swapchain), index, fromLinear(vec4(out_colour, 1)));
}