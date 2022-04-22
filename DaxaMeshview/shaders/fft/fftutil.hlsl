#pragma once

#include "fftcommon.hlsl"

#define PI 3.14159265359

#if !defined(N)
#define N 1024
#endif
#define T N/2
#define rN (float(1) / float(N))

groupshared float shared_freq[N*2];

int expand(int idxL, int N1, int N2){
	return (idxL / N1) * N1 * N2 + (idxL % N1);
}

void exchange(
	inout float shared_freq[N*2],
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

float2 complex_div(float2 a, float2 b) {
	float d = 1.0 / (b.x*b.x + b.y*b.y);
	return float2(
		(a.x*b.x + a.y*b.y) * d,
		(a.y*b.x - a.x*b.y) * d
	);
}

//float2 complex_conjugate(float2 a) {
//	return a * float2(1,-1);
//}
//
//void extract_real_results(float2 z_k, float2 z_nk, out float2 X, out float2 Y) {
//	X = (z_k + complex_conjugate(z_nk)) * 0.5;
//	Y = complex_mul(float2(0,-1), (z_k - complex_conjugate(z_nk)) * 0.5);
//}

void do_fft(inout float2 v[2], in int t, in int sign) {
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
		exchange(shared_freq, v, idxD, Ns, idxS, N / 2);
	}
	if (sign < 0) {
		v[0] *= rN;
		v[1] *= rN;
	}
}

void do_fft2(inout float sequence[N*2], in int t, in int sign) {
	float2 v[2] = {
		float2(sequence[t], sequence[t + N]),
		float2(sequence[t + T], sequence[t + T + N]),
	};
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
		exchange(sequence, v, idxD, Ns, idxS, N / 2);
	}
	if (sign < 0) {
		v[0] *= rN;
		v[1] *= rN;
	}
	sequence[t] 		= v[0].x;
	sequence[t + N] 	= v[0].y;
	sequence[t + T] 	= v[1].x;
	sequence[t + T + N] = v[1].y;
}

void fft_horizontal_forward(
    RWTexture2D<float4> inImage,    // input image, is float4 but only one channel will be used
    RWTexture2D<float2> freqImgRG, 	// outout image for red and green channel
	RWTexture2D<float2> freqImgBA,	// output image for blue and alpha channel
    uint t,		 					// thread index into the fft sequence
	uint row 						// row we read from and write to in the image
) {
	int sign = 1;

	float2 v_rg[2];
	for(int r = 0; r < 2; ++r) {
		v_rg[r] = inImage[int2(t + r * T, row)].rg;
	}
	do_fft(v_rg, t, sign);

	for(int r = 0; r < 2; ++r) {
		freqImgRG[int2(t + r * T, row)] = v_rg[r];
	}

    GroupMemoryBarrierWithGroupSync();

	float2 v_ba[2];
	for(int r = 0; r < 2; ++r) {
		v_ba[r] = inImage[int2(t + r * T, row)].ba;
	}
	do_fft(v_ba, t, sign);

	for(int r = 0; r < 2; ++r) {
		freqImgBA[int2(t + r * T, row)] = v_ba[r];
	}
}

void fft_horizontal_backward(
    RWTexture2D<float4> outImage,   // output image, is float4 but only one channel will be used
	RWTexture2D<float2> fullRG,		// input full frequency image for channels red green
	RWTexture2D<float2> fullBA,		// input full frequency image for channels blue alpha
    RWTexture2D<float2> RGFreqImg, 	// frequency image for red and green channel
	RWTexture2D<float2> BAFreqImg,	// frequency image for blue and alpha channel
    uint t,		 					// thread index into the fft sequence
	uint row 						// row we read from and write to in the image
) { 
	int sign = -1;

	float2 v_rg[2];
	for(int r = 0; r < 2; ++r) {
		v_rg[r] = RGFreqImg[int2(t + r * T, row)];
	}
	do_fft(v_rg, t, sign);

	float2 v_ba[2];
	for(int r = 0; r < 2; ++r) {
		v_ba[r] = BAFreqImg[int2(t + r * T, row)];
	}
	do_fft(v_ba, t, sign);

	for (int r = 0; r < 2; ++r) {
		outImage[int2(t + r * T, row)] = float4(
			v_rg[r],
			v_ba[r]
		);
	}
}

float2 test_filter(float2 value, int2 index) {
	int threashhold = 100;
	bool under = (
		(index.x < threashhold || index.x > (N-threashhold)) && 
		(index.y < (threashhold/2) || index.y > (T-(threashhold/2)))
	);
	if (!under) {
		return float2(0,0);
	}
	else {
		return value;
	}
}

void fft_vertical_apply(
	RWTexture2D<float4> kernel, 	// kernel to apply to image
	RWTexture2D<float2> fullRG,		// 
	RWTexture2D<float2> fullBA,		// 
	RWTexture2D<float4> inImage,	// rgb input image
    RWTexture2D<float2> RGFreqImg, 	// frequency image for red and green channel
	RWTexture2D<float2> BAFreqImg,	// frequency image for blue and alpha channel
	uint column, 					// row we read from and write to in the image
    uint t		 					// thread index into the fft sequence
) {
	// forward transform rg 
	int sign = 1;
	float2 v_rg[2];
	for(int r = 0; r < 2; ++r) {
		v_rg[r] = RGFreqImg[int2(column,t + r * T)];
	}
	do_fft(v_rg, t, sign);
	// v_rg now contains the vertical transform of two pixel's red and green channels
	for (int r = 0; r < 2; ++r) {
		fullRG[int2(column,t + r * T)] = v_rg[r];
		// apply filter
		// TODO
		//v_rg[r] = test_filter(v_rg[r], int2(column, t));
		v_rg[r] = complex_mul(v_rg[r], kernel[int2(column,t + r * T)].rg);
		//v_rg[r] = v_rg[r] * kernel[int2(column,t + r * T)].rg;
	}

	// backwards transform rg
    GroupMemoryBarrierWithGroupSync();
	sign = -1;
	do_fft(v_rg, t, sign);
	for(int r = 0; r < 2; ++r) {
		RGFreqImg[int2(column,t + r * T)] = v_rg[r];
	}

	// forward transform ba
    GroupMemoryBarrierWithGroupSync();
	sign = 1;
	float2 v_ba[2];
	for(int r = 0; r < 2; ++r) {
		v_ba[r] = BAFreqImg[int2(column,t + r * T)];
	}
	do_fft(v_ba, t, sign);

	// v_ba now contains the vertical transform of two pixel's red and green channels
	for (int r = 0; r < 2; ++r) {
		fullBA[int2(column,t + r * T)] = v_ba[r];
		// apply filter
		// TODO
		//v_ba[r] = test_filter(v_ba[r], int2(column, t));
		v_ba[r] = complex_mul(v_ba[r], kernel[int2(column,t + r * T)].ba);
		//v_ba[r] = v_ba[r] * kernel[int2(column,t + r * T)].ba;
	}

	// backwards transform ba
    GroupMemoryBarrierWithGroupSync();
	sign = -1;
	do_fft(v_ba, t, sign);
	for(int r = 0; r < 2; ++r) {
		BAFreqImg[int2(column,t + r * T)] = v_ba[r];
	}
}