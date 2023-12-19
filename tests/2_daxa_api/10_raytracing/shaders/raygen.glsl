#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#include <daxa/daxa.inl>

#include "shared.inl"

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)

layout(location = 0) rayPayloadEXT hitPayload prd;

// Credit: https://gamedev.stackexchange.com/questions/92015/optimized-linear-to-srgb-glsl
vec4 fromLinear(vec4 linearRGB)
{
    bvec4 cutoff = lessThan(linearRGB, vec4(0.0031308));
    vec4 higher = vec4(1.055)*pow(linearRGB, vec4(1.0/2.4)) - vec4(0.055);
    vec4 lower = linearRGB * vec4(12.92);

    return mix(higher, lower, cutoff);
}

void main()
{
    const ivec2 index = ivec2(gl_LaunchIDEXT.xy);
    // if (index.x >= p.size.x || index.y >= p.size.y)
    // {
    //     return;
    // }

    uint cull_mask = 0xff;
    vec3 origin = vec3(
        (float(index.x) + 0.5f) / float(p.size.x),
        (float(index.y) + 0.5f) / float(p.size.y),
        0
    );
    // vec3 origin = vec3(index.x, index.y, 0);

    vec3 direction = vec3(0,0,1);
    
    uint  rayFlags = gl_RayFlagsOpaqueEXT;
    float tMin     = 0.001;
    float tMax     = 10000.0;

    traceRayEXT(daxa_accelerationStructureEXT(p.tlas), // acceleration structure
                rayFlags,                              // rayFlags
                0xFF,                                  // cullMask
                0,                                     // sbtRecordOffset
                0,                                     // sbtRecordStride
                0,                                     // missIndex
                origin.xyz,                            // ray origin
                tMin,                                  // ray min range
                direction.xyz,                         // ray direction
                tMax,                                  // ray max range
                0                                      // payload (location = 0)
    );

    // imageStore(daxa_image2D(p.swapchain), index, fromLinear(vec4(prd.hitValue, 1.0)));
    imageStore(daxa_image2D(p.swapchain), index, vec4(prd.hitValue, 1.0));
    // imageStore(daxa_image2D(p.swapchain), index, vec4(1.0, 1.0, 1.0, 1.0));
}