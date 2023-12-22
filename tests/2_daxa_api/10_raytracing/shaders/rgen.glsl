#define DAXA_RAY_TRACING 1
#extension GL_EXT_ray_tracing : enable
#include <daxa/daxa.inl>

#include "shared.inl"
#include "random.glsl"

DAXA_DECL_PUSH_CONSTANT(PushConstant, p)

layout(location = 0) rayPayloadEXT hitPayload prd;

const uint NBSAMPLES = 1;

void main()
{
    const ivec2 index = ivec2(gl_LaunchIDEXT.xy);

    uint frame = p.frame;
    
    uint seed = tea(gl_LaunchIDEXT.y * gl_LaunchSizeEXT.x + gl_LaunchIDEXT.x, frame * NBSAMPLES);
    prd.seed  = seed;

    uint cull_mask = 0xff;
    
    // Camera setup
    daxa_f32mat4x4 inv_view = deref(p.camera_buffer).inv_view;
    daxa_f32mat4x4 inv_proj = deref(p.camera_buffer).inv_proj;

    const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
    const vec2 inUV        = pixelCenter / vec2(gl_LaunchSizeEXT.xy);
    vec2       d           = inUV * 2.0 - 1.0;

    vec4 origin    = inv_view * vec4(0, 0, 0, 1);
    vec4 target    = inv_proj * vec4(d.x, d.y, 1, 1);
    vec4 direction = inv_view * vec4(normalize(target.xyz), 0);

    
    uint  rayFlags = gl_RayFlagsNoneEXT;
    float tMin     = 0.0001;
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