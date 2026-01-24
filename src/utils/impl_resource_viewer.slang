#ifndef __TASK_GRAPH_DEBUG_UI_SLANG__
#define __TASK_GRAPH_DEBUG_UI_SLANG__

#include <daxa/daxa.inl>

#define TASK_GRAPH_DEBUG_UI_X 8
#define TASK_GRAPH_DEBUG_UI_Y 8

#define READBACK_CIRCULAR_BUFFER_SIZE 4

struct TaskGraphDebugUiImageReadbackStruct
{
    daxa_f32vec4 hovered_color;
    daxa_u32vec4 hovered_value;
    daxa_u32 pos_max_value;
    daxa_u32 pos_min_value;
    daxa_u32 neg_max_value;
    daxa_u32 neg_min_value;
};

struct TaskGraphDebugUiPush
{
    daxa::ImageViewId src;
    daxa::RWTexture2DIndex<daxa_f32vec4> dst;
    daxa_u32vec2 display_image_size;
    daxa::u32 image_view_type;
    daxa_i32 format;
    daxa::u32 display_min;
    daxa::u32 display_max;
    daxa::i32 rainbow_ints;
    daxa::i32 gamma_correct;
    daxa_i32vec4 enabled_channels;
    daxa_i32vec2 mouse_over_index;
    daxa_BufferPtr(TaskGraphDebugUiImageReadbackStruct) readback_ptr;
};

#if !defined(__cplusplus)

daxa_f32vec3 srgb_to_linear(daxa_f32vec3 srgb)
{
    daxa_f32vec3 color_srgb = srgb.rgb;
    daxa_f32vec3 selector = clamp(ceil(color_srgb - 0.04045), 0.0, 1.0); // 0 if under value, 1 if over
    daxa_f32vec3 under = color_srgb / 12.92;
    daxa_f32vec3 over = pow((color_srgb + 0.055) / 1.055, daxa_f32vec3(2.4, 2.4, 2.4));
    daxa_f32vec3 result = lerp(under, over, selector);
    return daxa_f32vec3(result);
}

[[vk::push_constant]] TaskGraphDebugUiPush push;

float3 hsv2rgb(float3 c) {
    float4 k = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + k.xyz) * 6.0 - k.www);
    return c.z * lerp(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
}

float3 rainbow_maker(uint i)
{
    return (0.2987123 * float(i), 1.0f, 1.0f);
}
float3 rainbow_maker(int i)
{
    return (0.2987123 * float(i), 1.0f, 1.0f);
}

[numthreads(TASK_GRAPH_DEBUG_UI_X, TASK_GRAPH_DEBUG_UI_Y, 1)]
[shader("compute")]
func main(uint2 dtid : SV_DispatchThreadID)
{
    if (any(dtid >= push.display_image_size))
        return;

    float4 sample_color = float4(0,0,0,0);

    let readback_pixel = all(dtid == push.mouse_over_index);
    
    let one_channel_active = (push.enabled_channels[0] + push.enabled_channels[1] + push.enabled_channels[2] + push.enabled_channels[3]) == 1;
    let only_alpha_active = one_channel_active && push.enabled_channels[3] == 1;

    uint pos_max = 0u;
    uint pos_min = ~0u;
    uint neg_max = 0u;
    uint neg_min = ~0u;
    switch (push.format)
    {
        case 0: 
        {
            var sample = RWTexture2D<float4>::get(push.src)[dtid];
            if (readback_pixel)
            {
                push.readback_ptr->hovered_value = asuint(sample);
            }

            pos_max = push.enabled_channels.x != 0 && sample.x >= 0.0f ? max(pos_max, asuint(sample.x)) : pos_max;
            pos_max = push.enabled_channels.y != 0 && sample.y >= 0.0f ? max(pos_max, asuint(sample.y)) : pos_max;
            pos_max = push.enabled_channels.z != 0 && sample.z >= 0.0f ? max(pos_max, asuint(sample.z)) : pos_max;
            pos_max = push.enabled_channels.w != 0 && sample.w >= 0.0f ? max(pos_max, asuint(sample.w)) : pos_max;
            pos_min = push.enabled_channels.x != 0 && sample.x >= 0.0f ? min(pos_min, asuint(sample.x)) : pos_min;
            pos_min = push.enabled_channels.y != 0 && sample.y >= 0.0f ? min(pos_min, asuint(sample.y)) : pos_min;
            pos_min = push.enabled_channels.z != 0 && sample.z >= 0.0f ? min(pos_min, asuint(sample.z)) : pos_min;
            pos_min = push.enabled_channels.w != 0 && sample.w >= 0.0f ? min(pos_min, asuint(sample.w)) : pos_min;
            
            neg_max = push.enabled_channels.x != 0 && sample.x < 0.0f ? max(neg_max, asuint(abs(sample.x))) : neg_max;
            neg_max = push.enabled_channels.y != 0 && sample.y < 0.0f ? max(neg_max, asuint(abs(sample.y))) : neg_max;
            neg_max = push.enabled_channels.z != 0 && sample.z < 0.0f ? max(neg_max, asuint(abs(sample.z))) : neg_max;
            neg_max = push.enabled_channels.w != 0 && sample.w < 0.0f ? max(neg_max, asuint(abs(sample.w))) : neg_max;
            neg_min = push.enabled_channels.x != 0 && sample.x < 0.0f ? min(neg_min, asuint(abs(sample.x))) : neg_min;
            neg_min = push.enabled_channels.y != 0 && sample.y < 0.0f ? min(neg_min, asuint(abs(sample.y))) : neg_min;
            neg_min = push.enabled_channels.z != 0 && sample.z < 0.0f ? min(neg_min, asuint(abs(sample.z))) : neg_min;
            neg_min = push.enabled_channels.w != 0 && sample.w < 0.0f ? min(neg_min, asuint(abs(sample.w))) : neg_min;
            
            sample_color = float4(sample - asfloat(push.display_min)) * rcp(asfloat(push.display_max) - asfloat(push.display_min));
        }
        break;
        case 1: 
        {
            var sample = RWTexture2D<int4>::get(push.src)[dtid];
            if (readback_pixel)
            {
                push.readback_ptr->hovered_value = asuint(sample);
            }

            pos_max = push.enabled_channels.x != 0 && sample.x >= 0 ? max(pos_max, asuint(sample.x)) : pos_max;
            pos_max = push.enabled_channels.y != 0 && sample.y >= 0 ? max(pos_max, asuint(sample.y)) : pos_max;
            pos_max = push.enabled_channels.z != 0 && sample.z >= 0 ? max(pos_max, asuint(sample.z)) : pos_max;
            pos_max = push.enabled_channels.w != 0 && sample.w >= 0 ? max(pos_max, asuint(sample.w)) : pos_max;
            pos_min = push.enabled_channels.x != 0 && sample.x >= 0 ? min(pos_min, asuint(sample.x)) : pos_min;
            pos_min = push.enabled_channels.y != 0 && sample.y >= 0 ? min(pos_min, asuint(sample.y)) : pos_min;
            pos_min = push.enabled_channels.z != 0 && sample.z >= 0 ? min(pos_min, asuint(sample.z)) : pos_min;
            pos_min = push.enabled_channels.w != 0 && sample.w >= 0 ? min(pos_min, asuint(sample.w)) : pos_min;
            
            neg_max = push.enabled_channels.x != 0 && sample.x < 0 ? max(neg_max, asuint(abs(sample.x))) : neg_max;
            neg_max = push.enabled_channels.y != 0 && sample.y < 0 ? max(neg_max, asuint(abs(sample.y))) : neg_max;
            neg_max = push.enabled_channels.z != 0 && sample.z < 0 ? max(neg_max, asuint(abs(sample.z))) : neg_max;
            neg_max = push.enabled_channels.w != 0 && sample.w < 0 ? max(neg_max, asuint(abs(sample.w))) : neg_max;
            neg_min = push.enabled_channels.x != 0 && sample.x < 0 ? min(neg_min, asuint(abs(sample.x))) : neg_min;
            neg_min = push.enabled_channels.y != 0 && sample.y < 0 ? min(neg_min, asuint(abs(sample.y))) : neg_min;
            neg_min = push.enabled_channels.z != 0 && sample.z < 0 ? min(neg_min, asuint(abs(sample.z))) : neg_min;
            neg_min = push.enabled_channels.w != 0 && sample.w < 0 ? min(neg_min, asuint(abs(sample.w))) : neg_min;

            if (push.rainbow_ints)
                sample_color = float4(rainbow_maker(sample.x), 1);
            else
                sample_color = float4(sample - asint(push.display_min)) * rcp(asint(push.display_max) - asint(push.display_min));
        }
        break;
        case 2: 
        {
            var sample = RWTexture2D<uint4>::get(push.src)[dtid];
            if (readback_pixel)
            {
                push.readback_ptr->hovered_value = sample;
            }

            pos_max = push.enabled_channels.x != 0 ? max(pos_max, sample.x) : pos_max;
            pos_max = push.enabled_channels.y != 0 ? max(pos_max, sample.y) : pos_max;
            pos_max = push.enabled_channels.z != 0 ? max(pos_max, sample.z) : pos_max;
            pos_max = push.enabled_channels.w != 0 ? max(pos_max, sample.w) : pos_max;
            pos_min = push.enabled_channels.x != 0 ? min(pos_min, sample.x) : pos_min;
            pos_min = push.enabled_channels.y != 0 ? min(pos_min, sample.y) : pos_min;
            pos_min = push.enabled_channels.z != 0 ? min(pos_min, sample.z) : pos_min;
            pos_min = push.enabled_channels.w != 0 ? min(pos_min, sample.w) : pos_min;

            if (push.rainbow_ints)
                sample_color = float4(rainbow_maker(sample.x), 1);
            else
                sample_color = float4((sample - push.display_min) * rcp(push.display_max - push.display_min));    
        }
        break;
    }

    InterlockedMax(push.readback_ptr->pos_max_value, pos_max);
    InterlockedMin(push.readback_ptr->pos_min_value, pos_min);
    InterlockedMax(push.readback_ptr->neg_max_value, neg_max);
    InterlockedMin(push.readback_ptr->neg_min_value, neg_min);

    if (one_channel_active)
    {
        const uint channel = dot(int4(0, 1, 2, 3), push.enabled_channels);
        sample_color = sample_color[channel].xxxx;
    }
    else
    {
        sample_color[0] = push.enabled_channels[0] != 0 ? sample_color[0] : 0.0f;
        sample_color[1] = push.enabled_channels[1] != 0 ? sample_color[1] : 0.0f;
        sample_color[2] = push.enabled_channels[2] != 0 ? sample_color[2] : 0.0f;
        sample_color[3] = push.enabled_channels[3] != 0 ? sample_color[3] : 1.0f;
    }

    if (readback_pixel)
    {
        push.readback_ptr->hovered_color = sample_color;
    }

    float3 display_color = sample_color.rgb;
    if (push.gamma_correct == 0)
    {
       display_color = srgb_to_linear(sample_color.rgb);
    }
    push.dst.get()[dtid] = float4(display_color.rgb, 1.0f);
}

#endif

#endif // __TASK_GRAPH_DEBUG_UI_SLANG__