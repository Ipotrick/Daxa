#include "fftutil.hlsl"

struct Push {
    uint apertureID;
    uint horFreqImageRG;
    uint horFreqImageBA;
    uint kernelImageID;
    uint kernelID;
};
[[vk::push_constant]] Push p;

[numthreads(T,1,1)]
void MainHorizontal0(
    int3 dispatchThreadID : SV_DispatchThreadID 
) {
    Texture2D<float4> aperature = daxa::getTexture2D<float4>(p.apertureID);
    // greyscale transform, so only one of the freq images are needed
    RWTexture2D<float2> freqImageRG = daxa::getRWTexture2D<float2>(p.horFreqImageRG);

    // forward transform
    int sign = 1;
    float2 v[2];
    for (int r = 0; r < 2; ++r) {
        v[r] = float2(
            aperature[int2(r * T, 0) + dispatchThreadID.xy].r,
            0
        );
    }
	do_fft(v, dispatchThreadID.x, sign);

    for (int r = 0; r < 2; ++r) {
        freqImageRG[int2(r * T, 0) + dispatchThreadID.xy] = v[r];
    }
}

[numthreads(1,T,1)]
void MainVertical0(
    int3 dispatchThreadID : SV_DispatchThreadID 
) {
    RWTexture2D<float2> freqImageRG = daxa::getRWTexture2D<float2>(p.horFreqImageRG);
    RWTexture2D<float4> kernelImage = daxa::getRWTexture2D<float4>(p.kernelImageID);

    // forward transform
    int sign = 1;

    float2 v[2];
	for(int r = 0; r < 2; ++r) {
		v[r] = freqImageRG[int2(0, r * T) + dispatchThreadID.xy];
	}
	do_fft(v, dispatchThreadID.y, sign);
    
	for(int r = 0; r < 2; ++r) {
        int2 index = int2((dispatchThreadID.x), (r * T + dispatchThreadID.y));
        float value = pow(length(v[r]) * rN * rN,1.4);
        if (index.x == 0 && index.y == 0) {
            value = 1;
        } 
        //else {
        //    value = 0;
        //}
        //kernelImage[int2((dispatchThreadID.x + T) % N, (r * T + dispatchThreadID.y + T) % N)] = float4(
        kernelImage[index] = float4(
            value,
            value,
            value,
            0
        );
	}
}

// pass 1 is wrong, it does not fft

[numthreads(T,1,1)]
void MainHorizontal1(
    int3 dispatchThreadID : SV_DispatchThreadID 
) {
    //Texture2D<float4> kernelImage = daxa::getTexture2D<float4>(p.apertureID);
    RWTexture2D<float4> kernelImage = daxa::getRWTexture2D<float4>(p.kernelImageID);
    // greyscale transform, so only one of the freq images are needed
    RWTexture2D<float2> freqImageRG = daxa::getRWTexture2D<float2>(p.horFreqImageRG);
    RWTexture2D<float2> freqImageBA = daxa::getRWTexture2D<float2>(p.horFreqImageBA);
//
    // forward transform
    int sign = 1;
    //float2 v_rg[2];
    //float2 v_ba[2];
    //for (int r = 0; r < 2; ++r) {
    //    v_rg[r] = kernelImage[int2(r * T, 0) + dispatchThreadID.xy].rg;
    //    v_ba[r] = kernelImage[int2(r * T, 0) + dispatchThreadID.xy].ba;
    //}
	//do_fft(v_rg, dispatchThreadID.x, sign);
    //GroupMemoryBarrierWithGroupSync();
	//do_fft(v_ba, dispatchThreadID.x, sign);
    //for (int r = 0; r < 2; ++r) {
    //    freqImageRG[int2(r * T, 0) + dispatchThreadID.xy] = v_rg[r];
    //    freqImageBA[int2(r * T, 0) + dispatchThreadID.xy] = v_ba[r];
    //}
    int t = dispatchThreadID.x;
    for (int r = 0; r < 2; ++r) {
        shared_freq[t + r * T] = kernelImage[int2(r * T, 0) + dispatchThreadID.xy].r;
        shared_freq[t + r * T + N] = kernelImage[int2(r * T, 0) + dispatchThreadID.xy].g;
    }
    do_fft2(shared_freq, dispatchThreadID.x, sign);
    for (int r = 0; r < 2; ++r) {
        freqImageRG[int2(r * T, 0) + dispatchThreadID.xy] = float2(shared_freq[t + r * T], shared_freq[t + r * T + N]);
    }
    GroupMemoryBarrierWithGroupSync();
    for (int r = 0; r < 2; ++r) {
        shared_freq[t + r * T] = kernelImage[int2(r * T, 0) + dispatchThreadID.xy].b;
        shared_freq[t + r * T + N] = kernelImage[int2(r * T, 0) + dispatchThreadID.xy].a;
    }
    do_fft2(shared_freq, dispatchThreadID.x, sign);
    for (int r = 0; r < 2; ++r) {
        freqImageBA[int2(r * T, 0) + dispatchThreadID.xy] = float2(shared_freq[t + r * T], shared_freq[t + r * T + N]);
    }
}

[numthreads(1,T,1)]
void MainVertical1(
    int3 dispatchThreadID : SV_DispatchThreadID 
) {
    RWTexture2D<float2> freqImageRG = daxa::getRWTexture2D<float2>(p.horFreqImageRG);
    RWTexture2D<float2> freqImageBA = daxa::getRWTexture2D<float2>(p.horFreqImageBA);
    RWTexture2D<float4> kernel = daxa::getRWTexture2D<float4>(p.kernelID);

    // forward transform
    int sign = 1;

    float2 v_rg[2];
    float2 v_ba[2];
	for(int r = 0; r < 2; ++r) {
        v_rg[r] = freqImageRG[int2(0, r * T) + dispatchThreadID.xy];
        v_ba[r] = freqImageBA[int2(0, r * T) + dispatchThreadID.xy];
	}
	do_fft(v_rg, dispatchThreadID.y, sign);
    GroupMemoryBarrierWithGroupSync();
	do_fft(v_ba, dispatchThreadID.y, sign);
    
	for(int r = 0; r < 2; ++r) {
        float value = length(v_rg[r]);// * 100;
        float value2 = length(v_ba[r]);// * 100;
        //kernel[int2(0, r * T) + dispatchThreadID.xy] = float4(
        //    v_rg[r],
        //    v_ba[r]
        //) * 100000;
        kernel[int2((dispatchThreadID.x), (r * T + dispatchThreadID.y))] = float4(
            v_rg[r].x,
            0,
            v_ba[r]
        );
        //kernel[int2((dispatchThreadID.x), (r * T + dispatchThreadID.y))] = float4(
        //    1,
        //    0,
        //    1,
        //    0
        //);
	}
}