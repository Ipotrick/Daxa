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
    RWTexture2D<float2> rFreqImage = daxa::getRWTexture2D<float2>(globals[0].horFreqImageRID);

    //fft_horizontal_r_shared_mem_1024_forward(
    //    extractedImage,
    //    rFreqImage,
    //    dispatchThreadID.x,
    //    dispatchThreadID.y
    //);
	//AllMemoryBarrierWithGroupSync();
    //fft_horizontal_r_shared_mem_1024_backward(
    //    extractedImage,
    //    rFreqImage,
    //    dispatchThreadID.x,
    //    dispatchThreadID.y
    //);
}