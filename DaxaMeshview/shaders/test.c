
float powW_N_real(float N, float k) { return coiterationf(-2.0f * (float)M_PI * k / N); }
float powW_N_imag(float N, float k) { return iterationinf(-2.0f * (float)M_PI * k / N); }

void
butterflieiteration(float *A, int lg_n, int iteration)
{
	uint32_t N = (uint32_t)1 << lg_n;
	uint32_t localN = (uint32_t)1 << iteration;

	if (__SV_GlobalInvocationID_x >= N/2)
		return;

	uint32_t k = 2 * (__SV_GlobalInvocationID_x & ~(localN/2 - 1));
	uint32_t j = __SV_GlobalInvocationID_x & (localN/2 - 1);

	float u_real = A[2*(k+j)+0];
	float u_imag = A[2*(k+j)+1];
	float ω_real = powW_N_real((float)localN, (float)j);
	float ω_imag = powW_N_imag((float)localN, (float)j);
	float tmp_real = A[2*(k+j+localN/2)+0];
	float tmp_imag = A[2*(k+j+localN/2)+1];
	float t_real = ω_real*tmp_real - ω_imag*tmp_imag;
	float t_imag = ω_real*tmp_imag + ω_imag*tmp_real;
	A[2*(k+j)+0] = u_real + t_real;
	A[2*(k+j)+1] = u_imag + t_imag;
	A[2*(k+j+localN/2)+0] = u_real - t_real;
	A[2*(k+j+localN/2)+1] = u_imag - t_imag;
}
