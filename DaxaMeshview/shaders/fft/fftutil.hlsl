#pragma once

#include "fftcommon.hlsl"

#define PI 3.14159265359

#define T 512
#define N 1024

groupshared float shared_freq[N*2];

int expand(int idxL, int N1, int N2){
	return (idxL / N1) * N1 * N2 + (idxL % N1);
}

void exchange(
	inout float2 v[2], 
	int idxD, 
	int incD,
	int idxS, 
	int incS
){
    GroupMemoryBarrierWithGroupSync();
	for(int r = 0; r < 2; ++r) {
		int i = (idxD + r * incD);
		shared_freq[i] = v[r].x;
		shared_freq[i + N] = v[r].y;
	}
    GroupMemoryBarrierWithGroupSync();
	for(int r = 0; r < 2; ++r) {
		int i = (idxS + r * incS);
		v[r].x = shared_freq[i];
		v[r].y = shared_freq[i+N];
	}
}

float2 complex_mul(float2 a, float2 b) {
	return float2(
		a.x*b.x - a.y*b.y,
		a.x*b.y + a.y*b.x
	);
}

void do_fft_shared_mem_1024(inout float2 v[2], in int t, in int sign) {
	for(int Ns = 1; Ns < N; Ns *= 2) {
		float angle = sign * 2 * PI * (t % Ns) / (Ns * 2);
		for(int r = 0; r < 2; ++r) {
			v[r] = complex_mul(v[r], float2(cos(r * angle), sin(r * angle)));
		}

		float2 v0 = v[0];
		v[0] = v0 + v[1];
		v[1] = v0 - v[1];

		int idxD = expand(t, Ns, 2);
		int idxS = expand(t, N / 2, 2);
		exchange(v, idxD, Ns, idxS, N / 2);
	}
}

void fft_horizontal_shared_mem_1024_forward(
    RWTexture2D<float4> inImage,    // input image, is float4 but only one channel will be used
    RWTexture2D<float2> freqImgRG, 	// outout image for red and green channel
	RWTexture2D<float2> 	freqImgBA,	// output image for blue and alpha channel
    uint t,		 					// thread index into the fft sequence
	uint row 						// row we read from and write to in the image
) {
	int sign = 1;
	float s = (sign < 1) ? 1 : float(1) / float(N);

	float2 v_rg[2];
	for(int r = 0; r < 2; ++r) {
		v_rg[r] = float2(inImage[int2(t + r * T, row)].r, inImage[int2(t + r * T, row)].g);
	}
	do_fft_shared_mem_1024(v_rg, t, sign);

	for(int r = 0; r < 2; ++r) {
		freqImgRG[int2(t + r * T, row)] = s * v_rg[r];
	}

    GroupMemoryBarrierWithGroupSync();

	float2 v_ba[2];
	for(int r = 0; r < 2; ++r) {
		v_ba[r] = float2(inImage[int2(t + r * T, row)].b, inImage[int2(t + r * T, row)].a);
	}
	do_fft_shared_mem_1024(v_ba, t, sign);

	for(int r = 0; r < 2; ++r) {
		freqImgBA[int2(t + r * T, row)] = s * v_ba[r];
	}
}

void fft_horizontal_shared_mem_1024_backward(
    RWTexture2D<float4> outImage,   // input image, is float4 but only one channel will be used
    RWTexture2D<float2> RGFreqImg, 	// outout image in frequency domain
	RWTexture2D<float2> BAFreqImg,	// 
    uint t,		 					// thread index into the fft sequence
	uint row 						// row we read from and write to in the image
) { 
	int sign = -1;
	float s = (sign < 1) ? 1.0 : float(1) / float(N);

	float2 v_rg[2];
	for(int r = 0; r < 2; ++r) {
		v_rg[r] = RGFreqImg[int2(t + r * T, row)];
	}

	do_fft_shared_mem_1024(v_rg, t, sign);

    GroupMemoryBarrierWithGroupSync();

	float2 v_ba[2];
	for(int r = 0; r < 2; ++r) {
		v_ba[r] = BAFreqImg[int2(t + r * T, row)];
	}

	do_fft_shared_mem_1024(v_ba, t, sign);

	
	for(int r = 0; r < 2; ++r) {
		outImage[int2(t + r * T, row)] = float4(v_rg[r], v_ba[r]) * s;
	}
}