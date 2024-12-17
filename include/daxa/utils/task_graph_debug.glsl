#extension GL_GOOGLE_include_directive : enable
#extension GL_KHR_memory_scope_semantics : enable

#include "daxa/utils/task_graph_debug.inl"

DAXA_DECL_PUSH_CONSTANT(DebugTaskDrawDebugDisplayPush, p)

vec3 hsv2rgb(vec3 c)
{
    vec4 k = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + k.xyz) * 6.0 - k.www);
    return c.z * mix(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
}

vec3 rainbow_maker(uint i)
{
    return vec3(0.2987123 * float(i), 1.0f, 1.0f);
}
vec3 rainbow_maker(int i)
{
    return vec3(0.2987123 * float(i), 1.0f, 1.0f);
}

#define rcp(x) (1.0 / (x))

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void entry_draw_debug_display()
{
    uvec2 thread_index = gl_GlobalInvocationID.xy;

    if (any(greaterThanEqual(thread_index, p.src_size)))
        return;

    vec4 sample_color = vec4(0, 0, 0, 0);

    const bool readback_pixel = all(equal(thread_index, p.mouse_over_index));

    bool one_channel_active = (p.enabled_channels[0] + p.enabled_channels[1] + p.enabled_channels[2] + p.enabled_channels[3]) == 1;
    bool alpha_only = one_channel_active && p.enabled_channels[3] == 1;

    switch (p.format)
    {
    case 0:
    {
        vec4 value = texelFetch(daxa_texture2D(p.src), ivec2(thread_index), 0);
        if (readback_pixel)
        {
            deref(daxa_BufferPtr(vec4)(advance(p.readback_ptr, p.readback_index * 2))) = value;
        }
        sample_color = vec4((value.rgb - p.float_min) * rcp(p.float_max - p.float_min), value.a);
        if (alpha_only)
            sample_color.a = (value.a - p.float_min) * rcp(p.float_max - p.float_min);
    }
    break;
    case 1:
    {
        ivec4 value = texelFetch(daxa_itexture2D(p.src), ivec2(thread_index), 0);
        if (readback_pixel)
        {
            deref(daxa_BufferPtr(ivec4)(advance(p.readback_ptr, p.readback_index * 2))) = value;
        }
        if (p.rainbow_ints != 0)
            sample_color = vec4(rainbow_maker(value.x), 1);
        else
            sample_color = vec4((value.rgb - p.int_min) * rcp(p.int_max - p.int_min), value.a);
        if (alpha_only)
            sample_color.a = (value.a - p.int_min) * rcp(p.int_max - p.int_min);
    }
    break;
    case 2:
    {
        uvec4 value = texelFetch(daxa_utexture2D(p.src), ivec2(thread_index), 0);
        if (readback_pixel)
        {
            deref(daxa_BufferPtr(uvec4)(advance(p.readback_ptr, p.readback_index * 2))) = value;
        }
        if (p.rainbow_ints != 0)
            sample_color = vec4(rainbow_maker(value.x), 1);
        else
            sample_color = vec4((value.rgb - p.uint_min) * rcp(p.uint_max - p.uint_min), value.a);
        if (alpha_only)
            sample_color.a = (value.a - p.uint_min) * rcp(p.uint_max - p.uint_min);
    }
    break;
    }

    sample_color[0] = p.enabled_channels[0] != 0 ? sample_color[0] : 0.0f;
    sample_color[1] = p.enabled_channels[1] != 0 ? sample_color[1] : 0.0f;
    sample_color[2] = p.enabled_channels[2] != 0 ? sample_color[2] : 0.0f;
    sample_color[3] = p.enabled_channels[3] != 0 ? sample_color[3] : 1.0f;

    if (one_channel_active)
    {
        float single_channel_color =
            (p.enabled_channels[0] * sample_color[0]) +
            (p.enabled_channels[1] * sample_color[1]) +
            (p.enabled_channels[2] * sample_color[2]) +
            (p.enabled_channels[3] * sample_color[3]);
        sample_color.xyz = vec3(single_channel_color);
        sample_color[3] = 1.0f;
    }

    if (readback_pixel)
    {
        deref(advance(p.readback_ptr, p.readback_index * 2 + 1)) = sample_color;
        float color_max = max(max(sample_color.x, sample_color.y), max(sample_color.z, sample_color.w));
        uint color_max_int = uint(color_max);
        float color_min = max(max(sample_color.x, sample_color.y), max(sample_color.z, sample_color.w));
    }

    vec4 previous_value = imageLoad(daxa_image2D(p.dst), ivec2(thread_index));
    imageStore(daxa_image2D(p.dst), ivec2(thread_index), vec4(mix(previous_value.rgb, sample_color.rgb, sample_color.a), 1.0f));
}
