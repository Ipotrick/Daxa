#include "fftutil.hlsl"

struct Push {
    uint globalsID;
    uint backward;
};
[[vk::push_constant]] Push p;

[numthreads(T,1,1)]
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

    if (p.backward) {
        fft_horizontal_backward(
            extractedImage,
            fullRG,
            fullBA,
            RGFreqImage,
            BAFreqImage, 
            dispatchThreadID.x,
            dispatchThreadID.y
        );
    } else {
        fft_horizontal_forward(
            extractedImage,
            RGFreqImage,
            BAFreqImage,
            dispatchThreadID.x,
            dispatchThreadID.y
        );
    }
}