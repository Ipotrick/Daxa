#include "fftcommon.hlsl"

struct Push {
    uint globalsID;
};
[[vk::push_constant]] Push p;

#define MAX_CHANNEL_VALUE 1

[numthreads(8,8,1)]
void Main(
    int3 dispatchThreadID : SV_DispatchThreadID 
) {
    StructuredBuffer<Globals> globals = daxa::getBuffer<Globals>(p.globalsID);

    if (
        dispatchThreadID.x >= globals[0].width ||
        dispatchThreadID.y >= globals[0].height
    ) {
        return;
    }
    float2 uv = float2(
        float(dispatchThreadID.x) / float(globals[0].width - globals[0].padWidth),
        float(dispatchThreadID.y) / float(globals[0].height - globals[0].padHeight)
    );

    RWTexture2D<float4> fftImage = daxa::getRWTexture2D<float4>(globals[0].fftImageID);
    Texture2D<float4> hdrImage = daxa::getTexture2D<float4>(globals[0].hdrImageID);
    int hdrWidth;
    int hdrHeight;
    hdrImage.GetDimensions(hdrWidth, hdrHeight);

    if (uv.x > 1.0 || uv.y > 1.0) {
        fftImage[dispatchThreadID.xy] = float4(0,0,0,1);
    } else {
        float4 color = hdrImage[int2(uv * int2(hdrWidth, hdrHeight))].rgba;
        if (length(color) > 2.0 || true) {
            fftImage[dispatchThreadID.xy] = float4(
                min(MAX_CHANNEL_VALUE, color.r),
                min(MAX_CHANNEL_VALUE, color.g),
                min(MAX_CHANNEL_VALUE, color.b),
                min(MAX_CHANNEL_VALUE, color.a)
            );
        } else {
            fftImage[dispatchThreadID.xy] = float4(0,0,0,1);
        }
    }
}