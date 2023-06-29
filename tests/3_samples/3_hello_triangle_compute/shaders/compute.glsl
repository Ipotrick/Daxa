#define DAXA_ENABLE_SHADER_NO_NAMESPACE 1
#include <shared.inl>

DAXA_DECL_PUSH_CONSTANT(ComputePush, push)

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    u32vec3 pixel_i = gl_GlobalInvocationID.xyz;
    if (pixel_i.x >= push.frame_dim.x || pixel_i.y >= push.frame_dim.y)
        return;

    f32vec2 uv = f32vec2(pixel_i.xy) / f32vec2(push.frame_dim.xy);
    uv = (uv - 0.5) * f32vec2(f32(push.frame_dim.x) / f32(push.frame_dim.y), 1);
    uv = uv * 2;

    f32vec3 col = f32vec3(0, 0, 0);

    f32vec2 points[3] = {
        f32vec2(-0.5, +0.5),
        f32vec2(+0.5, +0.5),
        f32vec2(+0.0, -0.5),
    };

    f32vec3 point_colors[3] = {
        f32vec3(1, 0, 0),
        f32vec3(0, 1, 0),
        f32vec3(0, 0, 1),
    };

    f32vec2 points_del[3] = {
        points[1] - points[0],
        points[2] - points[1],
        points[0] - points[2],
    };

    f32 slopes[3] = {
        points_del[0].y / points_del[0].x,
        points_del[1].y / points_del[1].x,
        points_del[2].y / points_del[2].x,
    };

    if (slopes[0] * (uv.x - points[0].x) > (uv.y - points[0].y) &&
        slopes[1] * (uv.x - points[1].x) < (uv.y - points[1].y) &&
        slopes[2] * (uv.x - points[2].x) < (uv.y - points[2].y))
    {
        f32 p0 = clamp(dot(points_del[0], uv - points[0]) / dot(points_del[0], points_del[0]), 0, 1);
        f32 p1 = clamp(dot(points_del[1], uv - points[1]) / dot(points_del[1], points_del[1]), 0, 1);
        f32 p2 = clamp(dot(points_del[2], uv - points[2]) / dot(points_del[2], points_del[2]), 0, 1);

        col = mix(col, point_colors[0], f32vec3(1));
        col = mix(col, point_colors[1], p0);
        col = mix(col, point_colors[2], clamp((p1 - p2 + 0.5) / 1.5, 0, 1));
    }

    imageStore(daxa_image2D(push.image),i32vec2(pixel_i.xy),f32vec4(col, 1));
}
