#define DAXA_RAY_TRACING 1
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

    imageStore(daxa_image2D(p.swapchain), index, vec4(0,1,1,0));
}