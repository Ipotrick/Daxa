#include "fftutil.hlsl"

struct Push {
    uint globalsID;
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
    RWTexture2D<float2> RGFreqImage = daxa::getRWTexture2D<float2>(globals[0].horFreqImageRID);
    RWTexture2D<float2> BAFreqImage = daxa::getRWTexture2D<float2>(globals[0].horFreqImageGID);

    fft_horizontal_shared_mem_1024_forward(
        extractedImage,
        RGFreqImage,
        BAFreqImage,
        dispatchThreadID.x,
        dispatchThreadID.y
    );
	AllMemoryBarrierWithGroupSync();
    fft_horizontal_shared_mem_1024_backward(
        extractedImage,
        RGFreqImage,
        BAFreqImage,
        dispatchThreadID.x,
        dispatchThreadID.y
    );
}