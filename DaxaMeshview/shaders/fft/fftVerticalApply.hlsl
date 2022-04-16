#include "fftutil.hlsl"

struct Push {
    uint globalsID;
};
[[vk::push_constant]] Push p;

[numthreads(1,T,1)]
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

    RWTexture2D<float4> extractedImage = daxa::getRWTexture2D<float4>(globals[0].fftImageID);
    RWTexture2D<float2> RGFreqImage = daxa::getRWTexture2D<float2>(globals[0].horFreqImageRGID);
    RWTexture2D<float2> BAFreqImage = daxa::getRWTexture2D<float2>(globals[0].horFreqImageBAID);
    RWTexture2D<float2> fullRG = daxa::getRWTexture2D<float2>(globals[0].fullFreqRGID);
    RWTexture2D<float2> fullBA = daxa::getRWTexture2D<float2>(globals[0].fullFreqBAID);
    RWTexture2D<float4> kernel = daxa::getRWTexture2D<float4>(globals[0].kernelID);

    fft_vertical_apply(
        kernel,
        fullRG,
        fullBA,
        extractedImage,
        RGFreqImage, 
        BAFreqImage,
        dispatchThreadID.x,
        dispatchThreadID.y
    );
}