#include "daxa/daxa.inl"
#include "shared.inl"
// #include "custom file!!"

#if 1
[[vk::push_constant]] ComputePush p;

struct Test
{
    GpuInput* value;
};

#define CENTER float2(-0.694008, -0.324998)
#define SUBSAMPLES 2

float3 hsv2rgb(float3 c)
{
    float4 k = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + k.xyz) * 6.0 - k.www);
    return c.z * lerp(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
}

float test(float * v)
{
    return *v;
}

float3 mandelbrot_colored(float2 pixel_p)
{
    float2 uv = pixel_p / float2(p.frame_dim.xy);
    uv = (uv - 0.5) * float2(float(p.frame_dim.x) / float(p.frame_dim.y), 1);
    GpuInput * gpu_input = (GpuInput *)daxa::id_to_address(p.input_buffer_id);
    float time = test(&gpu_input[0].time);
    float scale = 12.0 / (exp(time) + 0.0001);
    float2 z = uv * scale * 2 + CENTER;
    float2 c = z;
    uint i = 0;
    for (; i < 512; ++i)
    {
        float2 z_ = z;
        z.x = z_.x * z_.x - z_.y * z_.y;
        z.y = 2.0 * z_.x * z_.y;
        z += c;
        if (dot(z, z) > 256 * 256)
            break;
    }
    float3 col = 0;
    if (i != 512)
    {
        float l = i;
        float sl = l - log2(log2(dot(z, z))) + 4.0;
        sl = pow(sl * 0.01, 1.0);
        col = hsv2rgb(float3(sl, 1, 1));
#if defined(MY_TOGGLE)
        col = 1 - col;
#endif
    }
    return col;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 pixel_i : SV_DispatchThreadID)
{
    if (pixel_i.x >= p.frame_dim.x || pixel_i.y >= p.frame_dim.y)
        return;

    float3 col = 0;
    for (int yi = 0; yi < SUBSAMPLES; ++yi)
    {
        for (int xi = 0; xi < SUBSAMPLES; ++xi)
        {
            float2 offset = float2(xi, yi) / float(SUBSAMPLES);
            col += mandelbrot_colored(float2(pixel_i.xy) + offset);
        }
    }
    col *= 1.0 / float(SUBSAMPLES * SUBSAMPLES);
    daxa_RWTexture2D(float4, p.image_id)[pixel_i.xy] = float4(col, 1);
}
#endif

#if 0
struct TestStruct
{
    uint index;
};

[[vk::binding(2, 0)]] StructuredBuffer<uint64_t> test;

struct PP
{
    int data;
    int data2;
}
struct Data
{
    int data;
    PP* pNext;
};

void funcThatTakesPointer(PP* p)
{
    p.data = 2;
}
int* funcThatReturnsPointer(PP* p)
{
    return &p.data;
}

// CHECK: OpEntryPoint

[[vk::binding(0, 0)]] StructuredBuffer<Data> buffer;
[[vk::binding(1, 0)]] RWStructuredBuffer<int> output;
[shader("compute")]
[numthreads(8, 8, 1)]
void main(int id : SV_DispatchThreadID)
{
    const TestStruct * ptr = (const TestStruct *)(test[0]);
    output[0] = buffer[ptr.index].pNext.data;
    let pData = &(buffer[0].pNext.data);
    // CHECK: OpPtrAccessChain
    int* pData1 = pData + 1;
    *pData1 = 3;
    *(int2*)pData = int2(1, 2);
    pData1[-1] = 2;
    buffer[0].pNext[1] = {5};
    // CHECK: OpConvertPtrToU
    // CHECK: OpINotEqual
    if (pData1)
    {
        *(funcThatReturnsPointer(buffer[0].pNext)) = 4;
    }
    if (pData1 > pData)
    {
        funcThatTakesPointer(buffer[0].pNext);
    }
}
#endif